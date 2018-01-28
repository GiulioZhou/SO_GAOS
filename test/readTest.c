/* readTest.c -- test of terminal read */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

#define BUFSIZE 21
#define NUMSIZE 7

int main() {

	int status, val;
	char buf[BUFSIZE];
	char numbuf[NUMSIZE];
	char b[4];
	
	print_term("Terminal Read Test starts\n");
	print_term("Enter a string (max ");
	print_term(to_string(b, 4, 10, -1, (BUFSIZE - 1)));
	print_term(" chars): ");
		
	status = read_term(buf);
	if (status > BUFSIZE) status = BUFSIZE;
	buf[status - 1] = '\0';
	
	print_term("\nYou entered: ");
	print_term(buf);

	print_term("\nEnter a number (max ");
	print_term(to_string(b, 4, 10, -1, NUMSIZE - 1));
	print_term(" chars): ");
		
	status = read_term(numbuf);
	if (status > NUMSIZE) status = NUMSIZE;
	numbuf[status - 1] = '\0'; /* remove trailing '\n' and replace with '\0' */
	
	/* Convert to number */
  val = to_num(numbuf);
	
	print_term("\nYou entered: ");
	print_term(to_string(numbuf, NUMSIZE, 10, -1, val));

	print_term("\n\nTerminal Read concluded\n");
		
	/* Terminate normally */	
	return 0;
}

