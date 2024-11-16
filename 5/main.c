#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define MAX_PEN_SIZE 4

int min(int a, int b) { return a < b ? a : b; }

int max(int a, int b) { return a > b ? a : b; }

int clamp(int n, int min_val, int max_val) {
    return max(min_val, min(max_val, n));
}

typedef enum {
    TOKEN_TYPE_COMMAND,
    TOKEN_TYPE_ARGUMENT,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_SPACE,
} TokenType;

typedef enum { PEN_STATE_DOWN, PEN_STATE_UP } PenState;

typedef enum {
    COMMAND_TYPE_NOOP = 0,
    COMMAND_PEN = 'P',
    COMMAND_DOWN = 'D',
    COMMAND_UP = 'U',

    COMMAND_EAST = 'E',
    COMMAND_WEST = 'W',
    COMMAND_NORTH = 'N',
    COMMAND_SOUTH = 'S',

    COMMAND_CHAR = 'C',

    COMMAND_RESET_PEN = 'R',
} CommandType;

typedef struct {
    int type;
    int value;
    int len;
} Token;

typedef struct {
    CommandType type;
    int argument;
} Command;

typedef struct {
    int x;
    int y;
    int size;
    PenState state;
    char character;
} Pen;

typedef struct {
    int width;
    int height;

    char *buffer;
} Canvas;

typedef struct {
    Token *tokens;
    int tokens_len;
} ParseResult;

typedef struct {
    Command *commands;
    int commands_len;
} Program;

int calculate_2d_index(int x, int y, int width) { return (x * width) + y; }

char *read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);

    long bytes_len = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    char *content = malloc(bytes_len + 1);
    fread(content, 1, bytes_len, fp);

    content[bytes_len] = '\0';

    return content;
}

ParseResult *parse(char *text) {
    int len = strlen(text);
    if (len == 0) {
        return NULL;
    }

    int idx = 0;
    char ch = text[idx];

    Token *tokens = malloc(sizeof(Token) * len);
    int tokens_len = 0;

    while (ch != '\0') {
        Token token;
        token.len = 1;

        switch (ch) {
        case COMMAND_PEN:
        case COMMAND_DOWN:
        case COMMAND_UP:
        case COMMAND_EAST:
        case COMMAND_WEST:
        case COMMAND_NORTH:
        case COMMAND_SOUTH:
        case COMMAND_CHAR:
        case COMMAND_RESET_PEN:
            token.type = TOKEN_TYPE_COMMAND;
            token.value = ch;
            break;

        default:
            if (isdigit(ch)) {
                int digit_idx = idx;
                int value = 0;
                char digit_ch = text[digit_idx];

                while (isdigit(digit_ch)) {
                    int digit_value = digit_ch - '0';
                    value = value * 10 + digit_value;

                    digit_idx++;
                    digit_ch = text[digit_idx];
                }

                token.type = TOKEN_TYPE_ARGUMENT;
                token.len = digit_idx - idx;
                token.value = value;

            } else if (isspace(ch)) {
                int space_idx = idx;
                while (isspace(text[space_idx])) {
                    space_idx++;
                }

                token.type = TOKEN_TYPE_SPACE;
                token.len = space_idx - idx;
            } else if (ch == '#') {
                int comment_idx = idx;
                while (
                    !(text[comment_idx] == '\n' || text[comment_idx] == '\0')) {
                    comment_idx++;
                    ch = text[idx];
                }

                token.type = TOKEN_TYPE_COMMENT;
                token.len = comment_idx - idx;
            } else if (ch == '\\') {
                if (idx + 1 >= len) {
                    fprintf(stderr, "Invalid syntax near \"%c\"", ch);
                    free(tokens);
                    return NULL;
                }

                char next_ch = text[idx + 1];
                if (iscntrl(next_ch)) {
                    fprintf(stderr, "Invalid syntax near \"%c\"", ch);
                    free(tokens);
                    return NULL;
                }

                token.type = TOKEN_TYPE_ARGUMENT;
                token.value = next_ch;
                token.len = 2;

            } else {
                fprintf(stderr, "Invalid syntax near \"%c\"", ch);
                free(tokens);
                return NULL;
            }
        }

        if (token.type != TOKEN_TYPE_COMMENT) {
            tokens[tokens_len] = token;
            tokens_len++;
        }

        idx += token.len;
        ch = text[idx];
    }

    ParseResult *parse_result = malloc(sizeof(ParseResult));
    parse_result->tokens = tokens;
    parse_result->tokens_len = tokens_len;

    return parse_result;
}

void free_parse_result(ParseResult *result) {
    assert(result != NULL);

    if (result->tokens != NULL) {
        free(result->tokens);
    }

    free(result);
}

Program *build_program(ParseResult *result) {
    Command *commands = malloc(sizeof(Command) * result->tokens_len);
    int commands_len = 0;

    Command current_command;
    current_command.type = COMMAND_TYPE_NOOP;
    current_command.argument = 0;

    for (int i = 0; i < result->tokens_len; i++) {
        Token token = result->tokens[i];

        if (token.type == TOKEN_TYPE_COMMAND) {
            if (current_command.type != COMMAND_TYPE_NOOP) {
                fprintf(stderr, "Command missing argument %c\n",
                        current_command.type);
                free(commands);
                return NULL;
            }

            current_command.type = token.value;
            current_command.argument = 0;

            if (current_command.type == COMMAND_DOWN ||
                current_command.type == COMMAND_UP ||
                current_command.type == COMMAND_RESET_PEN) {
                commands[commands_len] = current_command;
                commands_len++;

                current_command.type = COMMAND_TYPE_NOOP;
            }

        } else if (token.type == TOKEN_TYPE_ARGUMENT) {
            if (current_command.type == COMMAND_TYPE_NOOP) {
                fprintf(stderr, "No command found for argument %d\n",
                        token.value);
                free(commands);
                return NULL;
            }

            current_command.argument = token.value;
            commands[commands_len] = current_command;
            commands_len++;

            current_command.type = COMMAND_TYPE_NOOP;
        }
    }

    if (current_command.type != COMMAND_TYPE_NOOP) {
        fprintf(stderr, "Command missing argument %c\n", current_command.type);
        free(commands);
        return NULL;
    }

    Program *program = malloc(sizeof(Program));
    program->commands = commands;
    program->commands_len = commands_len;

    return program;
}

void free_program(Program *result) {
    assert(result != NULL);

    if (result->commands) {
        free(result->commands);
    }

    free(result);
}

void run_program(Program *program) {
    Pen pen = {0};
    pen.character = '.';
    pen.state = PEN_STATE_UP;

    Canvas canvas = {0};
    canvas.width = 41;
    canvas.height = 41;

    canvas.buffer = malloc(canvas.width * canvas.height);

    for (int i = 0; i < canvas.width * canvas.height; i++) {
        canvas.buffer[i] = ' ';
    }

    for (int i = 0; i < program->commands_len; i++) {
        Command cmd = program->commands[i];

        int dir_x = 0;
        int dir_y = 0;

        switch (cmd.type) {
        case COMMAND_PEN:
            if (cmd.argument > MAX_PEN_SIZE) {
                fprintf(stderr, "Invalid pen %d. Max pen size is %d\n",
                        cmd.argument, MAX_PEN_SIZE);
            } else {
                pen.size = cmd.argument;
            }

            break;

        case COMMAND_RESET_PEN:
            pen.x = 0;
            pen.y = 0;
            break;

        case COMMAND_UP:
            pen.state = PEN_STATE_UP;
            break;

        case COMMAND_DOWN:
            pen.state = PEN_STATE_DOWN;
            break;

        case COMMAND_EAST:
            dir_x = cmd.argument;
            break;

        case COMMAND_WEST:
            dir_x = -cmd.argument;
            break;

        case COMMAND_SOUTH:
            dir_y = cmd.argument;
            break;

        case COMMAND_NORTH:
            dir_y = -cmd.argument;
            break;

        case COMMAND_CHAR:
            pen.character = cmd.argument;
            break;

        case COMMAND_TYPE_NOOP:
            break;
        }

        if (pen.state == PEN_STATE_DOWN) {
            int idx = calculate_2d_index(pen.x, pen.y, canvas.width);
            canvas.buffer[idx] = pen.character;

            for (int offset_x = 1; offset_x <= abs(dir_x); offset_x++) {
                int dir = dir_x > 0 ? 1 : -1;
                int new_x = clamp(pen.x + (offset_x * dir), 0, canvas.width);

                int idx = calculate_2d_index(new_x, pen.y, canvas.width);
                canvas.buffer[idx] = pen.character;
            }

            for (int offset_y = 1; offset_y <= abs(dir_y); offset_y++) {
                int dir = dir_y > 0 ? 1 : -1;
                int new_y = clamp(pen.y + (offset_y * dir), 0, canvas.height);

                int idx = calculate_2d_index(pen.x, new_y, canvas.width);
                canvas.buffer[idx] = pen.character;
            }
        }

        pen.x = clamp(pen.x + dir_x, 0, canvas.width - 1);
        pen.y = clamp(pen.y + dir_y, 0, canvas.height - 1);
    }

    printf("\n------\n\n");

    for (int y = 0; y < canvas.height; y++) {
        for (int x = 0; x < canvas.width; x++) {
            int idx = calculate_2d_index(x, y, canvas.width);
            printf("%c", canvas.buffer[idx]);
        }
        printf("\n");
    }

    free(canvas.buffer);
    canvas.buffer = NULL;
}

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    char *content = read_file("test.turtle");
    ParseResult *result = parse(content);
    if (result == NULL) {
        return 1;
    }

    Program *program = build_program(result);
    if (program == NULL) {
        return 1;
    }

    free_parse_result(result);

    run_program(program);
    free_program(program);

    return 0;
}
