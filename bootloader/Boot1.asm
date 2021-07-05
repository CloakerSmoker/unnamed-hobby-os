[BITS 16]
[ORG 0x7C00]

hd0 equ 0x80
read equ 0x02
sec1 equ 0x01
sec2 equ 0x02

FrameBuffer equ 0x7000
BootDisk equ 0x7002
	
	mov byte [BootDisk], dh
	
	mov sp, TinyStack
	
	push 0x1400
	pop es
	
	mov ax, 0x4f01
	mov cx, 0x118 | (1 << 14)
	mov di, 0x100
	
	int 0x10
	
	mov ebx, dword [es:di+40]
	mov dword [FrameBuffer], ebx
	
	mov ax, 0x4f02
	mov bx, 0x118 | (1 << 14)
	
	int 0x10
	
	push 0x1000
	pop es
	
	mov dword [LoadSecondStage_DAP_LBA], 11
	
	mov dh, byte [BootDisk]
	mov ah, 0x42
	mov si, LoadSecondStage_DAP
	int 0x13
	
	mov eax, dword [es:0x28]
	shl eax, 1
	
	mov dword [LoadSecondStage_DAP_LBA], eax
	mov word [LoadSecondStage_DAP_SectorCount], 22
	mov word [LoadSecondStage_DAP_Segment], 0x1000
	mov word [LoadSecondStage_DAP_Offset], 0
	
	mov dh, byte [BootDisk]
	mov ah, 0x42
	mov si, LoadSecondStage_DAP
	int 0x13
	
	jmp Start
	
LoadSecondStage_DAP:
	db 0x10
	db 0
LoadSecondStage_DAP_SectorCount:
	dw 1
LoadSecondStage_DAP_Offset: 
	dw 0
LoadSecondStage_DAP_Segment: 
	dw 0x1000
LoadSecondStage_DAP_LBA: 
	dd 0
	dd 0

%include "./src/bootloader/LongModeDirectly.asm"

[BITS 64]

LongMain:
	xor ax, ax
	mov rsp, Stack
	
	mov rdi, [FrameBuffer]
	jmp 0x10000

times 510 - ($ - $$) db 0
dw 0xAA55

TinyStack equ 0x1000

PageTables equ 0x1000
Stack equ 0x20_0000
