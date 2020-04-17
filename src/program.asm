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
	SUB RSP, 40
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 40], 19
	MOV RAX, QWORD [RBP - 16 - 40]
	MOV QWORD [RBP - 16 - 8], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 48], 56
	MOV RAX, QWORD [RBP - 16 - 48]
	MOV QWORD [RBP - 16 - 16], RAX
	MOV R15, 3
label0: 
	MOV QWORD [RBP - 16 - 32], R15
	CMP R15, 6
	JG label1
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
	LEA RSI, [RBP - 16 - 0]
	MOV AL, 0
	CALL scanf
	CMP R8, 1
	JNE label3
	ADD RSP, 8
label3: 
	MOV R8D, [RBP - 16 - 0]
	CMP R8D, 1
	JE label4
	CMP R8D, 2
	JE label5
	JMP label7
label4: 
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 124], 2
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 0]
	MOV R12, QWORD [RBP - 16 - 124]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 132], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 8]
	MOV R12, QWORD [RBP - 16 - 32]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 140], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 132]
	MOV R12, QWORD [RBP - 16 - 140]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 148], RAX
	MOV RAX, QWORD [RBP - 16 - 148]
	MOV QWORD [RBP - 16 - 8], RAX
	MOV RSI, QWORD [RBP - 16 - 8]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	JMP label6
label5: 
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 156], 3
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 0]
	MOV R12, QWORD [RBP - 16 - 156]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 164], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 16]
	MOV R12, QWORD [RBP - 16 - 32]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 172], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 164]
	MOV R12, QWORD [RBP - 16 - 172]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 180], RAX
	MOV RAX, QWORD [RBP - 16 - 180]
	MOV QWORD [RBP - 16 - 16], RAX
	MOV RSI, QWORD [RBP - 16 - 16]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	JMP label6
label7: 
	MOV RSI, QWORD [RBP - 16 - 0]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
label6: 
	MOV RSI, QWORD [RBP - 16 - 8]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	MOV RSI, QWORD [RBP - 16 - 16]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	INC R15
	JMP label0
label1: 
	MOV RSP, RBP
	POP RBP
	RET
