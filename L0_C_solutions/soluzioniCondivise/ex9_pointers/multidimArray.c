#include "multidimArray.h"

int main(void){
	char helloSentence[100] = "Corso di sistemi operativi";
	stringToMultiDimArray(helloSentence);
	return 0;
}

void stringToMultiDimArray(char *s){
	char *helloSentenceMat[10]={0};
	char *token;
	token = strtok(s, " ");
   	
	int i=0;
   	while(token!=NULL) {
      		//printf(" %s\n", token );
		helloSentenceMat[i]=token;
		i++;
      		token = strtok(NULL, " ");
   	}
	char *tmp=helloSentenceMat[0];
	i=1;
	while(tmp!=NULL){
		printf("%s\n", tmp );
		tmp=helloSentenceMat[i];
		i++;
	}
}



// make or gcc multidimArray.c -o multidimArray
// ./multidimArray
