global _start 
extern print_int 
extern int_to_string 
extern exit 
 
section .text 
_start: 
mov rbp, rsp 
; cur : p 
; text:  
; -------------- 
; cur : r 
; text: p 
; -------------- 
; cur : i 
; text: pr 
; -------------- 
; cur : n 
; text: pri 
; -------------- 
; cur : t 
; text: prin 
; -------------- 
; cur : ( 
; text: print 
; -------------- 
; cur : ( 
; text: ï¡ÆW 
; -------------- 
; cur : 8 
; text: ( 
; -------------- 
; cur : < 
; text:  
; -------------- 
; cur : 9 
; text: < 
; -------------- 
; cur : 9 
; text:  
; -------------- 
pop rax 
pop rbx 
cmp rax, rbx 
push rax 

; cur : & 
; text: <9 
; -------------- 
push <9 
; cur : & 
; text: 8 
; -------------- 
push 8 
; cur : & 
; text: /¦Ù=yU 
; -------------- 
; cur : & 
; text: & 
; -------------- 
; cur : 1 
; text: && 
; -------------- 
; cur : 1 
; text:  
; -------------- 
; cur : < 
; text: 1 
; -------------- 
push 1 
pop rax 
pop rbx 
and rax, rbx 
push rax 

; cur : < 
; text: /¦Ù=yU 
; -------------- 
; cur : 2 
; text: < 
; -------------- 
; cur : 2 
; text:  
; -------------- 
pop rax 
pop rbx 
cmp rax, rbx 
push rax 

; cur : ) 
; text: <2 
; -------------- 
; cur : ; 
; text: <2) 
; -------------- 
; cur : 
 
; text: <2); 
; -------------- 
; cur : ÿÿÿÿ€÷j|U 
; text: <2);
 
; -------------- 
call exit
