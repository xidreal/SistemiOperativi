#include <stdio.h>
#define  SQ(x)  x*x // ((x)*(x))

int main(void){
	
	printf("SQ(7): %d \n\n",SQ(7));
	printf("SQ(7+5): %d \n\n",SQ(7+5));
	return 0;
}

// make or gcc macroSquare.c -o macroSquare
// ./macroSquare
