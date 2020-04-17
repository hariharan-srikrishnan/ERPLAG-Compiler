; ----------------------------------------------------------------------------------------
; Writes "Hello, World" to the console using only system calls. Runs on 64-bit Linux only.
; To assemble and run:
;
;     nasm -felf64 hello.asm && ld hello.o && ./a.out
; ----------------------------------------------------------------------------------------

          global    main
          extern scanf
          extern printf

section   .text
main:     
          lea rdi, [message]                
          lea rsi, [x]
          xor rax, rax           
          LEA RSI, [RBP - 16 - 64 - R9W * 8]

section   .data
message:  db        "%d", 10      ; note the newline at the end
x dq 5
