# 链接生成可执行文件
minimake: minimake.o preprocessing.o level2.o
	gcc -Wall -g minimake.o preprocessing.o level2.o -o minimake

# 编译主文件
minimake.o: minimake.c preprocessing.h level2.h
	gcc -Wall -g -c minimake.c -o minimake.o

# 编译preprocessing模块
preprocessing.o: preprocessing.c preprocessing.h
	gcc -Wall -g -c preprocessing.c -o preprocessing.o

# 编译level2模块
level2.o: level2.c level2.h
	gcc -Wall -g -c level2.c -o level2.o

# 清理生成的文件
clean:
	rm -f minimake.o preprocessing.o level2.o minimake

# 声明伪目标
.PHONY: all clean
