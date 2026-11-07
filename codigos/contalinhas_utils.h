#ifndef CONTALINHAS_UTILS_H
#define CONTALINHAS_UTILS_H
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <omp.h>

static long long count_lines_in_file(const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) { fprintf(stderr, "Aviso: impossivel abrir %s: %s\n", filepath, strerror(errno)); return 0; }
    /* Conforme o PDF, contamos apenas os caracteres '\n'. */
    long long count = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF)
        if (ch == '\n') ++count;
    fclose(fp);
    return count;
}

static int is_source_file(const char *name) {
    size_t len = strlen(name);
    return len > 2 && (!strcmp(name + len - 2, ".c") || !strcmp(name + len - 2, ".h"));
}

static char *join_path(const char *dir, const char *file) {
    size_t a = strlen(dir), b = strlen(file);
    if (a > SIZE_MAX - b - 2) return NULL;
    char *path = malloc(a + b + 2);
    if (!path) return NULL;
    memcpy(path, dir, a);
    path[a] = '/';
    memcpy(path + a + 1, file, b + 1);
    return path;
}
#endif
