align 16
global print_int
print_int:
    mov rax, rdi
    mov ecx, 0xa
    push rcx
    mov rsi, rsp

.int_to_string:
    xor edx, edx
    idiv ecx
    add edx, '0'
    dec rsi
    mov [rsi], dl
    test eax, eax
    jnz .int_to_string
    mov eax, 1
    mov edi, 1
    lea rdx, [rsp+1]
    sub rdx, rsi
    syscall
    add rsp, 8
    ret

global exit
exit:
    mov rax, 60
    xor rdi, rdi
    syscall
    ret

