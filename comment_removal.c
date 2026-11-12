
#include "comment_removal.h"

int remove_comments(FILE *input, FILE *output)
{
    int ch, next;

    while ((ch = fgetc(input)) != EOF)
    {
        if (ch == '/' )
        {
            next = fgetc(input);

            // Single-line comment
            if (next == '/')
            {
                while ((ch = fgetc(input)) != EOF && ch != '\n')
                    ;
		fputc(' ',output);

                if (ch == '\n')
                    fputc('\n', output);
            }

            // Multi-line comment
            else if (next == '*')
            {
                int prev = 0;
                int closed = 0;

                while ((ch = fgetc(input)) != EOF)
                {
                    if (prev == '*' && ch == '/')
                    {
                        closed = 1;
                        break;
                    }

                    if (ch == '\n')
                        fputc('\n', output);

                    prev = ch;
                }
		fputc(' ',output);

                if (!closed)
                    return -1;
            }

            else
            {
                fputc('/', output);

                if (next != EOF)
                    ungetc(next, input);
            }
        }

        // Preserve strings and characters
        else if (ch == '"' || ch == '\'')
        {
            int quote = ch;
            fputc(ch, output);

            while ((ch = fgetc(input)) != EOF)
            {
                fputc(ch, output);

                if (ch == '\\')
                {
                    next = fgetc(input);

                    if (next != EOF)
                        fputc(next, output);
                }
                else if (ch == quote)
                    break;
            }
        }

        else
        {
            fputc(ch, output);
        }
    }

    return 0;
}

