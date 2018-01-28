/* tableTest.c -- prints pithagoric tables (?) */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

#define BUFSIZE 3   /* to account for the \n */

int main() {

	int status;
	int max_val, i, j;
	char buf[BUFSIZE];
	char b[4];
	
	do {
	  print_term("Enter a number (1...10): ");
	  status = read_term(buf);
	
	  if (status > BUFSIZE) status = BUFSIZE;
		buf[status - 1] = '\0'; /* remove trailing '\n' and replace with '\0' */
    max_val = to_num(buf);
	} while ((max_val < 1) || (max_val > 10));
	
	print_term("\n\n");

	for (i = 0; i < max_val; i++) {
    for (j = 0; j < max_val; j++) {
      print_term(to_string(b, 4, 10, ' ', (i + 1) * (j + 1)));
			print_term("|");
		}
		print_term("\n");
	}
	
	print_term("\n\nFinished printing the table\n");
		
	/* Terminate normally */	
	return 0;	
}

