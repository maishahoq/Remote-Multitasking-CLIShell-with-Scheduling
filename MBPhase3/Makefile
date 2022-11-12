all: client server

client: MaishaBhavickaClientPhase3.o
	gcc MaishaBhavickaClientPhase3.o -o client

MaishaBhavickaClientPhase3.o: MaishaBhavickaClientPhase3.c
	gcc -c MaishaBhavickaClientPhase3.c

server: MaishaBhavickaServerPhase3.o
	gcc MaishaBhavickaServerPhase3.o -o server

MaishaBhavickaServerPhase3.o: MaishaBhavickaServerPhase3.c
	gcc -c MaishaBhavickaServerPhase3.c

clean:
	rm *.o client server