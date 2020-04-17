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
	SUB RSP, 112
	SUB RSP, 8
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 112], RAX
	MOV RAX, QWORD [RBP - 16 - 112]
	MOV QWORD [RBP - 16 - 48], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 120], 5
	MOV RAX, QWORD [RBP - 16 - 120]
	MOV QWORD [RBP - 16 - 24], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 128], 9
	MOV RAX, QWORD [RBP - 16 - 128]
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
	CMP R8, 1
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
	CMP R8, 1
	JNE label3
	ADD RSP, 8
label3: 
	SUB RSP, 8
	MOV R11, [RBP - 16 - 8]
	MOV R12, [RBP - 16 - 32]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 136], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 24]
	MOV R12, [RBP - 16 - 32]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 144], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 144]
	MOV R12, [RBP - 16 - 8]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 152], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 160], 2
	SUB RSP, 8
	MOV R11, [RBP - 16 - 24]
	MOV R12, [RBP - 16 - 160]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 168], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 32]
	MOV R12, [RBP - 16 - 0]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 176], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 168]
	MOV R12, [RBP - 16 - 176]
	MOV  RAX, R11
	SUB  RAX, R12
	MOV  QWORD [RBP - 16 - 184], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 152]
	MOV R12, [RBP - 16 - 184]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 192], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 136]
	MOV R12, [RBP - 16 - 192]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 200], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 0]
	MOV R12, [RBP - 16 - 200]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 208], RAX
	MOV RAX, QWORD [RBP - 16 - 208]
	MOV QWORD [RBP - 16 - 16], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 216], 10
	SUB RSP, 8
	MOV RAX, QWORD [RBP - 16 - 16]
	CMP RAX, QWORD [RBP - 16 - 216]
	JG label4
	JLE label5
label4: 
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 224], RAX
	JMP label6
label5: 
	MOV RAX, 0
	MOV QWORD [RBP - 16 - 224], RAX
label6: 
	SUB RSP, 8
	MOV RAX, QWORD [RBP - 16 - 24]
	CMP RAX, QWORD [RBP - 16 - 32]
	JLE label7
	JG label8
label7: 
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 232], RAX
	JMP label9
label8: 
	MOV RAX, 0
	MOV QWORD [RBP - 16 - 232], RAX
label9: 
	SUB RSP, 8
	MOV RAX, QWORD [RBP - 16 - 0]
	CMP RAX, QWORD [RBP - 16 - 8]
	JL label10
	JGE label11
label10: 
	MOV RAX, 1
	MOV QWORD [RBP - 16 - 240], RAX
	JMP label12
label11: 
	MOV RAX, 0
	MOV QWORD [RBP - 16 - 240], RAX
label12: 
	SUB RSP, 8
	MOV RAX, QWORD [RBP - 16 - 240]
	AND RAX, QWORD [RBP - 16 - 48]
	MOV QWORD [RBP - 16 - 248], RAX
	SUB RSP, 8
	MOV RAX, QWORD [RBP - 16 - 232]
	AND RAX, QWORD [RBP - 16 - 248]
	MOV QWORD [RBP - 16 - 256], RAX
	SUB RSP, 8
	MOV RAX, QWORD [RBP - 16 - 224]
	OR  RAX, QWORD [RBP - 16 - 256]
	MOV QWORD [RBP - 16 - 264], RAX
	MOV RAX, QWORD [RBP - 16 - 264]
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
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label18
	SUB RSP, 8
	MOV R13, 1
label18: 
	MOV RSI, 10
	MOV RDX, 15
	LEA RDI, [intArrMessage]
	MOV AL, 0
	CALL printf
	MOV R8, 10
	MOV R9, 0
label16: 
	CMP R8, 15
	JG label17
	PUSH R8
	PUSH R9
	MOV RAX, R9
	NEG RAX
	LEA RSI, [RBP + RAX * 8 - 16 - 64]
	LEA RDI, [integerRead]
	MOV AL, 0
	CALL scanf
	POP R9
	POP R8
	INC R8
	INC R9
	JMP label16
label17: 
	CMP R13, 1
	JNE label19
	ADD RSP, 8
label19: 
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 272], 2
	SUB RSP, 8
	MOV R10, 13
	MOV R8, 10
	MOV R9, 15
	CMP R8, R9
	JG _error
	CMP R8, R10
	JG _error
	CMP R9, R10
	JL _error
	MOV R11, R10
	SUB R11, R8
	MOV RAX, R11
	NEG RAX
	MOV R11, [RBP + RAX * 8 - 16 - 64]
	MOV R12, [RBP - 16 - 272]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 280], RAX
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 288], 3
	SUB RSP, 8
	MOV R10, 14
	MOV R8, 10
	MOV R9, 15
	CMP R8, R9
	JG _error
	CMP R8, R10
	JG _error
	CMP R9, R10
	JL _error
	MOV R11, R10
	SUB R11, R8
	MOV RAX, R11
	NEG RAX
	MOV R11, [RBP + RAX * 8 - 16 - 64]
	MOV R12, [RBP - 16 - 288]
	MOV   RAX, R11
	IMUL  R12
	MOV   QWORD [RBP - 16 - 296], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 296]
	MOV R10, 15
	MOV R8, 10
	MOV R9, 15
	CMP R8, R9
	JG _error
	CMP R8, R10
	JG _error
	CMP R9, R10
	JL _error
	MOV R12, R10
	SUB R12, R8
	MOV RAX, R12
	NEG RAX
	MOV R12, [RBP + RAX * 8 - 16 - 64]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 304], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 280]
	MOV R12, [RBP - 16 - 304]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 312], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 0]
	MOV R12, [RBP - 16 - 312]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 320], RAX
	SUB RSP, 8
	MOV R11, [RBP - 16 - 24]
	MOV R12, [RBP - 16 - 320]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 328], RAX
	MOV RAX, QWORD [RBP - 16 - 328]
	MOV QWORD [RBP - 16 - 8], RAX
	MOV RSI, QWORD [RBP - 16 - 8]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	MOV R8, 10
	MOV R9, 0
label26: 
	CMP R8, 15
	JG label27
	PUSH R8
	PUSH R9
	MOV RAX, R9
	NEG RAX
	MOV RSI, [RBP + RAX * 8 - 16 - 64]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	POP R9
	POP R8
	INC R8
	INC R9
	JMP label26
label27: 
	MOV RSP, RBP
	POP RBP
	RET
