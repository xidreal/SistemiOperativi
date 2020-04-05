#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
	int a,b;
	if(argc!=4){
		printf("Wrong number of arguments.\n");
		return 1;
	}
	sscanf (argv[2],"%d",&a);
	sscanf (argv[3],"%d",&b);
	if(strcmp(argv[1], "+") == 0)
		printf("The sum is %d\n",a+b);
	if(strcmp(argv[1], "-") == 0)
		printf("The difference is %d\n",a-b);
	if(strcmp(argv[1], "*") == 0) // \* expected
		printf("The product is %d\n",a*b);
	if(strcmp(argv[1], "/") == 0)
		printf("The ratio is %d\n",a/b);
	return 0;
}

// gcc commandLineArgs2.c -o commandLineArgs2
// ./commandLineArgs2 + 3 5
// NB: usare escape \* per prodotto e div \/

