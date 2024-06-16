/**
 ** dircont.h ---- Mini GUI for MGRX, directory utilities
 **
 ** Copyright (C) 2019 Mariano Alvarez Fernandez
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
 **/

#define DIRC_PATH_MAX 1024
#define DIRC_MAX_DIRS 1000
#define DIRC_MAX_FILES 1000

typedef struct _DirContent {
    char *dirs[DIRC_MAX_DIRS];
    int ndirs;
    char *files[DIRC_MAX_DIRS];
    int nfiles;
    char path[DIRC_PATH_MAX+1];
} DirContent;

DirContent *NewDirContent(void);
void DestroyDirContent(DirContent *dc);
DirContent *GetDirContent(char *dirname);

typedef struct _FNameComp{
    char fname[DIRC_PATH_MAX+1];
    char dname[DIRC_PATH_MAX+1];
    char bname[DIRC_PATH_MAX+1];
} FNameComp;

void SaveCurrentWD(void);
void RestoreCurrentWD(void);
void DecomposeFileName(FNameComp *fnc);
void ComposeFileName(FNameComp *fnc);
