section .data

integerRead db "%d", 0
integerWrite db "%d", 10, 0
realRead db "%lf", 0
realWrite db "%lf", 10, 0
intMessage db "Enter an integer", 10, 0
realMessage db "Enter an real:", 10, 0
boolMessage db "Enter a boolean (1 or 0 only):", 10, 0
intArrMessage db "Enter integer array for range %d to %d:", 10, 0
realArrMessage db "Enter real array for range %d to %d:", 10, 0
boolArrMessage db "Enter boolean array for range %d to %d (1 or 0 only):", 10, 0
trueMessage db "True", 10, 0
falseMessage db "False", 10,  0
runTimeError db "Run-time error: Bound values error", 10, 0


section .text
extern printf
extern scanf
global main

_error: 
	MOV RDI, runTimeError
	MOV AL, 0
	CALL printf
	MOV RAX, 60
	MOV RDI, 0
	SYSCALL


main: 
	PUSH RBP
	MOV RBP, RSP
	SUB RSP, 48
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 48], 5
	MOV RAX, QWORD [RBP - 16 - 48]
	MOV QWORD [RBP - 16 - 24], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 56], 9
	MOV RAX, QWORD [RBP - 16 - 56]
	MOV QWORD [RBP - 16 - 32], RAX
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label0
	SUB RSP, 8
	MOV R8, 1
label0: 
	LEA RDI, [intMessage]
	MOV AL, 0
	CALL printf
	LEA RDI, [integerRead]
	LEA RSI, [RBP - 16 - 0]
	MOV AL, 0
	CALL scanf
	CMP R8D, 1
	JNE label1
	ADD RSP, 8
label1: 
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label2
	SUB RSP, 8
	MOV R8, 1
label2: 
	LEA RDI, [intMessage]
	MOV AL, 0
	CALL printf
	LEA RDI, [integerRead]
	LEA RSI, [RBP - 16 - 8]
	MOV AL, 0
	CALL scanf
	CMP R8D, 1
	JNE label3
	ADD RSP, 8
label3: 
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 8]
	MOV R12, QWORD [RBP - 16 - 32]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 64], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 24]
	MOV R12, QWORD [RBP - 16 - 32]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 72], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 72]
	MOV R12, QWORD [RBP - 16 - 8]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 80], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 88], 2
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 24]
	MOV R12, QWORD [RBP - 16 - 88]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 96], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 32]
	MOV R12, QWORD [RBP - 16 - 0]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 104], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 96]
	MOV R12, QWORD [RBP - 16 - 104]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 112], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 80]
	MOV R12, QWORD [RBP - 16 - 112]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 120], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 64]
	MOV R12, QWORD [RBP - 16 - 120]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 128], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 0]
	MOV R12, QWORD [RBP - 16 - 128]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 136], RAX
	MOV RAX, QWORD [RBP - 16 - 136]
	MOV QWORD [RBP - 16 - 16], RAX
	MOV RSI, QWORD [RBP - 16 - 16]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	MOV RSP, RBP
	POP RBP
	RET
