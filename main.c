
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comment_removal.h"
#include "file_inclusion.h"
#include "macro_handler.h"

int main(int argc, char *argv[])
{
    FILE *fp1, *fp2, *fp3, *fp4;
    char output_file[256];

    if (argc != 2)
    {
        printf("Usage: ./mypreprocessor inputfile.c\n");
        return 1;
    }

    snprintf(output_file, sizeof(output_file), "%s", argv[1]);

    char *dot = strrchr(output_file, '.');

    if (dot != NULL)
        *dot = '\0';

    strcat(output_file, ".i");

    fp1 = fopen(argv[1], "r");

    if (fp1 == NULL)
    {
        printf("Unable to open input file\n");
        return 1;
    }

    fp2 = fopen("temp1.c", "w");

    if (fp2 == NULL)
    {
        printf("Unable to create temporary file\n");
        fclose(fp1);
        return 1;
    }

    if (remove_comments(fp1, fp2) == -1)
    {
        printf("Comment removal failed\n");
        fclose(fp1);
        fclose(fp2);
        return 1;
    }

    fclose(fp1);
    fclose(fp2);

    fp2 = fopen("temp1.c", "r");
    fp3 = fopen("temp2.c", "w");

    if (fp2 == NULL || fp3 == NULL)
    {
        printf("File opening failed\n");
        return 1;
    }

    if (process_include(fp2, fp3) == -1)
    {
        printf("File inclusion failed\n");
        fclose(fp2);
        fclose(fp3);
        return 1;
    }

    fclose(fp2);
    fclose(fp3);

    fp3 = fopen("temp2.c", "r");
    fp4 = fopen(output_file, "w");

    if (fp3 == NULL || fp4 == NULL)
    {
        printf("File opening failed\n");
        return 1;
    }

    if (process_macros(fp3, fp4) == -1)
    {
        printf("Macro substitution failed\n");
        fclose(fp3);
        fclose(fp4);
        return 1;
    }

    fclose(fp3);
    fclose(fp4);

    remove("temp1.c");
    remove("temp2.c");

    printf("Preprocessing completed successfully\n");
    printf("Output file: %s\n", output_file);

    return 0;
}

