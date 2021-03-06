#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "mkutils.h"

const char * TXTFMT =
"#ifndef _%s_H\n"
"#define _%s_H\n"
"\n"
"\n"
"#endif /* _%s_H */\n";

int mkheader (char * name)
{
    int ret = 0;
    FILE * f = NULL;
    char * tmp = NULL;

    ifjmp(name == NULL, ko);

    size_t len = strlen(name) + 1;
    tmp = calloc(len + 2, sizeof(char));

    ABORT(ko, tmp == NULL,
          "`%s`:Could not allocate memory, do nothing!\n",
          name);

    sprintf(tmp, "%s.h", name);

    ABORT(ko, file_exists(tmp),
          "`%s.h`:File already exists, do nothing!\n",
          name);

    f = fopen(tmp, "w");
    ABORT(ko, f == NULL,
          "`%s.h`:Could not open file for writing, do nothing!\n",
          name);

    for (size_t i = 0; i < len; i++)
        tmp[i] = (char) toupper(tmp[i]);
    tmp[len - 1] = '\0';

    ABORT(ko, fprintf(f, TXTFMT, tmp, tmp, tmp) < 0,
          "`%s.h`:An error occurred while writing to file!\n",
          name);

out:
    ifnotnull(f, fclose);
    ifnotnull(tmp, free);
    return ret;
ko:
    ret = 1;
    goto out;
}

int main (int argc, char ** argv)
{
   ABORT(ko, argc < 2, "Usage:\n\t%s HEADER...\n", argv[0]);
    int ret = 0;

    for (int i = 1; i < argc; i++)
        ret += mkheader(argv[i]);

    return ret;
ko:
    return EXIT_FAILURE;
}
