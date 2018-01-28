	#these constants habe to be set to a kernel-specific values
	.equ	TERMINATESYS,	21
	.equ	SYSCAL,		8


	# remember that $sp has to be set by kernel too

 	# useful constants

	# This is the standard __start function for generic program
	# activation: it loads r15 with starting address and
	# at main() function return calls kernel TERMINATE service
	# (a SYSCALL with r0 == TERMINATE)

	.text
	.global	__start
	.extern main

__start:
	# $sp has to be set to correct value by kernel

	SUB sp, sp, #16
	STR fp, [sp], #12
	MOV fp, sp

	B	main

	MOV sp, fp
	LDR fp, [sp], #12
	ADD sp, sp, #16

	# calls kernel TERMINATE service

	MOV r0, #TERMINATESYS

	SWI #SYSCAL
