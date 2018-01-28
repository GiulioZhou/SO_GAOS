/* consDiskTest -- Test Virtual P's and V's. consdiskTest (consumer) and
 * prodDiskTest (producer) exchange a message using disk1, and the shared
 * segment for synchronization.
 */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int main() {

	int *hold = (int *)(SEG3 + 40);
	int *empty = (int *)(SEG3 + 44);
	int *full = (int *)(SEG3 + 48);

	#define		MES_SIZE 40

	char buf1[4096];
	char buf2[4096];
	char buf3[4096];

	char msg[MES_SIZE], *p, c, *tmp;
	int count = 0;

	print_term("consDiskTest starts\n");

	/* initialize shared memory */
	*hold = 0;
	*full = 0;
	*empty = 0;

	/* block until prodDiskTest has started up */
	P(hold, 1);

	print_term("consDiskTest starts consuming...\n");

	/* receive characters from prodDiskTest */

	p = msg;  /* p points at the beginning of msg, so writes into msg */

	do {
		P(full, 1);               /* Blocks on full, waits for produces */

		switch (count % 3) {
			case 0: tmp = buf1; break;
			case 1: tmp = buf2; break;
			case 2: tmp = buf3; break;
		}

		read_disk(tmp, 1, 0);

		*p = c = *tmp;
		p++;
		print_term("Consumer, consumed a char\n");
		V(empty, 1);             /* Signal on empty, unblock producer */
	} while (c != '\0');

	/* print message received from producer */
	print_term("\nI received the following message from prodDiskTest:\n");
	print_term(msg);

	print_term("\nconsDiskTest completed\n");

	/* terminate normally */
	return 0;

	print_term("pvTestA error: did not terminate\n");
	uexit();
}
