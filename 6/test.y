%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int yyerror(char *s);
int yylex();

int hour = 0;
int minutes = 0;
char collon[2] = "";
char ampm[3] = "";

%}

%token DIGIT AMPM COLLON OTHER

%type <digit> DIGIT
%type <ampm> AMPM

%union {
    int digit;
    char ampm[3];
}

%%

prog:
    times

times:
    | time times

time:
    hour
    | hour AMPM {
        strncpy(ampm, $2, 2);
    }
    | hour COLLON minutes {
        strncpy(collon, ":", 1);
    }
    | hour COLLON minutes AMPM {
        strncpy(collon, ":", 1);
        strncpy(ampm, $4, 2);
    }
;

hour:
    DIGIT DIGIT {
        hour = $1 * 10 + $2;
    }

minutes:
    DIGIT DIGIT {
        minutes = $1 * 10 + $2;
    }

%%

int yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
    return 0;
}

int main() {
    yyparse();

    if (hour < 0 || hour > 24) {
        perror("Horá inválida");
        exit(-1);
    }

    if (strlen(ampm) > 0 && hour > 12) {
        perror("Hora inválida");
        exit(-1);
    }


    if (minutes < 0 || minutes > 59) {
        perror("Minutos inválidos");
        exit(-1);
    }

    char formatted_hour[20];
    if (strlen(collon) > 0) {
        snprintf(formatted_hour, 20, "%02d%s%02d%s", hour, collon, minutes, ampm);
    } else {
        snprintf(formatted_hour, 20, "%02d%s", hour, ampm);
    }
    printf("%s\n", formatted_hour);

    return 0;
}
