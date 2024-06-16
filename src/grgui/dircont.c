/**
 ** dircont.c ---- Mini GUI for MGRX, directory utilities
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
// in DJGPP dirname and basename are defined in unistd.h
#if !defined(__MSDOS__)
#include <libgen.h>
#endif
#include "dircont.h"

#if defined(__MSDOS__)
// DJGPP usually returns '/' as dir separator, sometimes '\'
#define CSLASH '/'
#define CSLASH2 '\\'
#elif defined(__WIN32__)
// WIN32 returns '\' as dir separator
#define CSLASH '\\'
#define CSLASH2 '\0'
#else
// Linux returns '/' as dir separator
#define CSLASH '/'
#define CSLASH2 '\0'
#endif

// This functions use posix libraries, it works with DJGPP and MingW too

DirContent *NewDirContent(void)
{
    DirContent *dc;

    dc = malloc(sizeof(DirContent));
    if (dc == NULL) return NULL;
    dc->ndirs = 0;
    dc->nfiles = 0;
    dc->path[0] = '\0';

    return dc;
}

void DestroyDirContent(DirContent *dc)
{
    int i;

    for (i=0; i<dc->ndirs; i++) free(dc->dirs[i]);
    for (i=0; i<dc->nfiles; i++) free(dc->files[i]);
    free(dc);
}

static int compstr(const void *str1, const void *str2)
{
    return strcmp(*(char **)str1, *(char **)str2);
}

DirContent *GetDirContent(char *dirname)
{
    struct stat file_stat;
    DirContent *dc;
    DIR *dir;
    struct dirent *dirfile;

    if (stat(dirname, &file_stat) != 0) return NULL; // doesn't exist
    if (!S_ISDIR(file_stat.st_mode)) return NULL; // not a directory
    chdir(dirname);
    dir = opendir(".");
    if (dir == NULL) return NULL;

    dc = NewDirContent();
    if (dc == NULL) goto end;

    getcwd(dc->path, DIRC_PATH_MAX);
    dc->path[DIRC_PATH_MAX] = '\0';

    while ((dirfile = readdir(dir))) {
        if (stat(dirfile->d_name, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                if (dc->ndirs < DIRC_MAX_DIRS) {
                    dc->dirs[dc->ndirs] = strdup(dirfile->d_name);
                    dc->ndirs++;
                }
            } else {
                if (dc->nfiles < DIRC_MAX_FILES) {
                    dc->files[dc->nfiles] = strdup(dirfile->d_name);
                    dc->nfiles++;
                }
            }
        }
    }
    qsort(dc->files, dc->nfiles, sizeof(char *), compstr);
    qsort(dc->dirs, dc->ndirs, sizeof(char *), compstr);
    //printf("%s %d %d %s\n", dirname, dc->ndirs, dc->nfiles, dc->path);

end:
    closedir(dir);
    return dc;
}

static char homewd[DIRC_PATH_MAX+1] = "\0";

void SaveCurrentWD(void)
{
    getcwd(homewd, DIRC_PATH_MAX);
    homewd[DIRC_PATH_MAX] = '\0';
}

void RestoreCurrentWD(void)
{
    chdir(homewd);
}

void DecomposeFileName(FNameComp *fnc)
{
    char *dn, *bn, *c1, *c2;

    c1 = strdup(fnc->fname);
    c2 = strdup(fnc->fname);
    dn = dirname(c1);
    bn = basename(c2);
    strcpy(fnc->dname, dn);
    strcpy(fnc->bname, bn);
    free(c1);
    free(c2);
}

void ComposeFileName(FNameComp *fnc)
{
    char aux[DIRC_PATH_MAX*2+4]; // we don't want overbounds
    int l;

    l = strlen(homewd);
    if (strncmp(fnc->dname, homewd, l) != 0) l = 0; // in the home dir
    if (l > 0 && strlen(&(fnc->dname[l])) > 0) l++; // remove slash
    strcpy(aux, &(fnc->dname[l]));
    l = strlen(aux);
    if (CSLASH2 != '\0') {
        if (l > 0 && aux[l-1] != CSLASH && aux[l-1] != CSLASH2) {
            aux[l] = CSLASH;
            aux[l+1] = '\0';
        }
    } else {
        if (l > 0 && aux[l-1] != CSLASH) {
            aux[l] = CSLASH;
            aux[l+1] = '\0';
        }
    }
    strcat(aux, fnc->bname);
    strncpy(fnc->fname, aux, DIRC_PATH_MAX);
    fnc->fname[DIRC_PATH_MAX] = '\0';
}
