GCC=gcc

practica2: suscriptor.c publicador.c broker.c
	$(GCC) -o suscriptor suscriptor.c 
	$(GCC) -o publicador publicador.c
	$(GCC) -o broker broker.c -lpthread


.PHONY: clean
clean:
	rm suscriptor publicador broker *.log
