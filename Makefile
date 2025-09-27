# 链接生成可执行文件
minimake: minimake.o preprocessing.o level2.o level3.o level4.o
	gcc -Wall -g minimake.o preprocessing.o level2.o level3.o level4.o -o minimake

# 编译主文件
minimake.o: minimake.c preprocessing.h level2.h level3.h level4.h
	gcc -Wall -g -c minimake.c -o minimake.o

# 编译preprocessing模块（若依赖level3.h，可添加到依赖列表，否则保持原样）
preprocessing.o: preprocessing.c preprocessing.h
	gcc -Wall -g -c preprocessing.c -o preprocessing.o

# 编译level2模块（若level2依赖level3.h，可添加到依赖列表，否则保持原样）
level2.o: level2.c level2.h
	gcc -Wall -g -c level2.c -o level2.o

# 新增：编译level3模块（依赖自身头文件level3.h）
level3.o: level3.c level3.h level2.h level4.h
	gcc -Wall -g -c level3.c -o level3.o

level4.o: level4.c level4.h
	gcc -Wall -g -c level4.c -o level4.o
# 清理生成的文件（新增level3.o清理项）
clean:
	rm -f minimake.o preprocessing.o level2.o level3.o minimake

# 声明伪目标
.PHONY: all clean
