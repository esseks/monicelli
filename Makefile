compile:
	bison --verbose -d Monicelli.y
	flex -P mc Monicelli.ll
	g++ -std=c++11 -DYYDEBUG=0 Monicelli.tab.c lex.mc.c Nodes.cpp main.cpp -o mcc
	rm Monicelli.tab.* lex.mc.c

graph:
	bison --graph Monicelli.y

clean:
	rm -f Monicelli.dot Monicelli.tab.* lex.mc.c Monicelli.output
