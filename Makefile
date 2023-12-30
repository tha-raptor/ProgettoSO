master.c: libscissione.h libscissione.c
	gcc libscissione.c -o libscissione

compile:
	gcc -c atomo.c libscissione.c 
	gcc -c attivatore.c libscissione.c
	gcc -c alimentazione.c libscissione.c
	gcc -c inibitore.c libscissione.c
	gcc -c master.c libscissione.c
	gcc -Wvla -Wextra -Werror atomo.o libscissione.o -o atomo
	gcc -Wvla -Wextra -Werror attivatore.o libscissione.o -o attivatore
	gcc -Wvla -Wextra -Werror alimentazione.o libscissione.o -o alimentazione
	gcc -Wvla -Wextra -Werror inibitore.o libscissione.o -o inibitore
	gcc -Wvla -Wextra -Werror master.o libscissione.o -o master
	
start:
	./master 
	
removeipc:
	./removeipc.sh

clean:
	rm -f *.o
	ls | grep -v "\." | grep -v Makefile | xargs rm