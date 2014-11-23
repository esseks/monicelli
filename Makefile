compile:
	bison --verbose -d Monicelli.y
	flex Monicelli.ll
	gcc -DYYDEBUG=0 Monicelli.tab.c lex.yy.c main.c -o mcc
	rm Monicelli.tab.* lex.yy.c

graph:
	bison --graph Monicelli.y

clean:
	rm -f Monicelli.dot Monicelli.tab.* lex.yy.c Monicelli.output
