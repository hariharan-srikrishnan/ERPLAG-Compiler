section .data

integerRead db "%d", 0
integerWrite db "Output: %d", 10, 0
realRead db "%lf", 0
realWrite db "Output: %lf", 10, 0
integerarrayWrite db "%d ", 0
realarrayWrite db "%d ", 0
beginArrPrint db "Output: ", 0
endArrPrint db 10, 0
intMessage db "Enter an integer", 10, 0
realMessage db "Enter an real:", 10, 0
boolMessage db "Enter a boolean (1 or 0 only):", 10, 0
intArrMessage db "Enter integer array for range %d to %d:", 10, 0
realArrMessage db "Enter real array for range %d to %d:", 10, 0
boolArrMessage db "Enter boolean array for range %d to %d (1 or 0 only):", 10, 0
trueMessage db "Output: True", 10, 0
falseMessage db "Output: False", 10,  0
runTimeError db "Run-time error: Bound values error", 10, 0


section .text
extern printf
extern scanf
extern malloc
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
	SUB RSP, 64
	SUB RSP, 8
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 64], RAX
	MOV RAX, QWORD [RBP - 16 - 64]
	MOV QWORD [RBP - 16 - 48], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 72], 5
	MOV RAX, QWORD [RBP - 16 - 72]
	MOV QWORD [RBP - 16 - 24], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 80], 9
	MOV RAX, QWORD [RBP - 16 - 80]
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
	MOV   QWORD [RBP - 16 - 88], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 24]
	MOV R12, QWORD [RBP - 16 - 32]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 96], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 96]
	MOV R12, QWORD [RBP - 16 - 8]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 104], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 112], 2
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 24]
	MOV R12, QWORD [RBP - 16 - 112]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 120], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 32]
	MOV R12, QWORD [RBP - 16 - 0]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 128], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 120]
	MOV R12, QWORD [RBP - 16 - 128]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 136], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 104]
	MOV R12, QWORD [RBP - 16 - 136]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 144], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 88]
	MOV R12, QWORD [RBP - 16 - 144]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 152], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 0]
	MOV R12, QWORD [RBP - 16 - 152]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 160], RAX
	MOV RAX, QWORD [RBP - 16 - 160]
	MOV QWORD [RBP - 16 - 16], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 168], 10
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 16]
	MOV R12, QWORD [RBP - 16 - 168]
	MOV RAX, R11
	CMP RAX, R12
	JG label4
	JLE label5
label4: 
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 176], RAX
	JMP label6
label5: 
	MOV RAX, 0
	MOV QWORD [RBP - 16 - 176], RAX
label6: 
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 24]
	MOV R12, QWORD [RBP - 16 - 32]
	MOV RAX, R11
	CMP RAX, R12
	JLE label7
	JG label8
label7: 
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 184], RAX
	JMP label9
label8: 
	MOV RAX, 0
	MOV QWORD [RBP - 16 - 184], RAX
label9: 
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 0]
	MOV R12, QWORD [RBP - 16 - 8]
	MOV RAX, R11
	CMP RAX, R12
	JL label10
	JGE label11
label10: 
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 192], RAX
	JMP label12
label11: 
	MOV RAX, 0
	MOV QWORD [RBP - 16 - 192], RAX
label12: 
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 192]
	MOV R12, QWORD [RBP - 16 - 48]
	MOV RAX, R11
	AND RAX, R12
	MOV QWORD [RBP - 16 - 200], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 184]
	MOV R12, QWORD [RBP - 16 - 200]
	MOV RAX, R11
	AND RAX, R12
	MOV QWORD [RBP - 16 - 208], RAX
	SUB RSP, 8
	MOV R11, QWORD [RBP - 16 - 176]
	MOV R12, QWORD [RBP - 16 - 208]
	MOV RAX, R11
	OR  RAX, R12
	MOV QWORD [RBP - 16 - 216], RAX
	MOV RAX, QWORD [RBP - 16 - 216]
	MOV QWORD [RBP - 16 - 56], RAX
	MOV RSI, QWORD [RBP - 16 - 16]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	MOV RSI, QWORD [RBP - 16 - 48]
	MOV RSI, 0
	CMP QWORD [RBP - 16 - 48], 1
	JE label13
	JMP label14
label13: 
	LEA RDI, [trueMessage]
	JMP label15
label14: 
	LEA RDI, [falseMessage]
label15: 
	MOV AL, 0
	CALL printf
	MOV RSP, RBP
	POP RBP
	RET
