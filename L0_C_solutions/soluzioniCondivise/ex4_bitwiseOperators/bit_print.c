#include "bit_print.h"

/* Stampa i bit di una espressione int*/
int main(void){
	// Stampa numeri da 0 a 9 in rappresentazione binaria
	int i;
	for(i=0; i<10; ++i){	
		printf("%d: ", i);
		bit_print(i);
		putchar ('\n');
	}
	printf("\n\n");

	// Operatori binari logici bit a bit
	int a=33333;
	int b= -77777;
	
	printf("a: %d ,", a);
	bit_print(a);
	putchar ('\n');

	printf("b: %d, ", b);
	bit_print(b);
	putchar ('\n');

	printf("a&b: %d, ", a&b);
	bit_print(a&b);
	putchar ('\n');

	printf("a^b: %d, ", a^b);
	bit_print(a^b);
	putchar ('\n');

	printf("a | b: %d, ", a|b);
	bit_print(a|b);
	putchar ('\n');

	printf("~(a | b): %d, ", ~(a | b));
	bit_print(~(a | b));
	putchar ('\n');

	printf("(~a & ~b): %d, ", (~a & ~b));
	bit_print((~a & ~b));
	putchar ('\n');putchar ('\n');

	// Operatori di scorrimento	
	printf("a: %d, ", a);
	bit_print(a);
	putchar ('\n');

	printf("a<<1: %d, ", a<<1);
	bit_print(a<<1);
	putchar ('\n');

	printf("a<<4: %d, ", a<<4);
	bit_print(a<<4);
	putchar ('\n');

	printf("a<<31: %d, ", a<<31);
	bit_print(a<<31);
	putchar ('\n');

	//int d=(1<<31);
	//unsigned e=(1<<31);

	//printf("d: %d, ", d);
	//bit_print(d);
	//putchar ('\n');

	printf("a>>4: %d, ", a>>4);
	bit_print(a>>4);
	putchar ('\n');

	return 0;
}

void bit_print(int a){
	int i; 
	int n=sizeof(int) * CHAR_BIT; // In limits.h (number of bits in a int)
	int mask = (1 << (n-1)); // mask = 100...0
	for(i=1; i<=n; i++){
		putchar(((a & mask)==0) ? '0' : '1'); // Print the most significative bit (on the left) of a
		a  <<= 1;	// Shift a to the left
		if(i % CHAR_BIT == 0 && i<n)
			putchar (' ');
	}
}

// gcc bit_print.c -o bit_print
// ./bit_print
