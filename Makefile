compile:
	bison --verbose -d Monicelli.ypp
	flex -P mc Monicelli.lpp
	g++ -Wall -Wno-deprecated-register -std=c++11 -DYYDEBUG=0 Monicelli.tab.cpp lex.mc.c Nodes.cpp main.cpp -o mcc
	rm Monicelli.tab.* lex.*

graph:
	bison --graph Monicelli.y

clean:
	rm -f Monicelli.dot Monicelli.tab.* lex.* Monicelli.output location.hh position.hh stack.hh
