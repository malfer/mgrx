/**
 ** fcdlg.c ---- Mini GUI for MGRX, file chooser dialog
 **
 ** Copyright (C) 2024 Mariano Alvarez Fernandez
 ** [e-mail: malfer at telefonica dot net]
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
 ** TODO this file chooser dialog is NOT reentrant
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "grguip.h"
#include "dircont.h"

#define COMMAND_OK           1
#define COMMAND_CANCEL       2

static GUIDialog *fcdlg = NULL;
static GUIGroup *fcgo = NULL;
static int gflags = 0;
static DirContent *glob_dc = NULL;
static FNameComp glob_fnc;

static GUIFileChooserLabels default_fcl = {
    "Directories", "Files", "Dir path", "File name", "Ok", "Cancel",
    "Overwrite file?", "Yes", "No"
};

static GUIFileChooserLabels *fclabels = &default_fcl;

void GUIFileChooserSetLabels(GUIFileChooserLabels *fcl)
{
    if (fcl) fclabels = fcl;
    else fclabels = &default_fcl;
}

static int check_file_exist(void)
{
    int i, ret = 0;
    char *s;

    if (glob_dc == NULL) return 0;
    s = GUIGroupGetText(fcgo, 7, GrGetChrtypeForUserEncoding());
    for (i=0; i<glob_dc->nfiles; i++) {
        if (strcmp(s, glob_dc->files[i]) == 0) {
            ret = 1;
            break;
        }
    }
    free(s);

    return ret;
}

static int process_fcdlg_event(void *udata, GrEvent *ev)
{
    char *newdir;

    GUIDialog *d = (GUIDialog *)udata;
    GUIGroup *go = (GUIGroup *)(d->exdata);

    if (ev->type == GREV_COMMAND) {
        switch (ev->p1) {
            case COMMAND_OK :
                if (gflags & FCDLGF_ASKOVERWRITE && check_file_exist()) {
                    int res = GUICDialogYesNo(fclabels->overwrite, NULL, 0,
                                              fclabels->yes, fclabels->no);
                    if (res != 1) return 1;
                }
                d->result = 1;
               return -1;
            case COMMAND_CANCEL :
                d->result = -1;
                return -1;
        }
    } else if (ev->type == GREV_FCHANGE && ev->p1 == 2) {
        newdir = strdup(glob_dc->dirs[ev->p2]);
        if (newdir == NULL) {
            d->result = -1;
            return -1;
        }
        DestroyDirContent(glob_dc);
        glob_dc = GetDirContent(newdir);
        if (glob_dc == NULL) {
            free(newdir);
            d->result = -1;
            return -1;
        }
        GUIObjectUpdDList(&(go->o[2]), (void **)glob_dc->dirs, glob_dc->ndirs, 0);
        GUIObjectUpdDList(&(go->o[3]), (void **)(glob_dc->files), glob_dc->nfiles, 0);
        GUIGroupSetText(go, 5, glob_dc->path);
        GUIGroupRePaintObject(go, 2);
        GUIGroupRePaintObject(go, 3);
        GUIGroupRePaintObject(go, 5);
        free(newdir);
        return 1;
    } else if (ev->type == GREV_FCHANGE && ev->p1 == 3) {
        GUIGroupSetText(go, 7, glob_dc->files[ev->p2]);
        GUIGroupRePaintObject(go, 7);
        return 1;
    }

    return GUIGroupProcessEvent(go, ev);
}

static int FileChooseDlgCreate(void *title, int width, int height)
{
    int x1, x2, y, fw, colwidth, colheight;

    if (fcdlg) return 1; // dialog already created

    if (width < 300) width = 300;
    if (height < 230) height = 230;
    colwidth = (width - 10) /2;
    colheight = height - 130;
    fcgo = GUIGroupCreate(10, 0, 0);
    if (fcgo == NULL) return 0;

    y = 0; x1 = 0; x2 = colwidth + 10;
    GUIObjectSetLabel(&(fcgo->o[0]), 0, x1, y, colwidth, 30, GrNOCOLOR, _cdlglabelfgcolor, fclabels->dirs);
    GUIObjectSetLabel(&(fcgo->o[1]), 1, x2, y, colwidth, 30, GrNOCOLOR, _cdlglabelfgcolor, fclabels->files);
    y += 30; 
    GUIObjectSetDList(&(fcgo->o[2]), 2, x1, y, colwidth, colheight, _cdlgobjbgcolor, _cdlgobjfgcolor, (void **)NULL, 0, 0);
    GUIObjectSetDList(&(fcgo->o[3]), 3, x2, y, colwidth, colheight, _cdlgobjbgcolor, _cdlgobjfgcolor, (void **)NULL, 0, 0);
    y += colheight + 10;
    fw = 80;
    GUIObjectSetLabel(&(fcgo->o[4]), 4, x1, y, fw, 30, GrNOCOLOR, _cdlglabelfgcolor, fclabels->dirpath);
    fw = colwidth + colwidth - 80;
    x2 = 90;
    GUIObjectSetLabel(&(fcgo->o[5]), 5, x2, y, fw, 30, _cdlgobjbggraycolor, _cdlgobjfgcolor, "");
    y += 40;
    fw = 80;
    GUIObjectSetLabel(&(fcgo->o[6]), 6, x1, y, fw, 30, GrNOCOLOR, _cdlglabelfgcolor, fclabels->fname);
    fw = colwidth + colwidth - 80;
    x2 = 90;
    GUIObjectSetEntry(&(fcgo->o[7]), 7, x2, y, fw, 30, _cdlgobjbgcolor, _cdlgobjfgcolor, FCDLG_MAXLEN_FNAME, "");
    y += 50;
    x1 = (width - 300) / 2;
    x2 = x1 + 140 + 20;
    GUIObjectSetButton(&(fcgo->o[8]), 8, x1, y, 140, 40, _cdlgbtbgcolor, _cdlgbtfgcolor, fclabels->ok, COMMAND_OK, 0, 0);
    GUIObjectSetButton(&(fcgo->o[9]), 9, x2, y, 140, 40, _cdlgbtbgcolor, _cdlgbtfgcolor, fclabels->cancel, COMMAND_CANCEL, 0, 0);
    GUIGroupSetSelected(fcgo, 2, 0);

    fcdlg = GUIGroupDialogCreate(title, fcgo, process_fcdlg_event);
    if (fcdlg == NULL) {
        GUIGroupDestroy(fcgo);
        fcgo = NULL;
        return 0;
    }

    return 1;
}

static void FileChooseDlgDestroy(void)
{
    if (fcdlg) {
        GUIDialogDestroy(fcdlg);
        fcdlg = NULL;
    }
    if (fcgo) {
        GUIGroupDestroy(fcgo);
        fcgo = NULL;
    }
    if (glob_dc) {
        DestroyDirContent(glob_dc);
        glob_dc = NULL;
    }
}

static int FileChooseDlgSetData(char *fname)
{
    if (!fcdlg) return 0;

    SaveCurrentWD();
    strncpy(glob_fnc.fname, fname, DIRC_PATH_MAX);
    glob_fnc.fname[DIRC_PATH_MAX] = '\0';
    DecomposeFileName(&glob_fnc);
    if (glob_dc) DestroyDirContent(glob_dc);
    glob_dc = GetDirContent(glob_fnc.dname);
    if (glob_dc == NULL) {
        FileChooseDlgDestroy();
        fprintf(stderr, "Internal error setting glob_dc");
        return 0;
    }
    GUIObjectUpdDList(&(fcgo->o[2]), (void **)glob_dc->dirs, glob_dc->ndirs, 0);
    GUIObjectUpdDList(&(fcgo->o[3]), (void **)(glob_dc->files), glob_dc->nfiles, 0);
    GUIGroupSetText(fcgo, 5, glob_dc->path);
    GUIGroupSetText(fcgo, 7, glob_fnc.bname);

    return 1;
}

static void FileChooseDlgGetData(char *fname)
{
    char *s;

    if (!fcdlg) return;

    strcpy(glob_fnc.dname, glob_dc->path);
    s = GUIGroupGetText(fcgo, 7, GrGetChrtypeForUserEncoding());
    strcpy(glob_fnc.bname, s);
    free(s);
    ComposeFileName(&glob_fnc);
    strncpy(fname, glob_fnc.fname, FCDLG_MAXLEN_FNAME);
    fname[FCDLG_MAXLEN_FNAME] = '\0';
    //printf("composed %s\n", glob_fnc.fname);
}

int GUICDialogFileChooser(void *title, int width, int height, char *fname, int flags)
{
    int result = -1;

    if (!FileChooseDlgCreate(title, width, height)) return result;

    if (!FileChooseDlgSetData(fname)) {
        FileChooseDlgDestroy();
        return result;
    }

    fcdlg->result = -1;
    gflags = flags;
    //SaveCurrentWD(); // it is done in FileChooseDlgSetData
    result = GUIDialogRun(fcdlg);
    if (result == 0) result = fcdlg->result;
    RestoreCurrentWD();

    if (result == 1) FileChooseDlgGetData(fname);
    FileChooseDlgDestroy();

    return result;
}
