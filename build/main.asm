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
; cur : ( 
; text: ( 
; lvl : 2 
; -------------- 
; cur : ( 
; text:  
; lvl : 3 
; -------------- 
; cur : 8 
; text: ( 
; lvl : 3 
; -------------- 
; cur : 8 
; text: ( 
; lvl : 3 
; -------------- 
; cur : + 
; text:  
; lvl : 4 
; -------------- 
; cur : 4 
; text: + 
; lvl : 4 
; -------------- 
; cur : 4 
; text:  
; lvl : 5 
; -------------- 
; cur : ) 
; text: 4 
; lvl : 5 
; -------------- 
push 4 
; cur : ) 
; text: 4 
; lvl : 5 
; -------------- 
; cur : * 
; text: ) 
; lvl : 5 
; -------------- 
pop rax 
pop rbx 
add rax, rbx 
push rax 

; cur : * 
; text: ( 
; lvl : 3 
; -------------- 
; cur : 3 
; text: ³_/ApU 
; lvl : 4 
; -------------- 
; cur : - 
; text: 3 
; lvl : 4 
; -------------- 
push 3 
; cur : - 
; text: 3 
; lvl : 4 
; -------------- 
; cur : ( 
; text: - 
; lvl : 4 
; -------------- 
; cur : ( 
; text: ãgQW 
; lvl : 5 
; -------------- 
; cur : 5 
; text: ( 
; lvl : 5 
; -------------- 
; cur : 5 
; text: ( 
; lvl : 5 
; -------------- 
; cur : - 
; text:  
; lvl : 6 
; -------------- 
; cur : 2 
; text: - 
; lvl : 6 
; -------------- 
; cur : 2 
; text:  
; lvl : 7 
; -------------- 
; cur : ) 
; text: 2 
; lvl : 7 
; -------------- 
push 2 
; cur : ) 
; text: 2 
; lvl : 7 
; -------------- 
; cur : * 
; text: ) 
; lvl : 7 
; -------------- 
pop rbx 
pop rax 
sub rax, rbx 
push rax 

; cur : * 
; text: ( 
; lvl : 5 
; -------------- 
; cur : 2 
; text: S_/ApU 
; lvl : 6 
; -------------- 
; cur : ) 
; text: 2 
; lvl : 6 
; -------------- 
push 2 
; cur : ) 
; text: 2 
; lvl : 6 
; -------------- 
; cur : / 
; text: ) 
; lvl : 6 
; -------------- 
; cur : / 
; text: ( 
; lvl : 5 
; -------------- 
; cur : 2 
; text: S_/ApU 
; lvl : 6 
; -------------- 
; cur : ) 
; text: 2 
; lvl : 6 
; -------------- 
push 2 
; cur : ) 
; text: 2 
; lvl : 6 
; -------------- 
; cur : ; 
; text: ) 
; lvl : 6 
; -------------- 
; cur : ÿÿÿÿ€8~uU 
; text: ); 
; lvl : 6 
; -------------- 
pop rbx 
pop rax 
sub rax, rbx 
push rax 

pop rbx 
lea rdi, [rbx] 
call print_int 

call exit
