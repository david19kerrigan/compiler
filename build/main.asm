align 16 
global _start 
extern print_int 
extern int_to_string 
extern exit 
 
section .text 
_start: 
mov rbp, rsp 
push 1 
push 2 
pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
mov [rbp-32], rax 

push 3 
push 2 
pop rax 
pop rbx 
imul rax, rbx 
push rax 

pop rax 
mov [rbp-64], rax 

mov rax, [rbp-32] 
push rax 

mov rax, [rbp-64] 
push rax 

push 3 
pop rax 
pop rbx 
imul rax, rbx 
push rax 

; recall 
pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rbx 
lea rdi, [rbx] 
call print_int 

call exit
