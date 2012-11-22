CPP=g++
CFLAGS=-g -Wall -rdynamic
LEX=flex
YACC=bison
LLIB=-lreadline

yinsh: cmdparser.tab.o lex.yy.o 
	$(CPP) $(CFLAGS) cmdparser.tab.o lex.yy.o -o yinsh $(LLIB)
cmdparser.tab.o: cmdparser.tab.c classheader.h cmddef.h pipedef.h \
    listdef.h managerdef.h
	$(CPP) -c $(CFLAGS) cmdparser.tab.c
lex.yy.o: lex.yy.c classheader.h cmdparser.tab.h
	$(CPP) -c $(CFLAGS) lex.yy.c $(LLIB)
lex.yy.c: wordparser.l 
	$(LEX) wordparser.l 
cmdparser.tab.c: cmdparser.y
	$(YACC) -d --report=all cmdparser.y
clean:
	rm cmdparser.tab.o lex.yy.o cmdparser.tab.c
	rm cmdparser.tab.h lex.yy.c cmdparser.output
