#include <stdio.h>
#define CLASS_SIZE 5

struct student {
	char *last_name;
	int student_id;
	char grade;
};
typedef struct student student;
// PROTOTIPO
int fail(student cl[]);
