/* helloTest.c -- The classic "Hello, world", revised */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int main() {

  char buf[5];
  unsigned int i;

  print_term("Size of char: ");
	print_term(to_string(buf, 5, 10, -1, sizeof(char)));
  print_term("\nSize of int: ");
	print_term(to_string(buf, 5, 10, -1, sizeof(int)));
  print_term("\nSize of long int: ");
	print_term(to_string(buf, 5, 10, -1, sizeof(long int)));
	print_term("\n");

	for (i = 0; i < 100; i++) {
	  print_term(to_string(buf, 5, 10, -1, i));
    print_term(" ->");
		print_term("Hello, ");
		print_term("world (1)!\n");
	}

  return 0;
}

