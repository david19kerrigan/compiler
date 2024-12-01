global _start 
extern print_int 
extern int_to_string 
extern exit 
 
section .text 
_start: 
mov rbp, rsp 
; cur : p 
; text:  
; lvl : 1 
; -------------- 
; cur : r 
; text: p 
; lvl : 1 
; -------------- 
; cur : i 
; text: pr 
; lvl : 1 
; -------------- 
; cur : n 
; text: pri 
; lvl : 1 
; -------------- 
; cur : t 
; text: prin 
; lvl : 1 
; -------------- 
; cur : ( 
; text: print 
; lvl : 1 
; -------------- 
; cur : ( 
; text:  
; lvl : 2 
; -------------- 
; cur : 8 
; text: ( 
; lvl : 2 
; -------------- 
; cur : 8 
; text:  
; lvl : 3 
; -------------- 
; cur : < 
; text: 8 
; lvl : 3 
; -------------- 
push 8 
; cur : < 
; text:  
; lvl : 3 
; -------------- 
; cur : 9 
; text: < 
; lvl : 3 
; -------------- 
; cur : 9 
; text:  
; lvl : 4 
; -------------- 
; cur : & 
; text: 9 
; lvl : 4 
; -------------- 
push 9 
pop rax 
pop rbx 
cmp rax, rbx 
push rax 

; cur : & 
; text:  
; lvl : 3 
; -------------- 
; cur : & 
; text: & 
; lvl : 3 
; -------------- 
; cur : 1 
; text: && 
; lvl : 3 
; -------------- 
; cur : 1 
; text:  
; lvl : 4 
; -------------- 
; cur : < 
; text: 1 
; lvl : 4 
; -------------- 
push 1 
; cur : < 
; text:  
; lvl : 4 
; -------------- 
; cur : 2 
; text: < 
; lvl : 4 
; -------------- 
; cur : 2 
; text:  
; lvl : 5 
; -------------- 
; cur : ) 
; text: 2 
; lvl : 5 
; -------------- 
push 2 
pop rax 
pop rbx 
cmp rax, rbx 
push rax 

; cur : ) 
; text:  
; lvl : 4 
; -------------- 
; cur : ; 
; text: ) 
; lvl : 4 
; -------------- 
; cur : 
 
; text: ); 
; lvl : 4 
; -------------- 
; cur : ÿÿÿÿ Ø"˜VU 
; text: );
 
; lvl : 4 
; -------------- 
pop rax 
pop rbx 
and rax, rbx 
push rax 

pop rbx 
lea rdi, [rbx] 
call print_int 

call exit
