#include "ascii.h"
int main(void){
	// Stampa numeri da 0 a 9 in rappresentazione binaria
	int i;
	printf("Please, insert a positive integer\n\n");
	scanf("%d", &i);
	intToString(i);
	return 0;
}

void intToString(int i){
	printf("Conversion to inverse string:\n\n");
	int r;	 
	while(i){
		r = i % 10;
		putchar(r+48);
		i=i/10;
	}
	putchar('\n');
}

// make or gcc ascii.c -o ascii
// ./ascii
