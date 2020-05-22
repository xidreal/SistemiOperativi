#include <stdio.h>
#include <string.h> 


int main(int argc, char *argv[]){
	int askName=1;
	char name[100];	
	while(askName){
		printf("Please, insert my name\n\n");
		scanf("%s", name);
		if(strcmp(name,argv[0]) == 0)
			askName=0;
		else
			printf("No, it's not me!\n\n");
	}
	printf("Well done! This is my name!\n\n");
	return 0;
}

// make or gcc commandLineArgs1.c -o commandLineArgs1
// ./commandLineArgs1

