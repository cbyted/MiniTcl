#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <stdint.h>

/* Allocation error handling */
void die(const char *fmt, ...);  

/* Dynamic Memory Allocation */
void *xmalloc(size_t size);
void *xmalloc_try(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xcalloc_try(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
void *xrealloc_try(void *ptr, size_t size);
void xfree(void *ptr);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);
void *xmemcpy(const void *src, size_t n);