#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim_whitespace(char *str);
static int is_blank_or_comment(const char *str);
static int parse_line(const char *line, int line_number, FilterCommand *command);
static int parse_brightness(const char *line, int line_number, FilterCommand *command);
static int parse_flip(const char *line, int line_number, FilterCommand *command);
static void parser_error(int line_number, const char *message);
static int has_extra_token(const char *line);

void free_filter_script(FilterScript *script)
{
    if (script == NULL)
    {
        return;
    }

    script->count = 0;
}

static void parser_error(int line_number, const char *message)
{
    fprintf(stderr, "Script Error (line %d): %s\n", line_number, message);
}

static void trim_whitespace(char *str)
{
    int start;
    int end;
    int len;
    int i;

    if (str == NULL)
    {
        return;
    }

    len = (int)strlen(str);
    start = 0;
    while (str[start] != '\0' && isspace((unsigned char)str[start]))
    {
        start++;
    }

    end = len - 1;
    while (end >= start && isspace((unsigned char)str[end]))
    {
        end--;
    }

    if (start > 0)
    {
        i = 0;
        while (start + i <= end)
        {
            str[i] = str[start + i];
            i++;
        }
        str[i] = '\0';
    }
    else
    {
        str[end + 1] = '\0';
    }
}

static int is_blank_or_comment(const char *str)
{
    if (str == NULL)
    {
        return 1;
    }

    if (str[0] == '\0')
    {
        return 1;
    }

    if (str[0] == '#')
    {
        return 1;
    }

    return 0;
}

static int has_extra_token(const char *line)
{
    char a[MAX_LINE_LENGTH];
    char b[MAX_LINE_LENGTH];
    char c[MAX_LINE_LENGTH];

    a[0] = '\0';
    b[0] = '\0';
    c[0] = '\0';

    if (sscanf(line, "%255s %255s %255s", a, b, c) == 3)
    {
        return 1;
    }

    return 0;
}

static int parse_brightness(const char *line, int line_number, FilterCommand *command)
{
    char keyword[MAX_LINE_LENGTH];
    int value;
    char extra[MAX_LINE_LENGTH];
    int count;

    keyword[0] = '\0';
    extra[0] = '\0';

    count = sscanf(line, "%255s %d %255s", keyword, &value, extra);
    if (count != 2)
    {
        parser_error(line_number, "brightness requires exactly one integer parameter");
        return 0;
    }

    if (value < -255 || value > 255)
    {
        parser_error(line_number, "brightness value must be between -255 and 255");
        return 0;
    }

    command->type = FILTER_BRIGHTNESS;
    command->int_value = value;
    return 1;
}

static int parse_flip(const char *line, int line_number, FilterCommand *command)
{
    char keyword[MAX_LINE_LENGTH];
    char direction[MAX_LINE_LENGTH];
    char extra[MAX_LINE_LENGTH];
    int count;

    keyword[0] = '\0';
    direction[0] = '\0';
    extra[0] = '\0';

    count = sscanf(line, "%255s %255s %255s", keyword, direction, extra);
    if (count != 2)
    {
        parser_error(line_number, "flip requires exactly one parameter: horizontal or vertical");
        return 0;
    }

    if (strcmp(direction, "horizontal") == 0)
    {
        command->type = FILTER_FLIP_HORIZONTAL;
        command->int_value = 0;
        return 1;
    }

    if (strcmp(direction, "vertical") == 0)
    {
        command->type = FILTER_FLIP_VERTICAL;
        command->int_value = 0;
        return 1;
    }

    parser_error(line_number, "flip parameter must be 'horizontal' or 'vertical'");
    return 0;
}

static int parse_line(const char *line, int line_number, FilterCommand *command)
{
    char keyword[MAX_LINE_LENGTH];
    int count;

    keyword[0] = '\0';
    count = sscanf(line, "%255s", keyword);

    if (count != 1)
    {
        parser_error(line_number, "empty or invalid command");
        return 0;
    }

    if (strcmp(keyword, "grayscale") == 0)
    {
        if (has_extra_token(line))
        {
            parser_error(line_number, "grayscale takes no parameters");
            return 0;
        }

        command->type = FILTER_GRAYSCALE;
        command->int_value = 0;
        return 1;
    }

    if (strcmp(keyword, "invert") == 0)
    {
        if (has_extra_token(line))
        {
            parser_error(line_number, "invert takes no parameters");
            return 0;
        }

        command->type = FILTER_INVERT;
        command->int_value = 0;
        return 1;
    }

    if (strcmp(keyword, "brightness") == 0)
    {
        return parse_brightness(line, line_number, command);
    }

    if (strcmp(keyword, "flip") == 0)
    {
        return parse_flip(line, line_number, command);
    }

    parser_error(line_number, "unknown filter command");
    return 0;
}

int parse_filter_script(const char *filename, FilterScript *script)
{
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    int line_number;
    size_t len;

    if (filename == NULL || script == NULL)
    {
        fprintf(stderr, "Script Error: invalid arguments to parse_filter_script\n");
        return 0;
    }

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Script Error: could not open script file\n");
        return 0;
    }

    script->count = 0;
    line_number = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line_number++;

        len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        {
            line[len - 1] = '\0';
            len--;
        }

        trim_whitespace(line);

        if (is_blank_or_comment(line))
        {
            continue;
        }

        if (script->count >= MAX_FILTERS)
        {
            fprintf(stderr, "Script Error: too many filter commands (max %d)\n", MAX_FILTERS);
            fclose(fp);
            return 0;
        }

        if (!parse_line(line, line_number, &script->commands[script->count]))
        {
            fclose(fp);
            return 0;
        }

        script->count++;
    }

    if (ferror(fp))
    {
        fprintf(stderr, "Script Error: failed while reading script file\n");
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}