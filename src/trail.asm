global main
extern printf, scanf

section .data
   msg: db "Enter a number: ",10,0
   format: db "%d",0
   x: dd 0

section .bss
   number resb 8

section .text
main:
   sub  rsp, 8       ; align the stack to a 16B boundary before function call
   mov rdi, msg
   xor rax, rax
   call printf
   
   mov rsi, x
   mov rdi, format
   xor rax, rax
   call scanf

   mov rsi, [x]
   mov rdi, format
   xor rax, rax
   call printf
   ADD   rsp, 8      ; restore the stack
   ret