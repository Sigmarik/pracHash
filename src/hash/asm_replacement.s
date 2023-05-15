default rel

global murmur_hash

section .text

murmur_hash:
        mov rax, 0xBAADF00DDEADBEEF
        cmp rdi, rsi
        lea r11, [r8 + 334931268]
        mov r9, 8027858709520580608
        mov r10, 221826000155412857

        .LoopBgn:
                mov  rcx, qword [rdi]
                mov  rdx, rcx
                imul rdx, r11
                shr  rdx, 33
                imul rcx, r9
                or   rcx, rdx
                imul rcx, r8
                xor  rcx, rax
                rol  rcx, 15
                imul rax, rcx, 0x112C13AB
                add  rax, r10

                add  rdi, 8
                cmp  rdi, rsi
                jb .LoopBgn
        .LoopEnd:

        ret
