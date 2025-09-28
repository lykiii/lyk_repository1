# 链接生成可执行文件
minimake: minimake.o preprocessing.o level2.o level3.o level4.o level5.o
	gcc -Wall -g minimake.o preprocessing.o level2.o level3.o level4.o level5.o -o minimake

# 编译主文件（保持不变，依赖正确）
minimake.o: minimake.c preprocessing.h level2.h level3.h level4.h level5.h
	gcc -Wall -g -c minimake.c -o minimake.o

# 编译preprocessing模块（保持不变，若后续依赖其他头文件可补充）
preprocessing.o: preprocessing.c preprocessing.h
	gcc -Wall -g -c preprocessing.c -o preprocessing.o

# 编译level2模块（保持不变，依赖正确）
level2.o: level2.c level2.h level5.h
	gcc -Wall -g -c level2.c -o level2.o

# 编译level3模块（保持不变，依赖正确）
level3.o: level3.c level3.h level2.h level4.h
	gcc -Wall -g -c level3.c -o level3.o

# 编译level4模块（保持不变，依赖正确）
level4.o: level4.c level4.h
	gcc -Wall -g -c level4.c -o level4.o

# 编译level5模块（修改2：修正源文件和目标文件匹配，原错误为用level4.c生成level4.o）
level5.o: level5.c level5.h
	gcc -Wall -g -c level5.c -o level5.o

# 清理生成的文件（修改3：补充level4.o、level5.o，确保所有目标文件都被清理）
clean:
	rm -f minimake.o preprocessing.o level2.o level3.o level4.o level5.o minimake

# 声明伪目标（保持不变）
.PHONY: all clean
