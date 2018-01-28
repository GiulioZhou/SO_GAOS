/* rogueTest.c -- tries to do various forbidden things: all of them should
   terminate the process. */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int main() {

	int choice, len;
	int *p;
	char buf[2];
	char first_array[] = "first";
	char second_array[] = "second";
/*
	print_term("Rogue test\n\n");
	print_term("0 - End test\n");
	print_term("1 - NULL pointer dereference\n");
	print_term("2 - Access memory in Kseg0\n");
	print_term("3 - Operate a semaphore not in Useg3\n");
	print_term("4 - Request to delay for < 0 seconds\n");
	print_term("5 - Write to .text section\n");
	print_term("6 - Buffer overflow!\n");
*/
	do {
	  print_term("\nEnter your choice: ");
    len = read_term(buf);
		if (len > 2) len = 2;
		buf[1] = '\0';
		choice = to_num(buf);
	} while((choice < 0) || (choice > 6));

	switch (choice) {
    case 0:
		  print_term("Terminating gracefully...\n");
		  return 0;
		  break;

		case 1:
		  p = NULL;
			*p = 10;  /* Should kill the process */
			print_term("Should not get here!\n");
		  break;

		case 2:
		  p = (int *)(SEG1 + 4);
			*p = 10;
			print_term("Should not get here!\n");
		  break;

		case 3:
		  p = (int *)(SEG2 + (20 * PAGE_SIZE));
			*p = 0;  /* Should succeed */
			print_term("Set the semaphore to 0...\n");
			V(p, 1);    /* Should fail */
			print_term("Should not get here!\n");
		  break;

		case 4:
		  delay(-6);
			print_term("Should not get here!\n");
      break;

    case 5:
		  p = (int *)(SEG2 + 10);  /* This is in the .text section */
			*p = 42;
			print_term("Should not get here!\n");
			break;

    case 6:
		  print_term("\nfirst_array[] is ");
			print_term(first_array);
			print_term("\nsecond_array[] is ");
			print_term(second_array);

			print_term("\nnow enter a string longer than 6 characters...");
			read_term(first_array);

			print_term("\nnow first_array[] is ");
			print_term(first_array);
			print_term("\nand second_array[] is ");
			print_term(second_array);

      return 0;

		default:
      print_term("Should not get here...\n");
		  break;
	}

	print_term("Houston, we have a problem...\n");
	return 0;
}
