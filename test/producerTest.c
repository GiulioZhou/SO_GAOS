/* producerTest -- Test Virtual P's and V's. producerTest (producer) and
 * consumerTest (consumer) exchange a message using the shared segment for
 * synchronization and data transmission.
 */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int main() {
  
  /* these pointers are in the u-proc .data section, and point to some location
  in kUseg3. */

  int *hold = (int *)(SEG3);
  int *empty = (int *)(SEG3 + 4);
  int *full = (int *)(SEG3 + 8);
  char *strbuff[STRMAX];

	int mysem;
	unsigned int i;

	char *msg, c = '\0' + 1;
	for(i = 0; i < STRMAX; i++){
		strbuff[i] = (char *) (SEG3+12+(i*4));
	}

	print_term("producerTest starts\n");

	/* give consumerTest a chance to start up and initialize shared memory */

	/* Delay for 4 seconds to be sure */
	delay(4);

	V(hold, 1);        /* this should have been set by consumerTest */

	print_term("producerTest can start the producing...\n");

	msg = "virtual synch OK\n\0";	/* message to be sent to consumerTest */

	/* send message to consumerTest, one char at a time */
	do {
		P(empty, 1);         /* waits for consumer to be ready */
		/* write some characters in buffer */
		i = 0;
		do {
			*strbuff[i] = c = *msg;
			i++;
			msg++;
		} while(i < STRMAX && c!= '\0');
		print_term("Prod. ");
		V(full, i);          /* Wake up consumer */
	} while (c != '\0');

	print_term("\nproducerTest finished sending\n");

	/* Make sure consumer is unlocked for last cycle */
	V(full, STRMAX);

	print_term("producerTest completed\n");

	/* try to block on a private semaphore */
	mysem = 0;
	P(&mysem, 1); /* this should kill the proc */

	/* should never reach here */
	print_term("producerTest error: private sem block did not terminate\n");
	return 0;
}
