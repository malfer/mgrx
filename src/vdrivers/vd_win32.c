/**
 ** vd_win32.c ---- the standard Win32-API driver
 **
 ** Author: Gernot Graeff
 ** E-mail: gernot.graeff@t-online.de
 ** Date:   13.11.98
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
 ** Changes by Josu Onandia (jonandia@fagorautomation.es) 21/02/2001
 **   - The colors loaded in the ColorList are guaranteed to be actually used
 **     by Windows (GetNearestColor), for the GR_frameWin32 to work.
 **   - When the window is created, it gets the maximum size allowed by the
 **     current mode. Indeed this size is stored (curWindowWidth/
 **     curWindowHeight).
 **     When the window is going to be resized (WM_GETMINMAXINFO) it's not
 **     allowed to grow bigger than this maximum size (it makes nosense).
 **   - Added some modes for 24bpp colors.
 **   - When changed to text-mode, the graphics window is hidden. If the
 **     application has a console (linked with -mconsole) it can use
 **     printf/scanf and the like.
 **     When changed again into graphics mode, the window reappears.
 **   - Inter-task synchronization. In some cases the two threads are
 **     manipulating at the same time the main window, and the on-memory
 **     bitmap. I guess this is causing trouble, so in some cases the main
 **     thread suspends the worker thread, make its operation, and then
 **     resumes it.
 **   - The window title is selectable with a define, at compile time.
 **     If not defined, it defaults to "GRX".
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 02/01/2002
 **   - Go to full screen if the framemode dimensions are equal to
 **     the screen dimensions (setting the client start area at 0,0).
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 02/02/2002
 **   - The w32 imput queue implemented as a circular queue.
 **   - All the input related code moved to w32inp.c
 **   - The main window is created in WinMain, so the grx program
 **     can use other videodrivers like the memory one.
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 11/02/2002
 **   - Now the GRX window is properly closed, so the previous app
 **     gets the focus.
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 31/03/2002
 **   - Accepts arbitrary (user defined) resolution.
 **
 ** Changes by Thomas Demmer (TDemmer@krafteurope.com)
 **   - Instead of begin with WinMain and start a thread with the main
 **     GRX program, do it backward: begin in main and start a thread to
 **     handle the Windows window. With this change we get rid of the
 **     awful GRXMain special entry point.
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 12/02/2003
 **   - Sanitize the Thomas changes.
 **
 ** Changes by Thomas Demmer (TDemmer@krafteurope.com) 18/03/2003
 **   - Use a DIB for the hDCMem.
 **
 ** Changes by Josu Onandia (jonandia@fagorautomation.es) 19/03/2003
 **   - With the Thomas idea of using a DIB, we can now use the DIB like
 **     a linear frame buffer, so the new win32 framedrivers can take
 **     advantage of the standard GRX frame drivers.
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 01/12/2007
 **   - Added videomodes for wide monitors
 **
 ** Change by Mario Zagar 02/09/2008
 **   - Fix app termination bug
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 25/01/2009
 **   - Simplified the WM_PAINT proccess to solve a race condition
 **     found in Vista running in multiple core cpu's.
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 06/09/2017
 **   - Workarround to loadcolor fail in W7 for the 256 color driver
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 16/10/2019
 **   - Solved a bug when changing modes without exiting, we need to
 **     delete the old hDCMem and generate a new one
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 12/11/2019
 **   - Added code to generate GREV_WMEND events
 **
 ** Changes by M.Alvarez (malfer@telefonica.net) 30/03/2020
 **   - Workarround to solve a race condition in W10, sometimes hDCMem
 **     was used before it was ready
 **
 ** 211123 M.Alvarez, support window resize
 ** 211205 M.Alvarez, process WM_NCMOUSEMOVE event, in w10 ther are no 
 **                   WM_NCHITTEST event over the caption title, so the
 **                   mouse pointer disappeared there
 ** 211205 M.Alvarez, when window resize support is active create a maximum
 **                   DIB and reuse it so transitions in window resizes can
 **                   be smooth
 ** 220318 M.Alvarez, changed ALT09AZ recognition to be more generic
 **/

#include "libwin32.h"
#include "libgrx.h"
#include "grdriver.h"
#include "arith.h"

#ifndef GRXWINDOW_TITLE
#define GRXWINDOW_TITLE "MGRX"
#endif

HWND hGRXWnd = NULL;
HDC hDCMem = NULL;

CRITICAL_SECTION _csEventQueue;
W32Event *_W32EventQueue = NULL;
int _W32EventQueueSize = 0;
int _W32EventQueueRead = 0;
int _W32EventQueueWrite = 0;
int _W32EventQueueLength = 0;

int _W32ClientWidth, _W32ClientHeight;

int _W32GenWMEndEvents = GR_GEN_NO;
int _W32GenWSzChgEvents = FALSE;

int _W32UserHadSetWName = 0;

static HBITMAP hBmpDIB = NULL;

static int maxScreenWidth, maxScreenHeight;
static int curWindowWidth, curWindowHeight;
static int winShowed;

HANDLE windowThread = INVALID_HANDLE_VALUE;
static HANDLE mainThread   = INVALID_HANDLE_VALUE;

static volatile int isWindowThreadRunning = 0;
static volatile int isMainWaitingTermination = 0;
static volatile int ishDCMemReady = 0;

static DWORD WINAPI WndThread(void *param);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam);

static void loadcolor(int c, int r, int g, int b)
{
    RGBQUAD color;
    int i;
    
    color.rgbBlue = b;
    color.rgbGreen = g;
    color.rgbRed = r;
    color.rgbReserved = 0;
    
    /* Workarround to SetDIBColorTable fail in W7, probably a race condition,
     * this seems to work without disturbing the driver a lot */
    i = 0;
    do {
        if (SetDIBColorTable(hDCMem, c, 1, &color) > 0) break;
        i++;
        GrSleep(5);
    } while (i < 5);
    
    InvalidateRect(hGRXWnd, NULL, FALSE);
}

static HBITMAP CreateDIB8(HDC hdc, int w, int h, char **pBits)
{
    BITMAPINFO *pbmInfo;
    HBITMAP hBmp;

    pbmInfo = malloc(sizeof(BITMAPINFO) +255*sizeof(RGBQUAD));
    pbmInfo->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
    pbmInfo->bmiHeader.biWidth = w;
    pbmInfo->bmiHeader.biHeight = -h;
    pbmInfo->bmiHeader.biPlanes = 1;
    pbmInfo->bmiHeader.biBitCount = 8;
    pbmInfo->bmiHeader.biCompression = BI_RGB;
    pbmInfo->bmiHeader.biSizeImage = 0;
    pbmInfo->bmiHeader.biXPelsPerMeter = 0;
    pbmInfo->bmiHeader.biYPelsPerMeter = 0;
    pbmInfo->bmiHeader.biClrUsed = 0;
    pbmInfo->bmiHeader.biClrImportant = 0;
    hBmp = CreateDIBSection(0, pbmInfo, DIB_RGB_COLORS, (void*)pBits, 0, 0);
    free(pbmInfo);
    return(hBmp);
}

static HBITMAP CreateDIB24(HDC hdc, int w, int h, char **pBits)
{
    BITMAPINFO *pbmInfo;
    HBITMAP hBmp;

    pbmInfo = malloc(sizeof(BITMAPINFO));
    pbmInfo->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
    pbmInfo->bmiHeader.biWidth = w;
    pbmInfo->bmiHeader.biHeight = -h;
    pbmInfo->bmiHeader.biPlanes = 1;
    pbmInfo->bmiHeader.biBitCount = 24;
    pbmInfo->bmiHeader.biCompression = BI_RGB;
    pbmInfo->bmiHeader.biSizeImage = 0;
    pbmInfo->bmiHeader.biXPelsPerMeter = 0;
    pbmInfo->bmiHeader.biYPelsPerMeter = 0;
    pbmInfo->bmiHeader.biClrUsed = 0;
    pbmInfo->bmiHeader.biClrImportant = 0;
    hBmp = CreateDIBSection(0, pbmInfo, DIB_RGB_COLORS, (void*)pBits, 0, 0);
    free(pbmInfo);
    return(hBmp);
}

static int setmode(GrVideoMode * mp, int noclear)
{
    RECT Rect;
    HDC hDC;
    HBRUSH hBrush;
    int inipos;
    int lowidth, resto;
    static int curbpp = 0;

    if (mp->extinfo->mode != GR_frameText) {
        char name[100];

        if (!_W32UserHadSetWName) {
            sprintf (name, "mgrx %dx%dx%d", mp->width, mp->height, mp->bpp);
            SetWindowText(hGRXWnd, name);
        }

        ishDCMemReady = 0;
        inipos = 50;
        if (mp->width == maxScreenWidth && mp->height == maxScreenHeight)
            inipos = 0;
        Rect.left = inipos;
        Rect.top = inipos;
        Rect.right = mp->width + inipos;
        Rect.bottom = mp->height + inipos;
        AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE);
        curWindowWidth = Rect.right - Rect.left;
        curWindowHeight = Rect.bottom - Rect.top;
        if (!winShowed) {
            SetWindowPos(hGRXWnd, NULL,
                         Rect.left, Rect.top,
                         curWindowWidth, curWindowHeight,
                         SWP_DRAWFRAME | SWP_NOZORDER | SWP_SHOWWINDOW);
            winShowed = TRUE;
        }
        _W32ClientWidth =  mp->width;
        _W32ClientHeight =  mp->height;

        if (!_W32GenWSzChgEvents) { //regenerate the DIB for each setmode
            if (hBmpDIB != NULL)
                DeleteObject(hBmpDIB);
            if (hDCMem != NULL)
                DeleteDC(hDCMem);
        
            hDC = GetDC(hGRXWnd);
            hDCMem = CreateCompatibleDC(hDC);
            //if (!hDCMem) printf("Error, hDCMem\n");
            if (mp->bpp == 8) {
                hBmpDIB = CreateDIB8(hDC, mp->width, mp->height,
                                     &mp->extinfo->frame);
            } else {
                hBmpDIB = CreateDIB24(hDC, mp->width, mp->height,
                                      &mp->extinfo->frame);
            }
            //if (!hBmpDIB) printf("Error, hBmpDIB\n");
            SelectObject(hDCMem, hBmpDIB);
            SetRect(&Rect, 0, 0, mp->width, mp->height);
            hBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hDCMem, &Rect, hBrush);
            ReleaseDC(hGRXWnd, hDC);
            DeleteObject(hBrush);
            lowidth = (mp->bpp == 8) ? mp->width : mp->width * 3;
            resto = lowidth % 4;
            if (resto) resto = 4 - resto;
            mp->lineoffset = lowidth + resto;
        } else { //create a big DIB and reuse it except if bpp changes
            if (hDCMem == NULL || curbpp != mp->bpp) {
                if (hBmpDIB != NULL) DeleteObject(hBmpDIB);
                if (hDCMem != NULL)DeleteDC(hDCMem);
                hDC = GetDC(hGRXWnd);
                hDCMem = CreateCompatibleDC(hDC);
                //if (!hDCMem) printf("Error, hDCMem\n");
                if (mp->bpp == 8) {
                    hBmpDIB = CreateDIB8(hDC, maxScreenWidth, maxScreenHeight,
                                         &mp->extinfo->frame);
                    curbpp = 8;
                } else {
                    hBmpDIB = CreateDIB24(hDC, maxScreenWidth, maxScreenHeight,
                                          &mp->extinfo->frame);
                    curbpp = 24;
                }
                //if (!hBmpDIB) printf("Error, hBmpDIB\n");
                SelectObject(hDCMem, hBmpDIB);
                SetRect(&Rect, 0, 0, maxScreenWidth, maxScreenHeight);
                hBrush = CreateSolidBrush(RGB(0, 0, 0));
                FillRect(hDCMem, &Rect, hBrush);
                ReleaseDC(hGRXWnd, hDC);
                DeleteObject(hBrush);
            }
            lowidth = (mp->bpp == 8) ? maxScreenWidth : maxScreenWidth * 3;
            resto = lowidth % 4;
            if (resto) resto = 4 - resto;
            mp->lineoffset = lowidth + resto;
        }
        ishDCMemReady = 1;
        //InvalidateRect(hGRXWnd, NULL, FALSE);
        //UpdateWindow(hGRXWnd);
        SetForegroundWindow(hGRXWnd);
    } else {
        /* If changing to text-mode, hide the graphics window. */
        if (hGRXWnd != NULL) {
            ShowWindow(hGRXWnd, SW_HIDE);
            winShowed = FALSE;
        }
        ishDCMemReady = 0;
        if (hBmpDIB != NULL) {
            DeleteObject(hBmpDIB);
            hBmpDIB = NULL;
        }
        if (hDCMem != NULL) {
            DeleteDC(hDCMem);
            hDCMem = NULL;
        }
    }
    return (TRUE);
}

static void setbank_dummy(int bk)
{
    bk = bk;
}

GrVideoModeExt grtextext = {
    GR_frameText,                /* frame driver */
    NULL,                        /* frame driver override */
    NULL,                        /* frame buffer address */
    {0, 0, 0},                   /* color precisions */
    {0, 0, 0},                   /* color component bit positions */
    0,                           /* mode flag bits */
    setmode,                     /* mode set */
    NULL,                        /* virtual size set */
    NULL,                        /* virtual scroll */
    NULL,                        /* bank set function */
    NULL,                        /* double bank set function */
    NULL                         /* color loader */
};

static GrVideoModeExt grxwinext8 = {
    GR_frameWIN32_8,             /* frame driver */
    NULL,                        /* frame driver override */
    NULL,                        /* frame buffer address */
    {8, 8, 8},                   /* color precisions */
    {0, 8, 16},                  /* color component bit positions */
    0,                           /* mode flag bits */
    setmode,                     /* mode set */
    NULL,                        /* virtual size set */
    NULL,                        /* virtual scroll */
    setbank_dummy,               /* bank set function */
    NULL,                        /* double bank set function */
    loadcolor                    /* color loader */
};

static GrVideoModeExt grxwinext24 = {
    GR_frameWIN32_24,            /* frame driver */
    NULL,                        /* frame driver override */
    NULL,                        /* frame buffer address */
    {8, 8, 8},                   /* color precisions */
    {16, 8, 0},                  /* color component bit positions */
    0,                           /* mode flag bits */
    setmode,                     /* mode set */
    NULL,                        /* virtual size set */
    NULL,                        /* virtual scroll */
    setbank_dummy,               /* bank set function */
    NULL,                        /* double bank set function */
    NULL                         /* color loader */
};

static GrVideoMode modes[] = {
    /* pres.  bpp wdt   hgt   BIOS   scan  priv. &ext  */
    {TRUE, 8, 80, 25, 0x00, 80, 1, &grtextext},

    {TRUE, 8, 320, 240, 0x00, 320, 0, &grxwinext8},
    {TRUE, 8, 640, 480, 0x00, 640, 0, &grxwinext8},
    {TRUE, 8, 800, 600, 0x00, 800, 0, &grxwinext8},
    {TRUE, 8, 1024, 768, 0x00, 1024, 0, &grxwinext8},
    {TRUE, 8, 1280, 1024, 0x00, 1280, 0, &grxwinext8},
    {TRUE, 8, 1600, 1200, 0x00, 1600, 0, &grxwinext8},
    {TRUE, 8, 1366, 768, 0x00, 1368, 0, &grxwinext8},
    {TRUE, 8, 1440, 900, 0x00, 1440, 0, &grxwinext8},
    {TRUE, 8, 1680, 1050, 0x00, 1680, 0, &grxwinext8},
    {TRUE, 8, 1920, 1080, 0x00, 1920, 0, &grxwinext8},
    {TRUE, 8, 1920, 1200, 0x00, 1920, 0, &grxwinext8},
    {TRUE, 8, 2560, 1440, 0x00, 2560, 0, &grxwinext8},
    {TRUE, 8, 3840, 2160, 0x00, 3840, 0, &grxwinext8},

    {TRUE, 24, 320, 240, 0x00, 960, 0, &grxwinext24},
    {TRUE, 24, 640, 480, 0x00, 1920, 0, &grxwinext24},
    {TRUE, 24, 800, 600, 0x00, 2400, 0, &grxwinext24},
    {TRUE, 24, 1024, 768, 0x00, 3072, 0, &grxwinext24},
    {TRUE, 24, 1280, 1024, 0x00, 3840, 0, &grxwinext24},
    {TRUE, 24, 1600, 1200, 0x00, 4800, 0, &grxwinext24},
    {TRUE, 24, 1366, 768, 0x00, 4100, 0, &grxwinext24},
    {TRUE, 24, 1440, 900, 0x00, 4320, 0, &grxwinext24},
    {TRUE, 24, 1680, 1050, 0x00, 5040, 0, &grxwinext24},
    {TRUE, 24, 1920, 1080, 0x00, 5760, 0, &grxwinext24},
    {TRUE, 24, 1920, 1200, 0x00, 5760, 0, &grxwinext24},
    {TRUE, 24, 2560, 1440, 0x00, 7680, 0, &grxwinext24},
    {TRUE, 24, 3840, 2160, 0x00, 11520, 0, &grxwinext24},

    {FALSE, 0, 9999, 9999, 0x00, 0, 0, NULL}
};

static int detect(void)
{
    static int inited = 0;
    WNDCLASSEX wndclass;

    if (!inited) {
        wndclass.cbSize = sizeof(wndclass);
        wndclass.style = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc = WndProc;
        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = 0;
        wndclass.hInstance = GetModuleHandle(NULL);
        wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
        wndclass.lpszMenuName = NULL;
        wndclass.lpszClassName = "GRXCLASS";
        wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        if (RegisterClassEx(&wndclass)== 0) return FALSE;
        inited = 1;
    }

    return TRUE;
}

static int init(char *options)
{
    int i;
    DWORD thread_id;

    if (!detect()) return FALSE;

    if (options) {
        char opt[101] = {0};
        char *token;
        strncpy(opt, options, 100);
        token = strtok(opt, " ");
        while (token != NULL) {
            if (strncmp ("rszwin", token, 6) == 0) _W32GenWSzChgEvents = TRUE;
            token = strtok(NULL, " ");
        }
    }

    if (_W32GenWSzChgEvents) {
        _GrVideoDriverWIN32.drvflags |= GR_DRIVERF_WINDOW_RESIZE;
    }
        
    /* WARNING: mainThread can not be used in the windowThread */
    mainThread = GetCurrentThread();

    InitializeCriticalSection(&_csEventQueue);

    /* The modes not compatible width the configuration */
    /* of Windows are made 'non-present'                */
    maxScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    for (i = 1; i < itemsof(modes); i++) {
        if (modes[i].width > maxScreenWidth)
            modes[i].present = FALSE;
    }
    maxScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    for (i = 1; i < itemsof(modes); i++) {
        if (modes[i].height > maxScreenHeight)
            modes[i].present = FALSE;
    }

    windowThread = CreateThread(NULL, 0, WndThread, NULL, 0, &thread_id);

    /* Wait for thread creating the window. This is a busy */
    /* waiting loop (bad), but we Sleep to yield (good)    */
    while (isWindowThreadRunning == 0)
        Sleep(1);

    return TRUE;
}

static void reset(void)
{
    if (isWindowThreadRunning) {
        isMainWaitingTermination = 1;
        PostMessage(hGRXWnd, WM_CLOSE, 0, 0);

        while (isWindowThreadRunning == 1)
            Sleep(1);

        isMainWaitingTermination = 0;
        DeleteCriticalSection(&_csEventQueue);
    }

    ishDCMemReady = 0;

    if(hBmpDIB != NULL)
    {
        DeleteObject(hBmpDIB);
        hBmpDIB = NULL;
    }
    if (hDCMem != NULL) {
        DeleteDC(hDCMem);
        hDCMem = NULL;
    }

    _W32GenWSzChgEvents = FALSE;
}

static GrVideoMode * _w32_selectmode(GrVideoDriver * drv, int w, int h,
                                     int bpp, int txt, unsigned int * ep)
{
    GrVideoMode *mp, *res;
    long resto;

    if (txt) {
        res = _gr_selectmode(drv, w, h, bpp, txt, ep);
        goto done;
    }
    for (mp = &modes[1]; mp < &modes[itemsof(modes)-1]; mp++) {
        if (mp->present && mp->width == w && mp->height == h) {
            res = _gr_selectmode(drv, w, h, bpp, txt, ep);
            goto done;
        }
    }
    /* no predefined mode found. Create a new mode if we can*/
    if (w <= maxScreenWidth && h <= maxScreenHeight) {
        mp->present = TRUE;
        mp->width = w;
        mp->height = h;
        if (bpp <= 8) {
            mp->bpp = 8;
            mp->extinfo = &grxwinext8;
            resto = mp->width % 4;
            if (resto) resto = 4 - resto;
            mp->lineoffset = mp->width + resto;
        }
        else {
            mp->bpp = 24;
            mp->extinfo = &grxwinext24;
            resto = (mp->width * 3) % 4;
            if (resto) resto = 4 - resto;
            mp->lineoffset = mp->width * 3 + resto;
        }
    }
    res = _gr_selectmode(drv, w, h, bpp, txt, ep);
done:
    return res;
}

static int genwmend(int gen)
{
    _W32GenWMEndEvents = gen;
    return _W32GenWMEndEvents;
}

GrVideoDriver _GrVideoDriverWIN32 = {
    "win32",                            /* name */
    GR_WIN32,                           /* adapter type */
    NULL,                               /* inherit modes from this driver */
    modes,                              /* mode table */
    itemsof(modes),                     /* # of modes */
    detect,                             /* detection routine */
    init,                               /* initialization routine */
    reset,                              /* reset routine */
    _w32_selectmode,                    /* special mode select routine */
    GR_DRIVERF_USER_RESOLUTION,         /* arbitrary resolution possible */
    0,                                  /* inputdriver, not used by now */
    NULL,                               /* generate GREV_EXPOSE events */
    genwmend,                           /* generate GREV_WMEND events */
    NULL                                /* generate GREV_FRAME events */
};

static DWORD WINAPI WndThread(void *param)
{
    MSG msg;
    DWORD style;

    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX;
    if (_W32GenWSzChgEvents) style |= WS_MAXIMIZEBOX;
    hGRXWnd = CreateWindow("GRXCLASS", GRXWINDOW_TITLE, style, 0, 0,
                           CW_USEDEFAULT, CW_USEDEFAULT, NULL,
                           NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hGRXWnd, SW_HIDE);
    winShowed = FALSE;

    isWindowThreadRunning = 1;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    isWindowThreadRunning = 0;

    ExitThread(0);
    return 0;
}

static int convertwin32keystate(void)
{
    int fkbState = 0;

    if (GetKeyState(VK_SHIFT) < 0)
        fkbState |= GRKBS_SHIFT;
    if (GetKeyState(VK_CONTROL) < 0)
        fkbState |= GRKBS_CTRL;
    if (GetKeyState(VK_MENU) < 0)
        fkbState |= GRKBS_ALT;
    if (GetKeyState(VK_SCROLL) < 0)
        fkbState |= GRKBS_SCROLLOCK;
    if (GetKeyState(VK_NUMLOCK) < 0)
        fkbState |= GRKBS_NUMLOCK;
    if (GetKeyState(VK_CAPITAL) < 0)
        fkbState |= GRKBS_CAPSLOCK;
    if (GetKeyState(VK_INSERT) < 0)
        fkbState |= GRKBS_INSERT;
    return fkbState;
}

static void EnqueueW32Event(UINT uMsg, WPARAM wParam, LPARAM lParam,
                            int kbstat)
{
    if (_W32EventQueue == NULL)
        return;

    EnterCriticalSection(&_csEventQueue);
    _W32EventQueue[_W32EventQueueWrite].uMsg = uMsg;
    _W32EventQueue[_W32EventQueueWrite].wParam = wParam;
    _W32EventQueue[_W32EventQueueWrite].lParam = lParam;
    _W32EventQueue[_W32EventQueueWrite].kbstat = kbstat;
    if (++_W32EventQueueWrite == _W32EventQueueSize)
        _W32EventQueueWrite = 0;
    if (++_W32EventQueueLength > _W32EventQueueSize) {
        _W32EventQueueLength--;
        if (++_W32EventQueueRead == _W32EventQueueSize)
            _W32EventQueueRead = 0;
    }
    LeaveCriticalSection(&_csEventQueue);
}

static void adjustdims(int wi, int hi, int *wo, int *ho)
{
    RECT Rect;

    Rect.left = 0;
    Rect.top = 0;
    Rect.right = wi;
    Rect.bottom = hi;
    AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, FALSE);
    *wo = Rect.right - Rect.left;
    *ho = Rect.bottom - Rect.top;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam)
{
    static int cursorOn = 1;
    int kbstat;
    BOOL fInsert;

    switch (uMsg) {

    case WM_NCHITTEST:
        {
            LRESULT res = DefWindowProc(hWnd, uMsg, wParam, lParam);
            if (res == HTCLIENT) {
                if (cursorOn) {
                    ShowCursor(FALSE);
                    cursorOn = 0;
                }
            } else {
                if (!cursorOn) {
                    ShowCursor(TRUE);
                    cursorOn = 1;
                }
            }
            return res;
        }

    case WM_NCMOUSEMOVE:
        {
            if (!cursorOn) {
                ShowCursor(TRUE);
                cursorOn = 1;
            }
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

    case WM_CLOSE:
        if (!isMainWaitingTermination) {
            if (_W32GenWMEndEvents != GR_GEN_NO) {
                EnqueueW32Event(uMsg, wParam, lParam, 0);
                return 0;
            }
            if (MessageBox(hWnd,
                "This will abort the program\nare you sure?", "Abort",
                MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO ) != IDYES)
                return 0;
        }
        DestroyWindow(hWnd);
        if (!isMainWaitingTermination)
        {
            isWindowThreadRunning = 0;
            ExitProcess(1);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;

            if (!_W32GenWSzChgEvents) {
                lpmmi->ptMaxSize.x = lpmmi->ptMaxTrackSize.x = curWindowWidth;
                lpmmi->ptMaxSize.y = lpmmi->ptMaxTrackSize.y = curWindowHeight;
                lpmmi->ptMinTrackSize.x = curWindowWidth;
                lpmmi->ptMinTrackSize.y = curWindowHeight;
            } else {
                int wo, ho;
                adjustdims(maxScreenWidth, maxScreenHeight, &wo, &ho);
                lpmmi->ptMaxSize.x = lpmmi->ptMaxTrackSize.x = wo;
                lpmmi->ptMaxSize.y = lpmmi->ptMaxTrackSize.y = ho;
                adjustdims(DRVINFO->minwgw, DRVINFO->minwgh, &wo, &ho);
                lpmmi->ptMinTrackSize.x = wo;
                lpmmi->ptMinTrackSize.y = ho;
            }
        }
        return 0;

    case WM_CHAR:
        //printf("char:%d %ld\n", wParam, lParam);
        kbstat = convertwin32keystate();
        EnqueueW32Event(uMsg, wParam, lParam, kbstat);
        return 0;

    case WM_SYSCHAR:
        //printf("syschar:%d %ld\n", wParam, lParam);
        fInsert = FALSE;
        kbstat = convertwin32keystate();
        if (kbstat & GRKBS_ALT) {
            /*if (wParam >= 'a' && wParam <= 'z')
                fInsert = TRUE;
            else if (wParam >= 'A' && wParam <= 'Z')
                fInsert = TRUE;
            else if (wParam >= '0' && wParam <= '9')
                fInsert = TRUE;
            else*/ if (wParam == 13 || wParam == 8) // return or backspace
                fInsert = TRUE;
        }
        if (!fInsert) return 0;
        EnqueueW32Event(uMsg, wParam, lParam, kbstat);
        return 0;

    case WM_KEYDOWN:
        //printf("pure keydown ---\n");
    case WM_SYSKEYDOWN:
        //printf("keydown:%d %ld\n", wParam, lParam);
        kbstat = convertwin32keystate();
        EnqueueW32Event(uMsg, wParam, lParam, kbstat);
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MOUSEWHEEL:
        kbstat = convertwin32keystate();
        EnqueueW32Event(uMsg, wParam, lParam, kbstat);
        return 0;

    case WM_MOUSEMOVE:
        kbstat = convertwin32keystate();
        EnqueueW32Event(uMsg, wParam, lParam, kbstat);
        return 0;

    case WM_PAINT:
        {
            HDC hDC;
            PAINTSTRUCT ps;

            if (!ishDCMemReady) return 0;

            hDC = BeginPaint(hWnd, &ps);
            BitBlt(hDC,
                   ps.rcPaint.left, ps.rcPaint.top,
                   ps.rcPaint.right - ps.rcPaint.left + 1,
                   ps.rcPaint.bottom - ps.rcPaint.top + 1,
                   hDCMem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
            EndPaint(hWnd, &ps);
        }
        return 0;

    case WM_SIZE:
        if (_W32GenWSzChgEvents) {
            int w = LOWORD (lParam);
            int h = HIWORD (lParam);
            if (wParam != 1 && w > 0 && h > 0) { // get rid of minimize events
                EnqueueW32Event(uMsg, wParam, lParam, 0);
            }
            return 0;
        }
        break;

    case WM_SYSCOMMAND:
    case WM_NCCREATE:
    case WM_NCPAINT:
    case WM_PALETTEISCHANGING:
    case WM_ACTIVATEAPP:
    case WM_NCCALCSIZE:
    case WM_ACTIVATE:
    case WM_NCACTIVATE:
    case WM_SHOWWINDOW:
    case WM_WINDOWPOSCHANGING:
    case WM_GETTEXT:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_GETICON:
    case WM_ERASEBKGND:
    case WM_QUERYNEWPALETTE:
    case WM_WINDOWPOSCHANGED:
    case WM_GETDLGCODE:
    case WM_MOVE:
    case WM_SETCURSOR:
    case WM_HELP:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        break;

    default:
/*
        char szMsg[255];
        sprintf(szMsg, "Msg %x, wParam %d, lParam %d",
                uMsg, wParam, lParam);
        MessageBox(NULL, szMsg, "Msg", MB_OK);
*/
        break;

    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
