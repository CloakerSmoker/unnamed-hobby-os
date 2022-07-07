ROOT_EXTRA= TestFile.txt test.txt 
BIN_EXTRA= 

RLX_FLAGS?= --crlf --dwarf
EFI_RLX_FLAGS?= $(RLX_FLAGS) --pe-reloc --pe
ELF_RLX_FLAGS?= $(RLX_FLAGS) --linux

CLEAN_FILES=

BUILD=./build

Disk.qcow2: $(BUILD)/Disk.img
	qemu-img convert -f raw -O qcow2 $(BUILD)/Disk.img Disk.qcow2

CLEAN_FILES+= EFIBoot.qcow2

$(BUILD)/Disk.img: $(BUILD)/Temp.img.gpt $(BUILD)/Temp.img.fat32 $(BUILD)/Temp.img.ext2
	cp $(BUILD)/Temp.img $(BUILD)/Disk.img

CLEAN_FILES+= $(BUILD)/Temp.img $(BUILD)/Disk.img

$(BUILD)/Temp.img.gpt:
	mkdir -p $(BUILD)

	rm -f $(BUILD)/Temp.img

	./GPTTool.elf "File($(BUILD)/Temp.img,512)" \
		"format 120 m" \
		"create start 0x32 b end 70 m name \"EFI System\" type system" \
		"create start 71 m end 119 m name \"Boot\" type custom" \
		"quit"
	
	touch $@

CLEAN_FILES+= $(BUILD)/Temp.img.gpt

$(BUILD)/Temp.img.fat32: $(BUILD)/Temp.img.gpt $(BUILD)/Boot.efi
	./FAT32Tool.elf "File($(BUILD)/Temp.img,512)>GPT(0)" \
		"format 64 m" \
		"mkdir EFI" \
		"cd EFI" \
		"mkdir BOOT" \
		"cd BOOT" \
		"import $(BUILD)/Boot.efi BOOTX64.EFI" \
		"quit"
	
	touch $@

CLEAN_FILES+= $(BUILD)/Temp.img.fat32

$(BUILD)/Temp.img.ext2: $(BUILD)/Temp.img.gpt $(BUILD)/Kernel.elf
	./Ext2Tool.elf "File($(BUILD)/Temp.img,512)>GPT(1)" \
		"format 32 m" \
		"import Kernel.elf" \
		"import-all $(ROOT_EXTRA)" \
		"mkdir dev" \
		"cd dev" \
		"mknod tty1 c 4 1" \
		"mknod ttyS0 c 4 64" \
		"hard-link console tty1" \
		"cd .." \
		"mkdir bin" \
		"cd bin" \
		"import-all $(BIN_EXTRA)" \
		"quit"
	
	touch $@

CLEAN_FILES+= $(BUILD)/Temp.img.ext2

$(BUILD)/Boot.efi: ./bootloader/*.rlx
	cd ..; $(RLX) -i ./src/bootloader/EFIBoot.rlx -o ./src/$@ $(EFI_RLX_FLAGS) --pe-reloc --efi

CLEAN_FILES+= $(BUILD)/Boot.efi

$(BUILD)/Kernel.elf: ./kernel/*.rlx ./drivers/*.rlx ./utility/*.rlx ./linux/*.rlx
	cd ..; $(RLX) -i ./src/kernel/Main.rlx -o ./src/$@ ${ELF_RLX_FLAGS}

CLEAN_FILES+= $(BUILD)/Kernel.elf

clean:
	rm -f $(CLEAN_FILES)
	rm -df $(BUILD)