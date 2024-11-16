%{
#include <stdio.h>
#include "test.tab.h"
void show_error();
%}

digit       [0-9]
ampm        (am)|(pm)|(AM)|(PM)

%%
":"         { return (COLLON); }
{ampm}      { sscanf(yytext, "%s", yylval.ampm); return (AMPM); }
{digit}     { yylval.digit = atoi(yytext); return (DIGIT); }
.           { return (OTHER); }
%%
