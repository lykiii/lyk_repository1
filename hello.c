#include <stdio.h>
#include <string.h>
int main(int argc,char* argv[]) {
        if(argc<3){
        printf("The parameters are less than 2\n");
        for(int i=0;i<argc;i++){
        	printf("<parameter%d>:%s\n",i,argv[i]);
        }
        return 1;
        }
        if(strcmp(argv[1],"--help")==0){
	printf("help information:\n");
	}
    
    return 0;
}
