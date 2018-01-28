/*	prodDiskTest -- Test Virtual P's and V's. pvTestB (producer) and pvTestA
 *  (consumer) exchange a message using disk 1 and two semaphores in the
 *  shared segment for synchronization
 */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"


int main() {
	/* these pointers are in the u-proc .data section, and point to some location
	in kUseg3. */

	int *hold = (int *)(SEG3 + 40);
	int *empty = (int *)(SEG3 + 44);
	int *full = (int *)(SEG3 + 48);

	char buf1[4096];
	char buf2[4096];  /* to have more than 1 .data page */
	char buf3[4096];

	int mysem, count = 0;
	char *msg, c, *p;

	print_term("prodDiskTest starts\n");

	/* give consDiskTest a chance to start up and initialize shared memory */

	/* Delay for 4 seconds, to be sure */
	delay(4);

	V(hold, 1);        /* this should have been set by prodDiskTest */

	print_term("prodDiskTest starts producing...\n");

	msg = "disk synch OK\n"; /* message to be sent to consDiskTest */

	/* send message to consDiskTest, one char at a time using disk 1 sect 0 */
	do {

		switch (count % 3) {
			case 0: p = buf1; break;
			case 1: p = buf2; break;
			case 2: p = buf3; break;
		}

		p[0] = c = *msg;
		write_disk(p, 1, 0);
		msg++;
		print_term("ProdDisk, produced a character\n");
		V(full, 1);          /* Wake up cons */
		P(empty, 1);         /* waits for cons to read the value */
		count++;
	} while (c != '\0');

	print_term("\nprodDiskTest, finished sending\n");

	print_term("prodDiskTest completed\n");

	/* try to block on a private semaphore */
	mysem = 0;
	P(&mysem, 1); /* this should kill the proc */

	/* should never reach here */
	print_term("pvTestB error: private sem block did not terminate\n");
	return 0;
}
