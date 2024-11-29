align 16 
global _start 
extern print_int 
extern int_to_string 
extern exit 
 
section .text 
_start: 
mov rbp, rsp 
push 22 
push 22 
push 22 
pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rbx 
lea rdi, [rbx] 
call print_int 

call exit
