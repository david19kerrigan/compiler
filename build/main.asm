align 16 
global _start 
extern print_int 
extern int_to_string 
extern exit 
 
section .text 
_start: 
; cur a 
; cur = 
; cur 1 
; cur + 
push 1 
; cur 2 
; cur ; 
pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
mov [rbp-16], rax 

; cur 
 
; cur b 
; cur = 
; cur 3 
; cur * 
push 3 
mov rax, [rbp--1534026480] 
; cur 2 
; cur ; 
push 2 
mov rax, [rbp--1534024944] 
pop rax 
pop rbx 
imul rax, rbx 
push rax 

; cur ; 
pop rax 
mov [rbp-32], rax 

; cur 
 
; cur p 
; cur r 
; cur i 
; cur n 
; cur t 
; cur ( 
; cur a 
; cur + 
mov rax, [rbp-16] 
; cur b 
; cur ) 
pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rbx 
lea rdi, [rbx] 
call print_int 

; cur ; 
; cur 
 
; cur ÿÿÿÿ 
call exit
