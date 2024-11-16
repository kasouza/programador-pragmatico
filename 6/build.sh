bison -d test.y
flex test.l
cc test.tab.c lex.yy.c -o test -I. -lfl
