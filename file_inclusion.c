
#include "file_inclusion.h"
#include <string.h>

int process_include(FILE *input, FILE *output)
{
    char line[1024];
    char filename[256];
    FILE *fp;
    int ch;

    while (fgets(line, sizeof(line), input) != NULL)
    {
        if (sscanf(line, " #include <%255[^>]>", filename) == 1 ||
            sscanf(line, " #include \"%255[^\"]\"", filename) == 1)
        {
            fp = fopen(filename, "r");

            if (fp == NULL)
            {
                printf("Unable to open header file: %s\n", filename);
                return -1;
            }

            while ((ch = fgetc(fp)) != EOF)
            {
                fputc(ch, output);
            }

            fclose(fp);
        }
        else
        {
            fputs(line, output);
        }
    }

    return 0;
}

