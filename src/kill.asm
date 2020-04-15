; ----------------------------------------------------------------------------------------
; Writes "Hello, World" to the console using only system calls. Runs on 64-bit Linux only.
; To assemble and run:
;
;     nasm -felf64 hello.asm && ld hello.o && ./a.out
; ----------------------------------------------------------------------------------------

          global    main
          extern scanf

          section   .text
main:     mov       rax, 1                  ; system call for write
          mov       rdi, message                 ; file handle 1 is stdout
          mov rsi, x
          xor       rax, rax            ; address of string to output
          call scanf

          section   .data
message:  db        "%d", 10      ; note the newline at the end
x: dq 0.0