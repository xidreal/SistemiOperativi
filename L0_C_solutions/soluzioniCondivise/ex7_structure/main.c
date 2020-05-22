#include "class_info.h"

int main(void){
	student cl[CLASS_SIZE]; // Array di struct student
	// Inizializzazone 
	cl[0].grade='A';
	cl[0].last_name="Casanova";
	cl[0].student_id=910017;
	cl[1].grade='F';
	cl[1].last_name="Rossi";
	cl[1].student_id=910018;
	cl[2].grade='F';
	cl[2].last_name="Verdi";
	cl[2].student_id=910019;
	cl[3].grade='F';
	cl[3].last_name="Neri";
	cl[3].student_id=910020;
	cl[4].grade='C';
	cl[4].last_name="Bianchi";
	cl[4].student_id=910021;
	int nf=fail(cl);
	printf("Numero voti negativi: %d\n\n",nf);
	
	// Test accesso a membri di struct
	printf("Cognome primo studente: %s\n\n",cl[0].last_name); // Accesso con operatore .
	student *p;
	p=&cl[0];
	printf("Voto primo studente: %c\n\n",p->grade); // Accesso con operatore ->

}

/* Conta voti scarsi */ 
int fail(student cl[]){
	int i,cnt=0;
	for(i=0; i<CLASS_SIZE; i++)
		cnt += (cl[i].grade=='F'); // Somma 1 se il grade è ‘F’
	return cnt;
}

// gcc main.c -o cl_info
// ./cl_info
