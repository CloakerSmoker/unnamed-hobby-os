[BITS 16]
[ORG 0x7C00]

hd0 equ 0x80
read equ 0x02
sec1 equ 0x01
sec2 equ 0x02
	
	mov sp, TinyStack
	
	push 0x1400
	pop es
	
	mov ax, 0x4f01
	mov cx, 0x118 | (1 << 14)
	mov di, 0x100
	
	int 0x10
	
	mov ebx, dword [es:di+40]
FrameBuffer equ 0x7000
	mov dword [FrameBuffer], ebx
	
	mov ax, 0x4f02
	mov bx, 0x118 | (1 << 14)
	
	int 0x10
	
	mov bx, 0
	mov al, 62
	mov ch, 0
	mov cl, sec2
	call ReadSectors
	
	mov bx, 0x7e00
	mov al, 63
	mov ch, 1
	mov cl, sec1
	call ReadSectors
	
	jmp Start
	
ReadSectors:
	mov ah, read
	mov dh, 0
	mov dl, hd0
	int 0x13
ret

%include "./src/LongModeDirectly.asm"

[BITS 64]

LongMain:
	xor ax, ax
	mov rsp, Stack
	
	mov rax, FS_Base
	
	mov rbx, [rax + 8]
	mov r8d, (KernelLimit - KernelStart)
	
	xor edi, edi
LoadKernel:
	mov rcx, [rax + rdi]
	mov [rbx + rdi], rcx
	inc edi
	cmp edi, r8d
	jl LoadKernel
	
	mov edx, FS_Base
	mov esi, [FrameBuffer]
	mov edi, r8d
	jmp rbx

times 510 - ($ - $$) db 0
dw 0xAA55

TinyStack equ 0x1000
FS_Base equ 0x14000

KernelStart equ $

INCBIN "./src/FS/Kernel.bin"

KernelLimit equ $

TIMES 64504 - ($ - $$) db 0

Stack equ $

TIMES 64512 - ($ - $$) db 0

PageTables equ 0x8000
