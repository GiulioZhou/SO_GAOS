/* todTest -- Test of Delay and Get Time of Day */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

int main() {
	unsigned int now1 ,now2;

	now1 = get_TOD();
	print_term("todTest starts\n");
	now1 = get_TOD();

	now2 = get_TOD();

	if (now2 < now1) {
		print_term("todTest error: time decreasing\n");
	} else {
		print_term("todTest ok: time increasing\n");
	}

	delay(1);	    /* Delay 1 second */
	now1 = get_TOD();

	if ((now1 - now2) < SECOND)
		print_term("todTest error: did not delay one second\n");
	else
		print_term("todTest ok: one second delay\n");
		
	print_term("todTest completed\n");
		
	/* Try to execute nucleys system call. Should cause termination */
	now1 = SYSCALL(6, 0, 0, 0);
	
	print_term("todTest error: SYS6 did not terminate\n");
	
	return 0;
}

