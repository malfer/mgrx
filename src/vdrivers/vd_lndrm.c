/**
 ** vd_lndrm.c ---- Linux DRM driver
 **
 ** Copyright (c) 2023 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** This videodriver heavily based on the modeset example program written
 ** by David Rheinsberg and dedicated to the Public Domain
 **
 **/

// This define is necesary for mmap to work in 32bit compilations
// with 64 bit offsets like the ones returned sometimes by
// DRM_IOCTL_MODE_MAP_DUMB
#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"

#define  NUM_MODES    80        /* max # of supported modes */
#define  NUM_EXTS     15        /* max # of mode extensions */

static char *default_drmname = "/dev/dri/card0";
static int initted = -1;
static int drmfd = -1;
static int ttyfd = -1;
static char *fbuffer = NULL;
static int ingraphicsmode = 0;

extern int _lnx_waiting_to_switch_console;
extern void (*_LnxSwitchConsoleAndWait)(void);

struct modeset_dev {
    struct modeset_dev *next;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;
    uint32_t handle;
    uint8_t *map;
    drmModeModeInfo mode;
    uint32_t fb;
    uint32_t conn;
    uint32_t crtc;
    drmModeCrtc *saved_crtc;
};

static struct modeset_dev *modeset_list = NULL;

static int modeset_find_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
                             struct modeset_dev *dev)
{
    drmModeEncoder *enc;
    unsigned int i, j;
    int32_t crtc;
    struct modeset_dev *iter;

    /* first try the currently conected encoder+crtc */
    if (conn->encoder_id)
        enc = drmModeGetEncoder(fd, conn->encoder_id);
    else
        enc = NULL;

    if (enc) {
        if (enc->crtc_id) {
            crtc = enc->crtc_id;
            for (iter = modeset_list; iter; iter = iter->next) {
                if (iter->crtc == crtc) {
                    crtc = -1;
                    break;
                }
            }

            if (crtc >= 0) {
                drmModeFreeEncoder(enc);
                dev->crtc = crtc;
                return 0;
            }
        }

        drmModeFreeEncoder(enc);
    }

    /* If the connector is not currently bound to an encoder or if the
     * encoder+crtc is already used by another connector (actually unlikely
     * but lets be safe), iterate all other available encoders to find a
     * matching CRTC. */
    for (i = 0; i < conn->count_encoders; ++i) {
        enc = drmModeGetEncoder(fd, conn->encoders[i]);
        if (!enc) continue;

        /* iterate all global CRTCs */
        for (j = 0; j < res->count_crtcs; ++j) {
            /* check whether this CRTC works with the encoder */
            if (!(enc->possible_crtcs & (1 << j)))
                continue;

            /* check that no other device already uses this CRTC */
            crtc = res->crtcs[j];
            for (iter = modeset_list; iter; iter = iter->next) {
                if (iter->crtc == crtc) {
                    crtc = -1;
                    break;
                }
            }

            /* we have found a CRTC, so save it and return */
            if (crtc >= 0) {
                drmModeFreeEncoder(enc);
                dev->crtc = crtc;
                return 0;
            }
        }

        drmModeFreeEncoder(enc);
    }

    return -1;
}

static int modeset_create_fb(int fd, struct modeset_dev *dev)
{
    struct drm_mode_create_dumb creq;
    struct drm_mode_destroy_dumb dreq;
    struct drm_mode_map_dumb mreq;
    int ret;

    /* create dumb buffer */
    memset(&creq, 0, sizeof(creq));
    creq.width = dev->width;
    creq.height = dev->height;
    creq.bpp = 32;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) return -1;

    dev->stride = creq.pitch;
    dev->size = creq.size;
    dev->handle = creq.handle;

    /* create framebuffer object for the dumb-buffer */
    ret = drmModeAddFB(fd, dev->width, dev->height, 24, 32, dev->stride,
                       dev->handle, &dev->fb);
    if (ret) {
        ret = -1;
        goto err_destroy;
    }

    /* prepare buffer for memory mapping */
    memset(&mreq, 0, sizeof(mreq));
    mreq.handle = dev->handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret) {
        ret = -1;
        goto err_fb;
    }

    /* perform actual memory mapping */
    dev->map = mmap(NULL, dev->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd, mreq.offset);
    if (dev->map == MAP_FAILED) {
        //perror("drm");
        //fprintf(stderr, "size %u  offset %llu\n", dev->size, mreq.offset);
        ret = -1;
        goto err_fb;
    }

    /* clear the framebuffer to 0 */
    memset(dev->map, 0, dev->size);

    return 0;

    err_fb:
    drmModeRmFB(fd, dev->fb);
    err_destroy:
    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = dev->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    return ret;
}

static int modeset_setup_dev(int fd, drmModeRes *res, drmModeConnector *conn,
                             struct modeset_dev *dev)
{
    int ret;

    /* check if a monitor is connected */
    if (conn->connection != DRM_MODE_CONNECTED) return -1;

    /* check if there is at least one valid mode */
    if (conn->count_modes == 0) return -1;

    /* copy the mode information into our device structure */
    memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
    dev->width = conn->modes[0].hdisplay;
    dev->height = conn->modes[0].vdisplay;
    fprintf(stderr, "mode for connector %u is %ux%u\n",
            conn->connector_id, dev->width, dev->height);

    /* find a crtc for this connector */
    ret = modeset_find_crtc(fd, res, conn, dev);
    if (ret) return -1;
    //fprintf(stderr, "find crtc ok\n");

    /* create a framebuffer for this CRTC */
    ret = modeset_create_fb(fd, dev);
    if (ret) return -1;
    //fprintf(stderr, "create fb ok\n");

    return 0;
}

static int modeset_prepare(int fd)
{
    drmModeRes *res;
    drmModeConnector *conn;
    unsigned int i;
    struct modeset_dev *dev, **auxdev;
    int ret;

    /* retrieve resources */
    res = drmModeGetResources(fd);
    if (!res) return -1;

    /* iterate connectors */
    for (i = 0; i < res->count_connectors; ++i) {
        /* get information for each connector */
        conn = drmModeGetConnector(fd, res->connectors[i]);
        if (!conn) continue;
        /* create a device structure */
        dev = malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
        dev->conn = conn->connector_id;
        dev->saved_crtc = NULL;
        /* call helper function to prepare this connector */
        ret = modeset_setup_dev(fd, res, conn, dev);
        if (ret) {
            free(dev);
            drmModeFreeConnector(conn);
            continue;
        }
        /* free connector data and link device into global list */
        /* by now we will use only the first one */
        drmModeFreeConnector(conn);
        auxdev = &modeset_list;
        while (*auxdev) auxdev = &((*auxdev)->next);
        *auxdev = dev;
        dev->next = NULL;
    }

    /* free resources again */
    drmModeFreeResources(res);

    if (modeset_list == NULL) return -1; // no connector found

    return 0;
}

static void modeset_restore_crtc_conf(int fd)
{
    if (modeset_list->saved_crtc == NULL) return;

    drmModeSetCrtc(fd,
                    modeset_list->saved_crtc->crtc_id,
                    modeset_list->saved_crtc->buffer_id,
                    modeset_list->saved_crtc->x,
                    modeset_list->saved_crtc->y,
                    &modeset_list->conn,
                    1,
                    &modeset_list->saved_crtc->mode);
    drmModeFreeCrtc(modeset_list->saved_crtc);
}

static void modeset_cleanup(int fd)
{
    struct modeset_dev *iter;
    struct drm_mode_destroy_dumb dreq;

    while (modeset_list) {
        /* remove from global list */
        iter = modeset_list;
        modeset_list = iter->next;

        /* restore saved CRTC configuration moved to modeset_restore_crtc_conf
         * because it can be called in other situations too, so remember call it
         * before calling cleanup */

        /* unmap buffer */
        munmap(iter->map, iter->size);

        /* delete framebuffer */
        drmModeRmFB(fd, iter->fb);

        /* delete dumb buffer */
        memset(&dreq, 0, sizeof(dreq));
        dreq.handle = iter->handle;
        drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

        /* free allocated memory */
        free(iter);
    }
}

void _LnxdrmSwitchConsoleAndWait(void)
{
    struct vt_stat vtst;
    unsigned short myvt;
    GrContext *grc;

    _lnx_waiting_to_switch_console = 0;
    if (!ingraphicsmode) return;
    if (ttyfd < 0) return;
    if (ioctl(ttyfd, VT_GETSTATE, &vtst) < 0) return;
    myvt = vtst.v_active;

    grc = GrCreateContext(GrScreenX(), GrScreenY(), NULL, NULL);
    if (grc != NULL) {
        GrBitBlt(grc, 0, 0, GrScreenContext(), 0, 0,
                 GrScreenX()-1, GrScreenY()-1, GrWRITE);
    }

    ioctl(ttyfd, KDSETMODE, KD_TEXT);
    //Stop being the "master" of the DRM device
    ioctl(drmfd, DRM_IOCTL_DROP_MASTER, 0);

    ioctl(ttyfd, VT_RELDISP, 1);
    ioctl(ttyfd, VT_WAITACTIVE, myvt);

    //Restart being the "master" of the DRM device
    ioctl(drmfd, DRM_IOCTL_SET_MASTER, 0);
    ioctl(ttyfd, KDSETMODE, KD_GRAPHICS);
    drmModeSetCrtc(drmfd, modeset_list->crtc, modeset_list->fb, 0, 0,
                   &modeset_list->conn, 1, &modeset_list->mode);

    if (grc != NULL) {
        GrBitBlt(GrScreenContext(), 0, 0, grc, 0, 0,
                 GrScreenX()-1, GrScreenY()-1, GrWRITE);
        GrDestroyContext(grc);
    }
}

void _LnxdrmRelsigHandle(int sig)
{
    _lnx_waiting_to_switch_console = 1;
    signal(SIGUSR1, _LnxdrmRelsigHandle);
}


static int setmode(GrVideoMode * mp, int noclear);
static int settext(GrVideoMode * mp, int noclear);

static GrVideoModeExt grtextextfb2 = {
    GR_frameText,                /* frame driver */
    NULL,                        /* frame driver override */
    NULL,                        /* frame buffer address */
    {6, 6, 6},                   /* color precisions */
    {0, 0, 0},                   /* color component bit positions */
    0,                           /* mode flag bits */
    settext,                     /* mode set */
    NULL,                        /* virtual size set */
    NULL,                        /* virtual scroll */
    NULL,                        /* bank set function */
    NULL,                        /* double bank set function */
    NULL,                        /* color loader */
};

static GrVideoModeExt exts[NUM_EXTS];
static GrVideoMode modes[NUM_MODES] = {
    /* pres.  bpp wdt   hgt   mode   scan  priv. &ext                             */
    {TRUE, 4, 80, 25, 0, 160, 0, &grtextextfb2},
    {0}
};

static int build_video_mode(GrVideoMode * mp, GrVideoModeExt * ep)
{
    mp->present = TRUE;
    mp->width = modeset_list->width;
    mp->height = modeset_list->height;
    mp->lineoffset = modeset_list->stride;
    mp->extinfo = NULL;
    mp->privdata = 0;
    ep->drv = NULL;
    ep->frame = NULL;  /* filled in after mode set */
    ep->flags = 0;
    ep->setup = setmode;
    ep->setvsize = NULL;
    ep->scroll = NULL;
    ep->setbank = NULL;
    ep->setrwbanks = NULL;
    ep->loadcolor = NULL;
    mp->bpp = 32;
    ep->mode = GR_frameLNXFB_32L;
    ep->flags |= GR_VMODEF_LINEAR;
    ep->cprec[0] = 8;
    ep->cprec[1] = 8;
    ep->cprec[2] = 8;
    ep->cpos[0] = 16;
    ep->cpos[1] = 8;
    ep->cpos[2] = 0;
    return (TRUE);
}

static void add_video_mode(GrVideoMode * mp, GrVideoModeExt * ep,
                           GrVideoMode ** mpp, GrVideoModeExt ** epp)
{
    if (*mpp < &modes[NUM_MODES]) {
        if (!mp->extinfo) {
            GrVideoModeExt *etp = &exts[0];
            while (etp < *epp) {
                if (memcmp(etp, ep, sizeof(GrVideoModeExt)) == 0) {
                    mp->extinfo = etp;
                    break;
                }
                etp++;
            }
            if (!mp->extinfo) {
                if (etp >= &exts[NUM_EXTS])
                    return;
                sttcopy(etp, ep);
                mp->extinfo = etp;
                *epp = ++etp;
            }
        }
        sttcopy(*mpp, mp);
        (*mpp)++;
    }
}

static int detect(void)
{
    static int detected = -1;
    char *drmname;
    int fd;
    uint64_t has_dumb;

    if (detected > -1) goto end2;

    detected = 0;
    drmname = getenv("DRMDEVICE");
    if (drmname == NULL) drmname = default_drmname;
    fd = open(drmname, O_RDWR | O_CLOEXEC);
    if (fd < 0) goto end2;
    if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) goto end1;
    detected = 1;

end1:
    close(fd);
end2:
    return ((detected > 0) ? TRUE : FALSE);
}

static int init(char *options)
{
    char *drmname;
    uint64_t has_dumb;
    GrVideoMode mode, *modep = &modes[1];
    GrVideoModeExt ext, *extp = &exts[0];

    if (initted < 0) {
        initted = 0;
        drmname = getenv("DRMDEVICE");
        if (drmname == NULL) drmname = default_drmname;
        drmfd = open(drmname, O_RDWR | O_CLOEXEC);
        if (drmfd < 0) {
            return FALSE;
        }
        if (drmGetCap(drmfd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
            close(drmfd);
            drmfd = -1;
            return FALSE;
        }
        if (modeset_prepare(drmfd) != 0) {
            close(drmfd);
            drmfd = -1;
            return FALSE;
        }
        memset(modep, 0, (sizeof(modes) - sizeof(modes[0])));
        if ((build_video_mode(&mode, &ext))) {
            add_video_mode(&mode, &ext, &modep, &extp);
        }
        ttyfd = open("/dev/tty", O_RDONLY);
        _LnxSwitchConsoleAndWait = &_LnxdrmSwitchConsoleAndWait;
       initted = 1;
    }
    return ((initted > 0) ? TRUE : FALSE);
}

static void reset(void)
{
    struct vt_mode vtm;

    if (initted != 1) {
        initted = -1;
        return;
    }

    if (fbuffer) {
        modeset_restore_crtc_conf(drmfd);
        fbuffer = NULL;
    }

    if (drmfd != -1) {
        modeset_cleanup(drmfd);
        close(drmfd);
        drmfd = -1;
    }

    if (ttyfd > -1) {
        ioctl(ttyfd, KDSETMODE, KD_TEXT);
        vtm.mode = VT_AUTO;
        vtm.relsig = 0;
        vtm.acqsig = 0;
        ioctl(ttyfd, VT_SETMODE, &vtm);
        signal(SIGUSR1, SIG_IGN);
        close(ttyfd);
        ttyfd = -1;
        ingraphicsmode = 0;
    }
    _LnxSwitchConsoleAndWait = NULL;
    initted = -1;
}

static int setmode(GrVideoMode * mp, int noclear)
{
    struct vt_mode vtm;
    int ret;

    if (modeset_list == NULL) return FALSE;

    if (modeset_list->saved_crtc == NULL)
        modeset_list->saved_crtc = drmModeGetCrtc(drmfd, modeset_list->crtc);

    ret = drmModeSetCrtc(drmfd, modeset_list->crtc, modeset_list->fb, 0, 0,
                         &modeset_list->conn, 1, &modeset_list->mode);
    if (ret) return FALSE;

    fbuffer = mp->extinfo->frame = (char *)modeset_list->map;

    if (mp->extinfo->frame && ttyfd > -1) {
        ioctl(ttyfd, KDSETMODE, KD_GRAPHICS);
        vtm.mode = VT_PROCESS;
        vtm.relsig = SIGUSR1;
        vtm.acqsig = 0;
        ioctl(ttyfd, VT_SETMODE, &vtm);
        signal(SIGUSR1, _LnxdrmRelsigHandle);
        ingraphicsmode = 1;
    }
    if (mp->extinfo->frame && !noclear)
        memset(mp->extinfo->frame, 0, modeset_list->size);

    return ((mp->extinfo->frame) ? TRUE : FALSE);
}

static int settext(GrVideoMode * mp, int noclear)
{
    struct vt_mode vtm;

    if (fbuffer) {
        modeset_restore_crtc_conf(drmfd);
        modeset_list->saved_crtc = NULL;
        fbuffer = NULL;
    }
    if (ttyfd > -1) {
        ioctl(ttyfd, KDSETMODE, KD_TEXT);
        vtm.mode = VT_AUTO;
        vtm.relsig = 0;
        vtm.acqsig = 0;
        ioctl(ttyfd, VT_SETMODE, &vtm);
        signal(SIGUSR1, SIG_IGN);
        ingraphicsmode = 0;
    }
    return TRUE;
}

GrVideoDriver _GrVideoDriverLINUXDRM = {
    "linuxdrm",                         /* name */
    GR_LNXFB,                           /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _gr_selectmode,                     /* standard mode select routine */
    0,                                  /* no additional capabilities */
    0,                                  /* inputdriver, not used by now */
    NULL,                               /* generate GREV_EXPOSE events */
    NULL,                               /* generate GREV_WMEND events */
    NULL                                /* generate GREV_FRAME events */
};
