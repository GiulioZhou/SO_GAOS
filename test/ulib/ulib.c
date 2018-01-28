/*****************************************************************************
 * ulib.c: very very very minimal set of "library routines" for u-proc use.  *
 *****************************************************************************/

#include "uconst.h"
#include "ulib.e"
#include "ulibuarm.e"

/*****************************************************************************
 * print_term(): prints the given string on the u-proc's reserved terminal.  *
 *****************************************************************************/
int print_term(char *s) {
	int res;
	res = SYSCALL(WRITETERMINAL, (int)s, str_len(s), 0);
	return res;
}


/*****************************************************************************
 * print_prt(): prints the given string on the u-proc's reserved printer.    *
 *****************************************************************************/
int print_prt(char *s) {
	int res;
	res = SYSCALL(WRITEPRINTER, (int)s, str_len(s), 0);
	return res;
}

/*****************************************************************************
 * read_term(): reads a string from the terminal. NO BOUND CHECKING          *
 *****************************************************************************/
int read_term(char *s) {
	int len;
	len = SYSCALL(READTERMINAL, (int)s, 0, 0);
	/* NOTE: len includes the '\n' character */
	return len;
}

/*****************************************************************************
 * write_disk(): writes given buffer on the given disk at the given sector.  *
 *****************************************************************************/
int write_disk(void *buf, int disk_no, int sec_no) {
	int res;
	res = SYSCALL(DISK_PUT, (int)buf, disk_no, sec_no);
	return res;
}

/*****************************************************************************
 * read_disk(): reads the given disk at the given sector into given buffer.  *
 *****************************************************************************/
int read_disk(void *buf, int disk_no, int sec_no) {
	int res;
	res = SYSCALL(DISK_GET, (int)buf, disk_no, sec_no);
	return res;
}

/*****************************************************************************
 * V(): "V" the given semaphore.                                             *
 *****************************************************************************/
void V(int *sem, int weight) {
	SYSCALL(VSEMVIRT, (int)sem, weight, 0);
}

/*****************************************************************************
 * P(): "P" the given semaphore.                                             *
 *****************************************************************************/
void P(int *sem, int weight) {
	SYSCALL(PSEMVIRT, (int)sem, weight, 0);
}

/*****************************************************************************
 * delay(): blocks the proces for (at least) "nsec" seconds.                 *
 *****************************************************************************/
void delay(int nsec) {
	SYSCALL(DELAY, nsec, 0, 0);
}


/*****************************************************************************
 * get_TOD(): asks the OS for the Time Of Day and returns it.                *
 *****************************************************************************/
unsigned int get_TOD(void) {
	return SYSCALL(GETTOD, 0, 0, 0);
}


/*****************************************************************************
 * to_string(): fills the user-supplied string buffer "buf" with the         *
 * numerical representation of "n" in base "base". Always terminates the     *
 * string with \0. The parameter "fill" determines the filling symbol at the *
 * beginning of the string: valid values are '0', ' ' and -1). -1 is         *
 * a "no-fill" value, requesting that the function returns a pointer to the  *
 * first significative character of the string.                              *
 * No need to say that the caller should supply a reasonable value for "len".*
 * Consider that:                                                            *
 * - 1 character goes for the '\0' terminator;                               *
 * - 1 character may be used for the sign, if the number is negative.        *
 *   Negative numbers are accepted only if "base" is 10.                     *
 * If the value of "len" is too small, the string representation will be     *
 * missing its most significative digits.                                    *
 * Examples:                                                                 *
 *                                                                           *
 * |len|fill |  n  | actual string returned               |                  *
 * +---+-----+-----+--------------------------------------+                  *
 * | 3 | -1  | 12  | '1','2','\0'             --> "12"    | (OK, no fill)    *
 * | 3 | -1  | 123 | '2','3','\0'             --> "23"    | (truncated)      *
 * | 3 | -1  | -12 | '-','2','\0'             --> "-2"    | (truncated)      *
 * | 6 | ' ' | 15  | ' ',' ',' ','1','5','\0' --> "   15" | (OK)             *
 * | 6 | '0' | 15  | '0','0','0','1','5','\0' --> "00015" | (OK)             *
 * | 6 | ' ' | -15 | ' ',' ','-','1','5','\0' --> "  -15" | (OK)             *
 * | 6 | '0' | -15 | '-','0','0','1','5','\0' --> "-0015" | (OK)             *
 * | 6 | -1  | -15 | ...,'-','1','5','\0'     --> "-15"   | (OK, no fill)    *
 * | 6 | -1  | 15  | ...,'1','5','\0'         --> "15"    | (OK, no fill)    *
 *****************************************************************************/
char *to_string(char *buf, int len, int base, signed char fill, int n) {

	int output_length = 8;
	int i;
	buf[0] = '0';
	buf[1] = 'x';
	buf[2 + output_length] = 0;
	for (i = 2; i < 10; i++)
	buf[i] = '0';
	i = 1 + output_length;
	while (n > 0 && i > 1) {
		buf[i] = 48 + n % 16;
		buf[i] += buf[i] < 58 ? 0 : 7;
		n /= 16;
		i--;
	}

	return buf;

}

/*****************************************************************************
 * to_num(): return the numeric value of the string contained in the buffer  *
 * "buf". "buf" MUST be terminated by '\0'.                                  *
 *****************************************************************************/
int to_num(char *buf) {

	/* consider possible spaces/zeros at the beginning, or a '-'/'+'.
	   To keep things simple, aborts as soon as a malformed string is recognized.
	   The returned value is the number computed so far. */

	signed char negative = 1;
	int res = 0;

	/* Skip leading zeros and spaces */
	while ((*buf == ' ') || (*buf == '0')) buf++;

	/* Check if the first useful character is a '-' or a '+' */
	if (*buf == '-') {
		negative = -1; buf++;
	} else if (*buf == '+') {
		buf++;   /* skip the '+' also */
	}

	while ((*buf != '\0') && (*buf >= '0') && (*buf <= '9')) {
		res *= 10;
		res += (*buf - '0');
		buf++;
	}

	return (res * negative);
}

/*****************************************************************************
 * pow(): returns the value of "base" to the "exp"th power.                  *
 *****************************************************************************/
int pow(int base, int exp) {

	int ret = 1;

	while (exp-- > 0) {
		ret *= base;
	}
	return ret;
}

/*****************************************************************************
 * str_len(): returns the length of the given string, not including the \0.  *
 *****************************************************************************/
int str_len(char *s) {
	int len = 0;

	while (*s++ != '\0') len++;
	return len;
}

/*****************************************************************************
 * uexit(): terminates the u-proc cleanly.                                   *
 *****************************************************************************/
int uexit(void) {
	return SYSCALL(TERMINATE, 0, 0, 0);
}
