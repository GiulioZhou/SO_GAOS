/* diskTest.c -- Test Disk Get and Disk Put */

#include "uconst.h"
#include "ulib.e"

#include "ulibuarm.e"

#define MILLION	1000000

int main() {
	
	int i;
	int dstatus;
	int *buffer;
	char buf[8];
	
	/* Use page 20 of kUseg2 as the buffer - ugly but we can't keep it locally,
	   since the u-proc has only one page for the stack! */
	buffer = (int *)(SEG2 + (20 * PAGE_SIZE));

	print_term("diskTest starts - use disk 1\n");
	
	*buffer = 42;  /* write 42 into the buffer */

	dstatus = write_disk(buffer, 1, 3); /* write in sector 3 */
	
	if (dstatus != DEV_S_READY) {	
		print_term("diskTest error: disk I/O result ");
	}	else {
		print_term("diskTest ok: disk I/O result\n");
	}

	*buffer = 100;  /* write 100 into the buffer */
	dstatus = write_disk(buffer, 1, 23); /* write 100 in sector 23 */
	dstatus = read_disk(buffer, 1, 3);  /* re-read the old block */
	
	if (*buffer != 42) {
		print_term("diskTest error: bad first disk sector readback\n");
	} else {
		print_term("diskTest ok: first disk sector readback\n");
	}

	dstatus = read_disk(buffer, 1, 23); /* re-read sector 23 */
	
	if (*buffer != 100) {
		print_term("diskTest error: bad second disk sector readback\n");
	} else {
		print_term("diskTest ok: second disk sector readback\n");
	}

  for (i = 0; i < (4096 / sizeof(int)); i++) {
    buffer[i] = i;
	}

  /* Write and re-read a whole sector */
	dstatus = write_disk(buffer, 1, 42); /* write in sector 42 */
	if (dstatus != DEV_S_READY) {
    print_term("diskTest error: bad write status\n");
	} else {
    dstatus = read_disk(buffer, 1, 42);

		for (i = 0; i < (4096 / sizeof(int)); i++) {
      if (buffer[i] != i) {
        print_term("diskTest error: bad data read from disk!\n");
			}
		}
		print_term("Write and re-read a whole sector finished\n");
	}

	/* should eventually exceed device capacity */
	i = 0;
	do {	  
 		print_term("diskTest: reading sector ");
		print_term(to_string(buf, 8, 10, -1, i));
    print_term("\n");
  	dstatus = read_disk(buffer, 1, i);
		i++;
	} while ((dstatus == DEV_S_READY) && (i < MILLION));
		
	if (i < MILLION) {
		print_term("diskTest ok: device capacity detection\n");
	}	else {
		print_term("diskTest ERROR: read one million sectors...\n");
	}
		
	print_term("diskTest: completed part 1 (should terminate now)\n");

	/* try to do a disk read into segment 1: should kill the proc */
	read_disk((void *)SEG1, 1, 3);

	print_term("diskTest error: just read into segment 1\n");

	/* generate a variety of program traps */
	i = i + *((int*)0);
	print_term("diskTest error: access invalid address succeded\n");

	SYSCALL(1, (int)buffer, 0, 0);
	print_term("diskTest error: sys1 did not terminate\n");
	
	return 0;
}

