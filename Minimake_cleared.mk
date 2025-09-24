minimake: minimake.o preprocessing.o level2.o
	gcc -Wall -g minimake.o preprocessing.o level2.o -o minimake
minimake.o: minimake.c preprocessing.h level2.h
	gcc -Wall -g -c minimake.c -o minimake.o
preprocessing.o: preprocessing.c preprocessing.h
	gcc -Wall -g -c preprocessing.c -o preprocessing.o
level2.o: level2.c level2.h
	gcc -Wall -g -c level2.c -o level2.o
clean:
	rm -f minimake.o preprocessing.o level2.o minimake
.PHONY: all clean
