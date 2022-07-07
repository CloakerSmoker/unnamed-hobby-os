ROOT_EXTRA= TestFile.txt test.txt 
BIN_EXTRA= 

RLX_FLAGS?= --crlf --dwarf --debug
EFI_RLX_FLAGS?= $(RLX_FLAGS) --pe-reloc --pe
ELF_RLX_FLAGS?= $(RLX_FLAGS) --linux

RLX?=no-rlx-compiler-set

CLEAN_FILES=
LIGHT_CLEAN_FILES=

BUILD=./build

Disk.qcow2: $(BUILD)/Disk.img
	qemu-img convert -f raw -O qcow2 $(BUILD)/Disk.img Disk.qcow2

LIGHT_CLEAN_FILES+= EFIBoot.qcow2

$(BUILD)/Disk.img: $(BUILD)/Temp.img.gpt $(BUILD)/Temp.img.fat32 $(BUILD)/Temp.img.ext2
	cp $(BUILD)/Temp.img $(BUILD)/Disk.img

LIGHT_CLEAN_FILES+= $(BUILD)/Temp.img $(BUILD)/Disk.img

$(BUILD)/Temp.img.gpt: $(BUILD)/GPTTool.elf
	rm -f $(BUILD)/Temp.img

	$(BUILD)/GPTTool.elf "File($(BUILD)/Temp.img,512)" \
		"format 120 m" \
		"create start 0x32 b end 70 m name \"EFI System\" type system" \
		"create start 71 m end 119 m name \"Boot\" type custom" \
		"quit"
	
	touch $@

LIGHT_CLEAN_FILES+= $(BUILD)/Temp.img.gpt

$(BUILD)/GPTTool.elf: ./host/GPTTool.rlx ./drivers/block-device/*.rlx ./drivers/GPT.rlx
	cd ..; $(RLX) -i ./src/host/GPTTool.rlx -o ./src/$@ ${ELF_RLX_FLAGS}

CLEAN_FILES+= $(BUILD)/GPTTool.elf

$(BUILD)/Temp.img.fat32: $(BUILD)/Temp.img.gpt $(BUILD)/Boot.efi $(BUILD)/FAT32Tool.elf
	$(BUILD)/FAT32Tool.elf "File($(BUILD)/Temp.img,512)>GPT(0)" \
		"format 64 m" \
		"mkdir EFI" \
		"cd EFI" \
		"mkdir BOOT" \
		"cd BOOT" \
		"import $(BUILD)/Boot.efi BOOTX64.EFI" \
		"quit"
	
	touch $@

LIGHT_CLEAN_FILES+= $(BUILD)/Temp.img.fat32

$(BUILD)/FAT32Tool.elf: ./host/FAT32Tool.rlx ./drivers/block-device/*.rlx ./drivers/FAT32.rlx
	cd ..; $(RLX) -i ./src/host/FAT32Tool.rlx -o ./src/$@ ${ELF_RLX_FLAGS}

CLEAN_FILES+= $(BUILD)/FAT32Tool.elf

$(BUILD)/Temp.img.ext2: $(BUILD)/Temp.img.gpt $(BUILD)/Kernel.elf $(BUILD)/Ext2Tool.elf
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

LIGHT_CLEAN_FILES+= $(BUILD)/Temp.img.ext2

$(BUILD)/Ext2Tool.elf: ./host/Ext2Tool.rlx ./drivers/block-device/*.rlx ./drivers/Ext2.rlx
	cd ..; $(RLX) -i ./src/host/Ext2Tool.rlx -o ./src/$@ ${ELF_RLX_FLAGS}

CLEAN_FILES+= $(BUILD)/Ext2Tool.elf

$(BUILD)/Boot.efi: ./bootloader/*.rlx
	cd ..; $(RLX) -i ./src/bootloader/EFIBoot.rlx -o ./src/$@ $(EFI_RLX_FLAGS) --pe-reloc --efi

LIGHT_CLEAN_FILES+= $(BUILD)/Boot.efi

$(BUILD)/Kernel.elf: ./kernel/*.rlx ./drivers/*.rlx ./utility/*.rlx ./linux/*.rlx
	cd ..; $(RLX) -i ./src/kernel/Main.rlx -o ./src/$@ ${ELF_RLX_FLAGS}

LIGHT_CLEAN_FILES+= $(BUILD)/Kernel.elf

kernel/core/%.rlx: kernel/core/generated/%.py
	python3 $^

gen: kernel/core/*.rlx

tools: $(BUILD)/GPTTool.elf $(BUILD)/FAT32Tool.elf $(BUILD)/Ext2Tool.elf

fast: $(BUILD)/Disk.img

clean:
	rm -f $(LIGHT_CLEAN_FILES)

clean-all: clean
	rm -f $(CLEAN_FILES)