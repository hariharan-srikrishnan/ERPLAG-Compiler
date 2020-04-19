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
	SUB RSP, 176
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label2
	SUB RSP, 8
	MOV R13, 1
label2: 
	MOV RSI, 6
	MOV RDX, 10
	LEA RDI, [intArrMessage]
	MOV AL, 0
	CALL printf
	MOV R8, 6
	MOV R9, 0
label0: 
	CMP R8, 10
	JG label1
	PUSH R8
	PUSH R9
	MOV RAX, R9
	NEG RAX
	LEA RSI, [RBP + RAX * 8 - 16 - 56]
	LEA RDI, [integerRead]
	MOV AL, 0
	CALL scanf
	POP R9
	POP R8
	INC R8
	INC R9
	JMP label0
label1: 
	CMP R13, 1
	JNE label3
	ADD RSP, 8
label3: 
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label6
	SUB RSP, 8
	MOV R13, 1
label6: 
	MOV RSI, 6
	MOV RDX, 10
	LEA RDI, [intArrMessage]
	MOV AL, 0
	CALL printf
	MOV R8, 6
	MOV R9, 0
label4: 
	CMP R8, 10
	JG label5
	PUSH R8
	PUSH R9
	MOV RAX, R9
	NEG RAX
	LEA RSI, [RBP + RAX * 8 - 16 - 96]
	LEA RDI, [integerRead]
	MOV AL, 0
	CALL scanf
	POP R9
	POP R8
	INC R8
	INC R9
	JMP label4
label5: 
	CMP R13, 1
	JNE label7
	ADD RSP, 8
label7: 
	SUB RSP, 8
	MOV QWORD [RBP - 16 - 176], 7
	MOV RAX, QWORD [RBP - 16 - 176]
	MOV QWORD [RBP - 16 - 16], RAX
	MOV R15, 1
label8: 
	MOV QWORD [RBP - 16 - 24], R15
	CMP R15, 3
	JG label9
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label10
	SUB RSP, 8
	MOV R8, 1
label10: 
	LEA RDI, [intMessage]
	MOV AL, 0
	CALL printf
	LEA RDI, [integerRead]
	LEA RSI, [RBP - 16 - 0]
	MOV AL, 0
	CALL scanf
	CMP R8D, 1
	JNE label11
	ADD RSP, 8
label11: 
	MOV RAX, RSP
	MOV RDX, 0
	MOV R9, 16
	IDIV R9
	CMP RDX, 0
	JE label12
	SUB RSP, 8
	MOV R8, 1
label12: 
	LEA RDI, [intMessage]
	MOV AL, 0
	CALL printf
	LEA RDI, [integerRead]
	LEA RSI, [RBP - 16 - 8]
	MOV AL, 0
	CALL scanf
	CMP R8D, 1
	JNE label13
	ADD RSP, 8
label13: 
	SUB RSP, 8
	MOV R8, 6
	MOV R9, 10
	CMP R8D, R9D
	JG _error
	MOV R10, QWORD [RBP - 16 - 0]
	CMP R8D, R10D
	JG _error
	CMP R9D, R10D
	JL _error
	MOVSX R11, R10D
	SUB R11, R8
	MOV RAX, R11
	NEG RAX
	MOV R11, QWORD [RBP + RAX * 8 - 16 - 56]
	MOV R10, QWORD [RBP - 16 - 8]
	MOV R8, 6
	MOV R9, 10
	CMP R8D, R9D
	JG _error
	CMP R8D, R10D
	JG _error
	CMP R9D, R10D
	JL _error
	MOVSX R12, R10D
	SUB R12, R8
	MOV RAX, R12
	NEG RAX
	MOV R12, QWORD [RBP + RAX * 8 - 16 - 96]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 184], RAX
	MOV RAX, QWORD [RBP - 16 - 184]
	MOV QWORD [RBP - 16 - 32], RAX
	SUB RSP, 8
	MOV R8, 6
	MOV R9, 10
	CMP R8D, R9D
	JG _error
	MOV R10, QWORD [RBP - 16 - 16]
	CMP R8D, R10D
	JG _error
	CMP R9D, R10D
	JL _error
	MOVSX R11, R10D
	SUB R11, R8
	MOV RAX, R11
	NEG RAX
	MOV R11, QWORD [RBP + RAX * 8 - 16 - 56]
	MOV R10, QWORD [RBP - 16 - 8]
	MOV R8, 6
	MOV R9, 10
	CMP R8D, R9D
	JG _error
	CMP R8D, R10D
	JG _error
	CMP R9D, R10D
	JL _error
	MOVSX R12, R10D
	SUB R12, R8
	MOV RAX, R12
	NEG RAX
	MOV R12, QWORD [RBP + RAX * 8 - 16 - 96]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 192], RAX
	MOV RAX, QWORD [RBP - 16 - 192]
	MOV QWORD [RBP - 16 - 40], RAX
	SUB RSP, 8
	MOV R8, 6
	MOV R9, 10
	CMP R8D, R9D
	JG _error
	MOV R10, QWORD [RBP - 16 - 0]
	CMP R8D, R10D
	JG _error
	CMP R9D, R10D
	JL _error
	MOVSX R11, R10D
	SUB R11, R8
	MOV RAX, R11
	NEG RAX
	MOV R11, QWORD [RBP + RAX * 8 - 16 - 56]
	MOV R10, QWORD [RBP - 16 - 16]
	MOV R8, 6
	MOV R9, 10
	CMP R8D, R9D
	JG _error
	CMP R8D, R10D
	JG _error
	CMP R9D, R10D
	JL _error
	MOVSX R12, R10D
	SUB R12, R8
	MOV RAX, R12
	NEG RAX
	MOV R12, QWORD [RBP + RAX * 8 - 16 - 96]
	MOV  RAX, R11
	ADD  RAX, R12
	MOV  QWORD [RBP - 16 - 200], RAX
	MOV RAX, QWORD [RBP - 16 - 200]
	MOV QWORD [RBP - 16 - 48], RAX
	MOV RSI, QWORD [RBP - 16 - 32]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	MOV RSI, QWORD [RBP - 16 - 40]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	MOV RSI, QWORD [RBP - 16 - 48]
	LEA RDI, [integerWrite]
	MOV AL, 0
	CALL printf
	INC R15
	JMP label8
label9: 
	MOV RSP, RBP
	POP RBP
	RET
