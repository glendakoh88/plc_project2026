#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"

static void trim_newline(char *line)
{
    size_t len;

    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

static void trim_whitespace(char *line)
{
    char *start;
    char *end;
    size_t len;

    if (line == NULL) {
        return;
    }

    start = line;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        line[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    len = (size_t)(end - start + 1);
    memmove(line, start, len);
    line[len] = '\0';
}

static int is_blank_line(const char *line)
{
    return line == NULL || line[0] == '\0';
}

static void to_uppercase_in_place(char *line)
{
    int i;

    if (line == NULL) {
        return;
    }

    for (i = 0; line[i] != '\0'; i++) {
        line[i] = (char)toupper((unsigned char)line[i]);
    }
}

static int parse_brightness(const char *line, FilterCommand *command, int line_number)
{
    char extra[32];
    int value;
    int count;

    count = sscanf(line, "BRIGHTNESS %d %31s", &value, extra);
    if (count != 1) {
        fprintf(stderr, "Line %d: BRIGHTNESS requires exactly 1 integer parameter\n", line_number);
        return 0;
    }

    if (value < -255 || value > 255) {
        fprintf(stderr, "Line %d: BRIGHTNESS value must be between -255 and 255\n", line_number);
        return 0;
    }

    command->type = FILTER_BRIGHTNESS;
    command->param = value;
    return 1;
}

static int parse_threshold(const char *line, FilterCommand *command, int line_number)
{
    char extra[32];
    int value;
    int count;

    count = sscanf(line, "THRESHOLD %d %31s", &value, extra);
    if (count != 1) {
        fprintf(stderr, "Line %d: THRESHOLD requires exactly 1 integer parameter\n", line_number);
        return 0;
    }

    if (value < 0 || value > 255) {
        fprintf(stderr, "Line %d: THRESHOLD value must be between 0 and 255\n", line_number);
        return 0;
    }

    command->type = FILTER_THRESHOLD;
    command->param = value;
    return 1;
}

static int parse_contrast(const char *line, FilterCommand *command, int line_number)
{
    char extra[32];
    int value;
    int count;

    count = sscanf(line, "CONTRAST %d %31s", &value, extra);
    if (count != 1) {
        fprintf(stderr, "Line %d: CONTRAST requires exactly 1 integer parameter\n", line_number);
        return 0;
    }

    if (value < -255 || value > 255) {
        fprintf(stderr, "Line %d: CONTRAST value must be between -255 and 255\n", line_number);
        return 0;
    }

    command->type = FILTER_CONTRAST;
    command->param = value;
    return 1;
}

static int parse_flip(const char *line, FilterCommand *command, int line_number)
{
    char direction[32];
    char extra[32];
    int count;

    count = sscanf(line, "FLIP %31s %31s", direction, extra);
    if (count != 1) {
        fprintf(stderr, "Line %d: FLIP requires exactly 1 parameter (H or V)\n", line_number);
        return 0;
    }

    if (strcmp(direction, "H") == 0) {
        command->type = FILTER_FLIP_H;
        command->param = 0;
        return 1;
    }

    if (strcmp(direction, "V") == 0) {
        command->type = FILTER_FLIP_V;
        command->param = 0;
        return 1;
    }

    fprintf(stderr, "Line %d: FLIP parameter must be H or V\n", line_number);
    return 0;
}

int parse_script(FILE *script_file, FilterCommand commands[], int *command_count)
{
    char line[MAX_LINE_LENGTH];
    int line_number;
    FilterCommand command;

    if (script_file == NULL || commands == NULL || command_count == NULL) {
        return 0;
    }

    line_number = 0;
    *command_count = 0;

    while (fgets(line, sizeof(line), script_file) != NULL) {
        line_number++;

        trim_newline(line);
        trim_whitespace(line);

        if (is_blank_line(line)) {
            continue;
        }

        to_uppercase_in_place(line);

        if (*command_count >= MAX_COMMANDS) {
            fprintf(stderr, "Too many commands in script (maximum %d)\n", MAX_COMMANDS);
            return 0;
        }

        if (strcmp(line, "GRAYSCALE") == 0 || strcmp(line, "GREYSCALE") == 0) {
            command.type = FILTER_GRAYSCALE;
            command.param = 0;
        } else if (strcmp(line, "INVERT") == 0) {
            command.type = FILTER_INVERT;
            command.param = 0;
        } else if (strcmp(line, "SEPIA") == 0) {
            command.type = FILTER_SEPIA;
            command.param = 0;
        } else if (strncmp(line, "BRIGHTNESS", 10) == 0) {
            if (!parse_brightness(line, &command, line_number)) {
                return 0;
            }
        } else if (strncmp(line, "THRESHOLD", 9) == 0) {
            if (!parse_threshold(line, &command, line_number)) {
                return 0;
            }
        } else if (strncmp(line, "CONTRAST", 8) == 0) {
            if (!parse_contrast(line, &command, line_number)) {
                return 0;
            }
        } else if (strncmp(line, "FLIP", 4) == 0) {
            if (!parse_flip(line, &command, line_number)) {
                return 0;
            }
        } else {
            fprintf(stderr, "Line %d: Unknown filter command: %s\n", line_number, line);
            return 0;
        }

        commands[*command_count] = command;
        (*command_count)++;
    }

    return 1;
}

void print_command(const FilterCommand *command, int index)
{
    if (command == NULL) {
        return;
    }

    printf("Command %d: ", index);

    switch (command->type) {
        case FILTER_GRAYSCALE:
            printf("GRAYSCALE\n");
            break;
        case FILTER_INVERT:
            printf("INVERT\n");
            break;
        case FILTER_BRIGHTNESS:
            printf("BRIGHTNESS %d\n", command->param);
            break;
        case FILTER_FLIP_H:
            printf("FLIP H\n");
            break;
        case FILTER_FLIP_V:
            printf("FLIP V\n");
            break;
        case FILTER_SEPIA:
            printf("SEPIA\n");
            break;
        case FILTER_THRESHOLD:
            printf("THRESHOLD %d\n", command->param);
            break;
        case FILTER_CONTRAST:
            printf("CONTRAST %d\n", command->param);
            break;
        default:
            printf("UNKNOWN\n");
            break;
    }
}
