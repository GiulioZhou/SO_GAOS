/* swapTest -- Tests the memory management. SHOULD NOT BE RUN TOGETHER WITH 
   OTHER PROCESSES THAT USE kUseg3 - you have been warned */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int main () {

  char i;
	int corrupt;
	int *location;
	char buf[6];  /* to hold number to string */

	print_term("swapTest starts\n");

	/* write into the first word of pages 20-29 of seg2:should be safe, since
	   kaya does not prevent writing beyond the end of .data */
	for (i = 20; i < 30; i++) {
	  location = (int *)(SEG2 + (i * PAGE_SIZE));
		*location = i;
		print_term("swapTest ok: wrote to page ");
		print_term(to_string(buf, 6, 10, -1, i));
		print_term(" of kUseg2\n");
	}

	print_term("swapTest ok: wrote to pages of seg kUseg2\n");
	print_term("Now check if data is still there...\n");

	/* check if first word of pages still contain what we wrote */
	corrupt = FALSE;
	for (i = 20; i < 30; i++) {
	  location = (int *)(SEG2 + (i * PAGE_SIZE));
		if (*location != i) {
			print_term("swapTest error: swapper corrupted data\n");
			corrupt = TRUE;
			break;
		}
	}

	if (corrupt == FALSE) print_term("swapTest ok: data survived swapper\n");

  print_term("\n");

  /* do the same things in kUseg3 */	
	/* write into the first word of pages 20-29 of seg3 */
	for (i = 20; i < 30; i++) {
	  location = (int *)(SEG3 + (i * PAGE_SIZE));
		*location = i;
		print_term("swapTest ok: wrote to page ");
		print_term(to_string(buf, 6, 10, -1, i));
		print_term(" of kUseg3\n");
	}

	print_term("swapTest ok: wrote to pages of seg kUseg3\n");
	print_term("Now check if data is still there...\n");

	/* check if first word of pages still contain what we wrote */
	corrupt = FALSE;
	for (i = 20; i < 30; i++) {
	  location = (int *)(SEG3 + (i * PAGE_SIZE));
		if (*location != i) {
			print_term("swapTest error: swapper corrupted data\n");
			corrupt = TRUE;
			break;
		}
	}
	
	if (corrupt == FALSE) print_term("swapTest ok: data survived swapper\n");

	/* try to access segment ksegOS Should cause termination */
	i = *((char *)(0x20000000));
	print_term("swapTest error: could access segment ksegOS\n");

	return 0;
}

