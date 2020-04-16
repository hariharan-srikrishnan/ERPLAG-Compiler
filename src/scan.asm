section .text
  global main
  extern printf
  extern scanf

section .data
  message: db "The result is = %d", 10, 0
  request: db "Enter the number: ", 0
  integer1: times 4 db 0 ; 32-bits integer = 4 bytes
  formatin: db "%d", 0

main:
  SUB RSP, 8;  Ask for an integer
  MOV RDI, request
  MOV AL, 0
  call printf
  add rsp, 8    ; remove the parameter

  MOV RSI, integer1 ; address of integer1, where the input is going to be stored (second parameter)
  MOV RDI, formatin ; arguments are right to left (first  parameter)
  MOV AL, 0
  call scanf
  sub rsp, 8    ; remove the parameters

  ; Move the value under the address integer1 to EAX
  mov rax, [integer1]

  ; Print out the content of eax register
  MOV RSI, rax
  MOV RDI, message
  MOV AL, 0
  call printf
  add rsp, 8

  ;  Linux terminate the app
  RET