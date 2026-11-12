
#include "macro_handler.h"
#include <string.h>
#include <ctype.h>

#define MAX_MACROS 100
#define MAX_NAME 64
#define MAX_VALUE 512
#define MAX_PARAMS 10
#define MAX_OUTPUT 8192

struct macro
{
    char name[MAX_NAME];
    char value[MAX_VALUE];
    int is_function;
    int param_count;
    char params[MAX_PARAMS][MAX_NAME];
};

static struct macro macros[MAX_MACROS];
static int macro_count = 0;

void trim(char *str)
{
    int i, len;

    i = 0;

    while (isspace((unsigned char)str[i]))
        i++;

    if (i > 0)
        memmove(str, str + i, strlen(str + i) + 1);

    len = strlen(str);

    while (len > 0 && isspace((unsigned char)str[len - 1]))
        str[--len] = '\0';
}

int append_text(char *dest, int size, const char *src)
{
    if (strlen(dest) + strlen(src) >= size)
        return -1;

    strcat(dest, src);
    return 0;
}

int is_start(char ch)
{
    return isalpha((unsigned char)ch) || ch == '_';
}

int is_part(char ch)
{
    return isalnum((unsigned char)ch) || ch == '_';
}

struct macro *find_macro(char *name)
{
    int i;

    for (i = 0; i < macro_count; i++)
    {
        if (strcmp(macros[i].name, name) == 0)
            return &macros[i];
    }

    return NULL;
}

int parse_definition(char *line)
{
    char *p, *end;
    char params_text[MAX_VALUE];
    char *token;

    p = line;

    while (isspace((unsigned char)*p))
        p++;

    if (strncmp(p, "#define", 7) != 0 ||
        !isspace((unsigned char)p[7]))
        return 0;

    p += 7;

    while (isspace((unsigned char)*p))
        p++;

    if (!is_start(*p))
        return -1;

    if (macro_count >= MAX_MACROS)
        return -1;

    end = p;

    while (is_part(*end))
        end++;

    macros[macro_count].is_function = 0;
    macros[macro_count].param_count = 0;

    if ((size_t)(end - p) >= MAX_NAME)
        return -1;

    strncpy(macros[macro_count].name, p, end - p);
    macros[macro_count].name[end - p] = '\0';

    p = end;

    if (*p == '(')
    {
        macros[macro_count].is_function = 1;
        p++;

        end = strchr(p, ')');

        if (end == NULL)
            return -1;

        if ((size_t)(end - p) >= MAX_VALUE)
            return -1;

        strncpy(params_text, p, end - p);
        params_text[end - p] = '\0';

        if (strlen(params_text) > 0)
        {
            token = strtok(params_text, ",");

            while (token != NULL)
            {
                if (macros[macro_count].param_count >= MAX_PARAMS)
                    return -1;

                trim(token);

                if (strlen(token) >= MAX_NAME)
                    return -1;

                strcpy(macros[macro_count].params
                       [macros[macro_count].param_count], token);

                macros[macro_count].param_count++;

                token = strtok(NULL, ",");
            }
        }

        p = end + 1;
    }

    while (isspace((unsigned char)*p))
        p++;

    if (strlen(p) >= MAX_VALUE)
        return -1;

    strcpy(macros[macro_count].value, p);

    {
        int len = strlen(macros[macro_count].value);

        while (len > 0 &&
               (macros[macro_count].value[len - 1] == '\n' ||
                macros[macro_count].value[len - 1] == '\r'))
        {
            macros[macro_count].value[--len] = '\0';
        }
    }

    macro_count++;

    return 1;
}

int parse_arguments(const char **p,
                    char args[MAX_PARAMS][MAX_VALUE],
                    int *argc)
{
    int depth = 0;
    int len = 0;
    int quote = 0;
    int i;

    *argc = 0;
    (*p)++;

    if (**p == ')')
    {
        (*p)++;
        return 0;
    }

    for (;;)
    {
        char ch = **p;

        if (ch == '\0')
            return -1;

        if (quote)
        {
            if (len >= MAX_VALUE - 1)
                return -1;

            args[*argc][len++] = ch;

            if (ch == '\\' && (*p)[1] != '\0')
            {
                if (len >= MAX_VALUE - 1)
                    return -1;

                args[*argc][len++] = (*p)[1];
                (*p) += 2;
                continue;
            }

            if (ch == quote)
                quote = 0;

            (*p)++;
            continue;
        }

        if (ch == '"' || ch == '\'')
        {
            quote = ch;
        }
        else if (ch == '(')
        {
            depth++;
        }
        else if (ch == ')' && depth == 0)
        {
            args[*argc][len] = '\0';
            trim(args[*argc]);
            (*argc)++;
            (*p)++;

            return 0;
        }
        else if (ch == ')' && depth > 0)
        {
            depth--;
        }
        else if (ch == ',' && depth == 0)
        {
            args[*argc][len] = '\0';
            trim(args[*argc]);

            (*argc)++;

            if (*argc >= MAX_PARAMS)
                return -1;

            len = 0;
            (*p)++;
            continue;
        }

        if (len >= MAX_VALUE - 1)
            return -1;

        args[*argc][len++] = ch;
        (*p)++;
    }
}

int substitute_params(char *body,
                      struct macro *m,
                      char args[MAX_PARAMS][MAX_VALUE],
                      char *result)
{
    char name[MAX_NAME];
    char *p = body;
    int i, found;

    result[0] = '\0';

    while (*p)
    {
        if (is_start(*p))
        {
            i = 0;

            while (is_part(*p))
            {
                if (i >= MAX_NAME - 1)
                    return -1;

                name[i++] = *p++;
            }

            name[i] = '\0';
            found = 0;

            for (i = 0; i < m->param_count; i++)
            {
                if (strcmp(name, m->params[i]) == 0)
                {
                    if (append_text(result, MAX_OUTPUT,
                                    args[i]) == -1)
                        return -1;

                    found = 1;
                    break;
                }
            }

            if (!found &&
                append_text(result, MAX_OUTPUT, name) == -1)
                return -1;
        }
        else
        {
            char temp[2] = {*p++, '\0'};

            if (append_text(result, MAX_OUTPUT, temp) == -1)
                return -1;
        }
    }

    return 0;
}

int expand_line(char *line, char *result)
{
    char name[MAX_NAME];
    char args[MAX_PARAMS][MAX_VALUE];
    char replacement[MAX_OUTPUT];

    const char *p = line;
    struct macro *m;
    int i, argc;

    result[0] = '\0';

    while (*p)
    {
        if (is_start(*p))
        {
            i = 0;

            while (is_part(*p))
            {
                if (i >= MAX_NAME - 1)
                    return -1;

                name[i++] = *p++;
            }

            name[i] = '\0';
            m = find_macro(name);

            if (m == NULL)
            {
                if (append_text(result, MAX_OUTPUT, name) == -1)
                    return -1;
            }
            else if (!m->is_function)
            {
                if (append_text(result, MAX_OUTPUT,
                                m->value) == -1)
                    return -1;
            }
            else
            {
                const char *q = p;

                while (isspace((unsigned char)*q))
                    q++;

                if (*q == '(' &&
                    parse_arguments(&q, args, &argc) == 0 &&
                    argc == m->param_count)
                {
                    if (substitute_params(m->value, m,
                                          args, replacement) == -1)
                        return -1;

                    if (append_text(result, MAX_OUTPUT,
                                    replacement) == -1)
                        return -1;

                    p = q;
                }
                else
                {
                    if (append_text(result, MAX_OUTPUT, name) == -1)
                        return -1;
                }
            }
        }
        else
        {
            char temp[2] = {*p++, '\0'};

            if (append_text(result, MAX_OUTPUT, temp) == -1)
                return -1;
        }
    }

    return 0;
}

int process_macros(FILE *input, FILE *output)
{
    char line[1024];
    char result[MAX_OUTPUT];
    int status;

    macro_count = 0;

    while (fgets(line, sizeof(line), input) != NULL)
    {
        status = parse_definition(line);

        if (status == -1)
        {
            printf("Invalid macro definition\n");
            return -1;
        }

        if (status == 1)
            continue;

        if (expand_line(line, result) == -1)
        {
            printf("Macro expansion too large\n");
            return -1;
        }

        fputs(result, output);
    }

    return 0;
}

