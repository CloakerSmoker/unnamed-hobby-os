[BITS 16]
[ORG 0x7C00]

hd0 equ 0x80
read equ 0x02
sec1 equ 0x01
sec2 equ 0x02
	
	mov sp, TinyStack
	
	push 0x1400
	pop es
	
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

Oct2Bin:
	xor r8, r8
	xor r9, r9
	xor r10, r10
	mov r11, 11
	
	Oct2Bin_Loop:
		imul r8, 8
		mov r9b, byte [rcx + r10]
		
		sub r9, '0'
		add r8, r9
		inc r10
		dec r11
		
		or r11, r11
		jnz Oct2Bin_Loop
	ret
ret

UStar_FindKernel:
	mov rax, FS_Base
	xor esi, esi
	
	UStar_Loop:
		add rax, rsi
		
		mov ecx, dword [rax + 257]
		cmp ecx, 'usta'
		jne UStar_Fail
		
		lea rcx, [rax + 0x7c]
		call Oct2Bin
		
		mov ecx, dword [rax]
		cmp ecx, 'Kern'
		jne UStar_Next
		
		lea rax, [rax + 512]
		ret
		
		UStar_Next:
			xchg rax, r8
			xor edx, edx
			mov ebx, 512
			idiv ebx
			imul rax, 512
			add rax, 512
			
			cmp edx, 0
			jz UStar_Loop_
			add rax, 512
			UStar_Loop_:
			add rax, r8
	jmp UStar_Loop
	
	UStar_Fail:
	hlt

LongMain:
	mov rsp, TinyStack
	call UStar_FindKernel
	
	mov rbx, [rax + 8]
	xor edi, edi
LoadKernel:
	mov rcx, [rax + rdi]
	mov [rbx + rdi], rcx
	add rdi, 8
	cmp edi, r8d
	jl LoadKernel
	
	mov edx, FS_Base
	mov rsi, Intrinsics
	mov edi, r8d
	jmp rbx

SetCR3:
	mov cr3, rdi
ret
GetCR3:
	mov rax, cr3
ret

Intrinsics:
	dq SetCR3
	dq GetCR3

times 510 - ($ - $$) db 0
dw 0xAA55

TinyStack equ 0x1000
FS_Base equ 0x14000

INCBIN "Kernel.tar"

TIMES 64512 - ($ - $$) db 0

PageTables equ 0x8000

Ext2LoadExtendedBootLoader:
	mov bx
	
