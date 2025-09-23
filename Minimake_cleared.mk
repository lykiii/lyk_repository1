hello: hello.o preprocessing.o
	gcc hello.o preprocessing.o -o hello
hello.o: hello.c preprocessing.h
	gcc -c hello.c -o hello.o
preprocessing.o: preprocessing.c preprocessing.h
	gcc -c preprocessing.c -o preprocessing.o
clean:
	rm -f hello hello.o preprocessing.o
