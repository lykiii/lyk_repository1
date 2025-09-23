# 生成可执行文件hello，依赖于两个目标文件
hello: hello.o preprocessing.o
	# 链接目标文件生成可执行文件
	gcc hello.o preprocessing.o -o hello

# 编译hello.c生成目标文件，依赖于源文件和头文件
hello.o: hello.c preprocessing.h
	gcc -c hello.c -o hello.o

# 编译preprocessing.c生成目标文件，依赖于源文件和头文件
preprocessing.o: preprocessing.c preprocessing.h
	gcc -c preprocessing.c -o preprocessing.o

# 清理编译产生的文件
clean:
	rm -f hello hello.o preprocessing.o
