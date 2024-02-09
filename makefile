all: server cli

# make rule per il server
server: server.o libs/logger.o libs/device_utils.o libs/common_utils.o libs/database.o
	gcc -Wall -g server.o libs/logger.o libs/device_utils.o libs/common_utils.o libs/database.o -o server

# make rule per i client
cli: cli.o libs/logger.o libs/common_utils.o libs/database.o
	gcc -Wall -g cli.o libs/logger.o libs/common_utils.o libs/database.o -o cli

server.o: server.c
	gcc -Wall -g -c server.c -o server.o  

cli.o: cli.c
	gcc -Wall -g -c cli.c -o cli.o

libs/logger.o: libs/logger.c
	gcc -Wall -g -c libs/logger.c -o libs/logger.o

libs/device_utils.o: libs/device_utils.c
	gcc -Wall -g -c libs/device_utils.c -o libs/device_utils.o

libs/common_utils.o: libs/common_utils.c
	gcc -Wall -g -c libs/common_utils.c -o libs/common_utils.o

libs/database.o: libs/database.c
	gcc -Wall -g -c libs/database.c -o libs/database.o

clean:
	rm *o server cli libs/*.o
