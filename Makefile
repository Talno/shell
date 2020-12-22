all: shell 

shell: shell.o logging.o
	gcc -D_POSIX_C_SOURCE -Wall -std=c99 -o shell shell.o logging.o

shell.o: shell.c shell.h
	gcc -D_POSIX_C_SOURCE -Wall -g -std=c99 -c shell.c   

logging.o:logging.c logging.h
	gcc -Wall -g -std=c99 -c logging.c     

clean:
	rm -rf shell.o logging.o shell 




