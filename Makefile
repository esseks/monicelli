compile:
	bison --verbose -d Monicelli.y
	flex -P monicelli_ Monicelli.ll
	g++ -std=c++11 -DYYDEBUG=0 Monicelli.tab.c lex.monicelli_.c Nodes.cpp main.cpp -o mcc
	rm Monicelli.tab.* lex.monicelli_.c

graph:
	bison --graph Monicelli.y

clean:
	rm -f Monicelli.dot Monicelli.tab.* lex.monicelli_.c Monicelli.output
