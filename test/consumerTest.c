/* consumerTest -- Test Virtual P's and V's. consumerTest (consumer) and
 * producerTest (producer) exchange a message using the shared segment for
 * synchronization and data transmission.
 */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

#define		MES_SIZE 40


int main() {
	int *hold = (int *)(SEG3);
	int *empty = (int *)(SEG3 + 4);
	int *full = (int *)(SEG3 + 8);
	char *strbuff[STRMAX];

	char msg[MES_SIZE], *p, c = '\0' + 1;
	unsigned int i;

	for(i = 0; i < STRMAX; i++){
		strbuff[i] = (char *) (SEG3+12+(i*4));
	}

	print_term("consumerTest starts\n");

	/* initialize shared memory */
	*hold = 0;
	*full = 0;
	*empty = 0;

	/* block until producerTest has started up */
	P(hold, 1);

	print_term("consumerTest starts consuming...\n");

	/* receive characters from producer */
	p = msg;  /* p points at the beginning of msg, so writes into msg */
	do {
		V(empty, 1);             /* Signal ready, unblock producer */
		P(full, STRMAX);               /* Blocks on full, waits for producer */
		i = 0;
		do {
			*p = c = *strbuff[i];
		 	i++;
			p++;
		} while(i < STRMAX && c != '\0');
		print_term("Cons. ");
	} while (c != '\0');

	/* print message received from producerTest */
	print_term("\nI received the following message from producerTest:\n");
	print_term(msg);

	print_term("\nconsumerTest completed\n");

	/* terminate normally */
	return 0;

	print_term("consumerTest error: did not terminate\n");
	return 0; /* to shut down gcc warning */
}
