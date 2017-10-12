MCC=mcc

all: turtle

clean:
	$(RM) *.o turtle

turtle.mc:;

%.mc.o: %.mc
	$(MCC) -c $< -o $@

turtle: turtle.mc.o turtle.cpp
	$(CXX) -std=c++11 $^ -lcairo -o $@
