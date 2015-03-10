compile:
	# Make sure to have mcc somewhere in path
	mcc turtle.mm turtle.mc
	llc turtle.bc
	c++ turtle.s turtle.cpp -I../.. -lcairo -o tartaruga
	rm -f turtle.s
