default: compile cleanautogen

compile:
	bison Monicelli.ypp
	flex Monicelli.lpp
	g++ \
    -Wall -Wno-deprecated-register -std=c++11 -DYYDEBUG=0 -O2 \
    Parser.cpp lex.yy.cc Nodes.cpp main.cpp -o mcc

graph:
	bison --graph Monicelli.y

cleanautogen:
	rm -f Parser.?pp lex.* location.hh position.hh stack.hh

clean: cleanautogen
	rm -f Monicelli.dot Monicelli.output
