[BITS 16]
[ORG 0x7C00]

hd0 equ 0x80
read equ 0x02
sec1 equ 0x01
sec2 equ 0x02
	
	mov sp, TinyStack
	
	mov bx, 0x7e00
	mov al, 62
	mov ch, 0
	mov cl, sec2
	call ReadSectors
	
	push 0xfa0
	pop es
	mov bx, 0
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

times 510 - ($ - $$) db 0
dw 0xAA55

TinyStack equ $

WriteCharacter:
	mov ah, 0x0e
	mov bh, 0
	mov bl, 0x07
	int 0x10
ret

WriteAString:
	mov al, [si]
	test al, al
	jz Exit
	
	call WriteCharacter
	inc si
	jmp WriteAString

	Exit:
ret

Hello db 'Hello world!', 0

ShortMain:
	mov si, Hello
	call WriteAString
ret

%include "./src/LongModeDirectly.asm"

[BITS 64]

Halt:
	hlt
	jmp Halt

LongMain:
	mov rsp, Stack
	push Halt
	mov rdi, (KernelLimit - KernelBase) + 0xC000
	mov rsi, Intrinsics
	jmp RelaxStub

SetCR3:
	mov cr3, rdi
ret
GetCR3:
	mov rax, cr3
ret

Intrinsics:
	dq SetCR3
	dq GetCR3

TIMES 0x400 - ($ - $$) nop

PageTables equ $

TIMES 0x4000 db 0

KernelBase equ $

RelaxStub:
INCBIN "Kernel.bin"

KernelLimit equ $

TIMES 64504 - ($ - $$) db 0

Stack equ $

TIMES 64512 - ($ - $$) db 0
