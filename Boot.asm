[BITS 16]	;Tells the assembler that its a 16 bit code
[ORG 0x7C00]	;Origin, tell the assembler that where the code will
	;be in memory after it is been loaded

hd0 equ 0x80
read equ 0x02
sec1 equ 0x01
sec2 equ 0x02
cy_size equ (63 * 512)
	
	mov bx, 0x7e00
	mov al, 63
	mov ch, 0
	mov cl, sec2
	call ReadSectors
	
	add bx, cy_size
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

TIMES 510 - ($ - $$) db 0	;Fill the rest of sector with 0
DW 0xAA55			;Add boot signature at the end of bootloader

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
	jmp RelaxStub

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
