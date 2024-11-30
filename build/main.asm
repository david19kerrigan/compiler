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

; set |a|
pop rax 
mov [rbp-32], rax 

push 3 
push 2 
; recall |a|
mov rax, [rbp-32] 
push rax 

pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
pop rbx 
imul rax, rbx 
push rax 

push 5 
pop rax 
pop rbx 
imul rax, rbx 
push rax 

; set |b|
pop rax 
mov [rbp-64], rax 

push 8 
push 5 
; recall |a|
mov rax, [rbp-32] 
push rax 

pop rbx 
pop rax 
sub rax, rbx 
push rax 

; recall |b|
mov rax, [rbp-64] 
push rax 

pop rax 
pop rbx 
imul rax, rbx 
push rax 

push 2 
pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
pop rbx 
imul rax, rbx 
push rax 

push 2 
pop rax 
pop rbx 
imul rax, rbx 
push rax 

; set |c|
pop rax 
mov [rbp-96], rax 

; recall |a|
mov rax, [rbp-32] 
push rax 

push 2 
; recall |b|
mov rax, [rbp-64] 
push rax 

push 3 
; recall |c|
mov rax, [rbp-96] 
push rax 

pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
pop rbx 
imul rax, rbx 
push rax 

push 3 
pop rax 
pop rbx 
imul rax, rbx 
push rax 

pop rax 
pop rbx 
add rax, rbx 
push rax 

pop rax 
pop rbx 
imul rax, rbx 
push rax 

push 3 
pop rbx 
pop rax 
sub rax, rbx 
push rax 

pop rbx 
lea rdi, [rbx] 
call print_int 

call exit
