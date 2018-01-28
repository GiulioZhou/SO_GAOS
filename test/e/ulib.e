int print_term(char *s);
int print_prt(char *s);
int read_term(char *s);
int write_disk(void *buf, int disk_no, int sec_no);
int read_disk(void *buf, int disk_no, int sec_no);
void V(int *sem, int weight);
void P(int *sem, int weight);
void delay(int nsec);
unsigned int get_TOD(void);
char *to_string(char *buf, int len, int base, signed char fill, int n);
int to_num(char *buf);
int pow(int base, int exp);
int str_len(char *s);
int uexit(void);

