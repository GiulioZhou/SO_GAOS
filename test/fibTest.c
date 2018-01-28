/*	Test of a CPU intensive recusive job */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int fib (int i) {
	if ((i == 1) || (i == 2)) return (1);		
	return (fib(i - 1) + fib(i - 2));
}

unsigned int fact(int i) {
  return (i <= 1) ? 1 : i * fact(i - 1);
}

int main() {
	unsigned int i;
	
	print_term("Recursive Fibonacci test starts\n");
	
	i = fib(24);
	
	print_term("Fibonacci recursion concluded...");
	
	if (i == 46368) {
		print_term("successfully\n");
	}	else {
		print_term("with problems: error!\n");
	}

	
	print_term("Recursive factorial test starts\n");
	
	i = fact(12);
	
	print_term("Factorial recursion concluded...");
	
	if (i == 479001600) {
		print_term("successfully\n");
	}	else {
		print_term("with problems: error!\n");
	}
		
	/* Terminate normally */	
	return 0;
}

