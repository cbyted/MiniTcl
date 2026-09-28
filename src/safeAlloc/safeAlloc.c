#include "../../include/safeAlloc.h"

/*----------------------------------------------
    ERROR HANDLING
-----------------------------------------------*/

void die(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    fprintf(stderr, "[!] %s\n", buffer);
    exit(EXIT_FAILURE);
}

/*----------------------------------------------
    SAFE DYNAMIC MEMORY ALLOCATION HANDLING
-----------------------------------------------*/

void *xmalloc(size_t size)
{
    void *mem = malloc(size);
    if (!mem && size)
        die("FATAL: malloc(%zu) failed", size);
    return mem;   
    
}

void *xmalloc_try(size_t size) 
{
    void *mem = malloc(size);
    if (!mem && size) 
        return NULL;
    return mem;
}

void *xcalloc(size_t nmemb, size_t size)
{
    if (size && nmemb > SIZE_MAX / size)
        die("Fatal: calloc overflow");
    void *mem = calloc(nmemb, size);
    if (!mem && nmemb && size)
        die("Failed using calloc(%zu, %zu)", nmemb, size);
    return mem;
}

 void *xcalloc_try(size_t nmemb, size_t size) 
 {
    if (size && nmemb > SIZE_MAX / size) 
        return NULL;
    void *mem = calloc(nmemb, size);
    if (!mem && nmemb && size) 
        return NULL;
    return mem;
}

void *xrealloc(void *ptr, size_t size)
{
    void *mem = realloc(ptr, size);
    if (!mem && size != 0)
        die("Failed using realloc(%p, %zu)", ptr, size);
    return mem;
}

void *xrealloc_try(void *ptr, size_t size) 
{
    void *mem = realloc(ptr, size);
    if (!mem && size != 0)
        return NULL;
    return mem;
}

void xfree(void *ptr)
{
    do
    {
        free(ptr);
        ptr = NULL;
    } while (0);
}

/* Bounded str duplicate: copy at most n bytes, n is size-1 (withouth NUL-terminate). */
char *xstrdup(const char *s) 
{
    if (!s) 
    {
        char *z = xmalloc(1);
        z[0] = '\0';
        return z;
    }
    size_t n = strlen(s);
    /* +1 checked overflow */
    if (n >= SIZE_MAX) 
    {
        fprintf(stderr, "FATAL: xstrdup overflow\n");
        abort();
    }

    char *p = xmalloc(n + 1);
    memcpy(p, s, n + 1); /* includes '\0' */
    return p;
}


/* Bounded str duplicate: copy at most n bytes, n is size-1 (withouth NUL-terminate). */
char *xstrndup(const char *s, size_t n) 
{
    if (!s) 
    {
        char *z = xmalloc(1);
        z[0] = '\0';
        return z;
    }
    size_t m = strnlen(s, SIZE_MAX / n);
    if (m >= SIZE_MAX) 
    {
        fprintf(stderr, "FATAL: xstrndup overflow\n");
        abort();
    }
    char *mem = xmalloc(m + 1);
    if (m)
        memcpy(mem, s, m);
    mem[m] = '\0';
    return mem;
}

/* Memcpy helper */
void *xmemcpy(const void *src, size_t n) 
{
    if (!src && n) 
        return NULL;
    void *mem = xmalloc(n ? n : 1);
    if (n)
        memcpy(mem, src, n);
    return mem;
}
