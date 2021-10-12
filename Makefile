ssu_mntr : main.o ssu_daemon.o
	gcc main.o -o ssu_mntr
	gcc ssu_daemon.o -o ssu_daemon

main.o : main.c
	gcc -c main.c

ssu_daemon.o : ssu_daemon.c
	gcc -c ssu_daemon.c

clean :
	rm *.o
	rm ssu_mntr
