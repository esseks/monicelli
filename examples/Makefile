MCC=mcc
EXAMPLES=factorial hello-world primes return fibonacci mandelbrot float ciao-mondo

all: $(EXAMPLES)

clean:
	$(RM) $(EXAMPLES)

%: %.mc
	$(MCC) $< -o $@
