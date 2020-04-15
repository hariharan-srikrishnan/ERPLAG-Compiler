section .data
	integerRead: db "%d", 0
	integerWrite: db "%d", 10, 0
	realRead: db "%lf", 0
	realWrite: db "%lf", 10, 0
	intMessage: db "Enter an integer", 10, 0
	reaMessage: db "Enter an real:", 10, 0
	boolMessage: db "Enter a boolean (1 or 0 only):", 10, 0
	intArrMessage: db "Enter integer array for range %d to %d:", 10, 0
	realArrMessage: db "Enter real array for range %d to %d:", 10, 0
	boolArrMessage: db "Enter boolean array for range %d to %d (1 or 0 only):", 10, 0
	runTimeError: db "Run-time error: Bound values error", 10, 0


section .text
extern printf
extern scanf
global main

_error: 
	MOV RDI, runTimeError
	XOR RAX, RAX
	CALL printf
	MOV RAX, 60
	SYSCALL


main: 
	PUSH RBP
	MOV RBP, RSP
	SUB RSP, 40
	MOV RAX, 5
	MOV QWORD [RBP - 16 - 0], RAX
	MOV RAX, QWORD [RBP - 16 - 0]
	MOV QWORD [RBP - 16 - 24], RAX
	MOV RAX, 9
	MOV QWORD [RBP - 16 - 0], RAX
	MOV RAX, QWORD [RBP - 16 - 0]
	MOV QWORD [RBP - 16 - 32], RAX
	LEA RDI, [intMessage]
	XOR RAX, RAX
	CALL printf
	LEA RDI, [integerRead]
	LEA RSI, [RBP - 16 - 0]
	XOR RAX, RAX
	CALL scanf
	LEA RDI, [intMessage]
	XOR RAX, RAX
	CALL printf
	LEA RDI, [integerRead]
	LEA RSI, [RBP - 16 - 8]
	XOR RAX, RAX
	CALL scanf
	MOV   RAX, QWORD [RBP - 16 - 8]
	IMUL QWORD [RBP - 16 - 32]
	MOV   QWORD [RBP - 16 - 40], RAX
	MOV  RAX, QWORD [RBP - 16 - 24]
	SUB  RAX, QWORD [RBP - 16 - 32]
	MOV  QWORD [RBP - 16 - 40], RAX
	MOV   RAX, QWORD [RBP - 16 - 40]
	IMUL QWORD [RBP - 16 - 8]
	MOV   QWORD [RBP - 16 - 40], RAX
	MOV RAX, 2
	MOV QWORD [RBP - 16 - 0], RAX
	MOV   RAX, QWORD [RBP - 16 - 24]
	IMUL QWORD [RBP - 16 - 0]
	MOV   QWORD [RBP - 16 - 40], RAX
	MOV   RAX, QWORD [RBP - 16 - 32]
	IMUL QWORD [RBP - 16 - 0]
	MOV   QWORD [RBP - 16 - 40], RAX
	MOV  RAX, QWORD [RBP - 16 - 40]
	SUB  RAX, QWORD [RBP - 16 - 0]
	MOV  QWORD [RBP - 16 - 40], RAX
	MOV  RAX, QWORD [RBP - 16 - 40]
	ADD  RAX, QWORD [RBP - 16 - 0]
	MOV  QWORD [RBP - 16 - 40], RAX
	MOV  RAX, QWORD [RBP - 16 - 40]
	ADD  RAX, QWORD [RBP - 16 - 0]
	MOV  QWORD [RBP - 16 - 40], RAX
	MOV  RAX, QWORD [RBP - 16 - 0]
	ADD  RAX, QWORD [RBP - 16 - 0]
	MOV  QWORD [RBP - 16 - 40], RAX
	MOV RAX, QWORD [RBP - 16 - 40]
	MOV QWORD [RBP - 16 - 16], RAX
	LEA RSI, [RBP - 16 - 16]
	LEA RDI, [integerWrite]
	XOR RAX, RAX
	CALL printf
	MOV RSP, RBP
	POP RBP
	RET
