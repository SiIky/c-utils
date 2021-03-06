#include <stdio.h>
#include <stdlib.h>

#define TRALLOC_CFG_TRACE
#define TRALLOC_CFG_IMPLEMENTATION
#include <utils/tralloc.h>

static void * g_trash = NULL;

void test1 (size_t nmemb)
{
    trcalloc(nmemb, 32);
}

void test2 (size_t nmemb)
{
    static void * trash = NULL;
    trash = trrealloc(trash, nmemb * 16);
}

void test3 (size_t nmemb)
{
    g_trash = trmalloc(32 * nmemb);
}

void test4 (size_t nmemb)
{
    trrealloc(g_trash, 64 * nmemb);
}

int main (void)
{
    FILE * outf = fopen("trace.txt", "w");
    outf = (outf == NULL) ?
        stderr :
        outf;

    for (size_t i = 0; i <= 1000; i++) {
        test1(i);
        test2(i);
        test3(i);
        test4(i);
    }

    int ret = (trfprint(outf) == 0) ?
        EXIT_SUCCESS :
        EXIT_FAILURE;

    return trdeinit(), ret;
}
