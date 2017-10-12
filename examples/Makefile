MCC=mcc
EXAMPLES=factorial hello-world primes return fibonacci mandelbrot float

all: $(EXAMPLES)

clean:
	$(RM) $(EXAMPLES)

%: %.mc
	$(MCC) $< -o $@
