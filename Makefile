# Config-ish

ROOT_EXTRA= TestFile.txt test.txt sponge.six dum.six
BIN_EXTRA=cat ed hexedit vi

RLX_FLAGS?= --crlf --dwarf --debug --silent
EFI_RLX_FLAGS?= $(RLX_FLAGS) --pe-reloc --pe --efi
ELF_RLX_FLAGS?= $(RLX_FLAGS) --linux
KERNEL_RLX_FLAGS?= $(RLX_FLAGS) --standalone-elf --platform kernel --platform-dir src/kernel/lib

RLX?=compiler/build/linux_compiler.elf

BUILD=./build

# Files to clean

CLEAN_FILES=
LIGHT_CLEAN_FILES=

# Preprocess

%.rlx: %.prlx
# Replace '#'->'\a', '%'->'#'
# Run CPP (%define -> #define)
# Replace '\a'->'#' (\aRequires -> #Requires)
	cat $^ | tr "#%" "\a#" | cpp -P -o - | tr "\a" "#" > $@

# qcow2 "release" image

Disk.qcow2: $(BUILD)/Disk.img
	qemu-img convert -f raw -O qcow2 $(BUILD)/Disk.img Disk.qcow2

LIGHT_CLEAN_FILES+= EFIBoot.qcow2

# Full disk image

$(BUILD)/Disk.img: $(BUILD)/GPTTool.elf
$(BUILD)/Disk.img: $(BUILD)/FAT32Tool.elf $(BUILD)/Boot.efi
$(BUILD)/Disk.img: $(BUILD)/Ext2Tool.elf $(BUILD)/Kernel.elf
$(BUILD)/Disk.img: $(BIN_EXTRA)
$(BUILD)/Disk.img: $(BUILD)/Beep.elf
$(BUILD)/Disk.img:
	rm -f $@

	$(BUILD)/GPTTool.elf "File($@,512)" \
		"format 120 m" \
		"create start 0x32 b end 70 m name \"EFI System\" type system" \
		"create start 71 m end 119 m name \"Boot\" type custom" \
		"quit"
	
	$(BUILD)/FAT32Tool.elf "File($@,512)>GPT(0)" \
		"format 64 m" \
		"mkdir EFI" \
		"cd EFI" \
		"mkdir BOOT" \
		"cd BOOT" \
		"import $(BUILD)/Boot.efi BOOTX64.EFI" \
		"quit"
	
	$(BUILD)/Ext2Tool.elf "File($@,512)>GPT(1)" \
		"format 32 m" \
		"import $(BUILD)/Kernel.elf Kernel.elf" \
		"import-all $(ROOT_EXTRA)" \
		"mkdir dev" \
		"cd dev" \
		"mknod tty1 c 4 1" \
		"mknod ttyS0 c 4 64" \
		"hard-link console tty1" \
		"mknod pc-speaker c 10 129" \
		"cd .." \
		"mkdir bin" \
		"cd bin" \
		"import-all $(BIN_EXTRA)" \
		"import $(BUILD)/Beep.elf beep" \
		"quit"

LIGHT_CLEAN_FILES+= $(BUILD)/Disk.img

# GPTTool

$(BUILD)/GPTTool.elf: $(RLX)
$(BUILD)/GPTTool.elf: $(BUILD)/GPTTool.d
$(BUILD)/GPTTool.elf: $(shell cat $(BUILD)/GPTTool.d 2>/dev/null)
	$(RLX) -i ./src/host/GPTTool.rlx -o $@ ${ELF_RLX_FLAGS}

secret-internal-deps: $(BUILD)/GPTTool.d

$(BUILD)/GPTTool.d: $(RLX)
	$(RLX) -i ./src/host/GPTTool.rlx -o $@ --makedep $(ELF_RLX_FLAGS)

CLEAN_FILES+= $(BUILD)/GPTTool.elf $(BUILD)/GPTTool.d

# FAT32Tool

$(BUILD)/FAT32Tool.elf: $(RLX)
$(BUILD)/FAT32Tool.elf: $(BUILD)/FAT32Tool.d
$(BUILD)/FAT32Tool.elf: $(shell cat $(BUILD)/FAT32Tool.d 2>/dev/null)
	$(RLX) -i ./src/host/FAT32Tool.rlx -o $@ ${ELF_RLX_FLAGS}

secret-internal-deps: $(BUILD)/FAT32Tool.d

$(BUILD)/FAT32Tool.d: $(RLX)
$(BUILD)/FAT32Tool.d:
	$(RLX) -i ./src/host/FAT32Tool.rlx -o $@ --makedep $(ELF_RLX_FLAGS)

CLEAN_FILES+= $(BUILD)/FAT32Tool.elf $(BUILD)/FAT32Tool.d

# Ext2Tool

$(BUILD)/Ext2Tool.elf: $(RLX)
$(BUILD)/Ext2Tool.elf: $(BUILD)/Ext2Tool.d
$(BUILD)/Ext2Tool.elf: $(shell cat $(BUILD)/Ext2Tool.d 2>/dev/null)
	$(RLX) -i ./src/host/Ext2Tool.rlx -o $@ ${ELF_RLX_FLAGS}

secret-internal-deps: $(BUILD)/Ext2Tool.d

$(BUILD)/Ext2Tool.d: $(RLX)
	$(RLX) -i ./src/host/Ext2Tool.rlx -o $@ --makedep $(ELF_RLX_FLAGS)

CLEAN_FILES+= $(BUILD)/Ext2Tool.elf $(BUILD)/Ext2Tool.d

# Bootloader

$(BUILD)/Boot.efi: $(RLX)
$(BUILD)/Boot.efi: $(BUILD)/Boot.d
$(BUILD)/Boot.efi: $(shell cat $(BUILD)/Boot.d 2>/dev/null)
	$(RLX) -i ./src/bootloader/EFIBoot.rlx -o $@ $(EFI_RLX_FLAGS)

secret-internal-deps: $(BUILD)/Boot.d

$(BUILD)/Boot.d: $(RLX)
	$(RLX) -i ./src/bootloader/EFIBoot.rlx -o $@ --makedep $(EFI_RLX_FLAGS)

LIGHT_CLEAN_FILES+= $(BUILD)/Boot.efi $(BUILD)/Boot.d

# Kernel

$(BUILD)/Kernel.elf: $(RLX)
$(BUILD)/Kernel.elf: $(BUILD)/Kernel.d
$(BUILD)/Kernel.elf: $(shell cat $(BUILD)/Kernel.d 2>/dev/null)
	$(DBG)$(RLX) -i ./src/kernel/Main.rlx -o $@ $(KERNEL_RLX_FLAGS)

secret-internal-deps: $(BUILD)/Kernel.d

$(BUILD)/Kernel.d: $(RLX)
	$(DBG)$(RLX) -i ./src/kernel/Main.rlx -o $@ --makedep $(KERNEL_RLX_FLAGS)

LIGHT_CLEAN_FILES+= $(BUILD)/Kernel.elf $(BUILD)/Kernel.d

# Userland

$(BUILD)/Beep.elf: $(RLX)
$(BUILD)/Beep.elf: $(BUILD)/Beep.d
$(BUILD)/Beep.elf: $(shell cat $(BUILD)/Beep.d 2>/dev/null)
	$(RLX) -i ./src/user/Beep.rlx -o $@ ${ELF_RLX_FLAGS}

secret-internal-deps: $(BUILD)/Beep.d

$(BUILD)/Beep.d: $(RLX)
	$(RLX) -i ./src/user/Beep.rlx -o $@ --makedep $(ELF_RLX_FLAGS)

LIGHT_CLEAN_FILES+= $(BUILD)/Beep.elf $(BUILD)/Beep.d

# Generated

./src/kernel/core/%.rlx: ./src/kernel/core/generated/%.py
	python3 $^

gen: kernel/core/*.rlx

# Helper targets

tools: $(BUILD)/GPTTool.elf $(BUILD)/FAT32Tool.elf $(BUILD)/Ext2Tool.elf

fast: $(BUILD)/Disk.img

clean:
	rm -f $(LIGHT_CLEAN_FILES)

clean-all: clean
	rm -f $(CLEAN_FILES)

depend dep deps:
	rm -f $(BUILD)/*.d
	$(MAKE) secret-internal-deps

all: Disk.qcow2

QEMU?=qemu-system-x86_64
QEMU_FLAGS=-machine q35 -bios OVMF.fd -hda Disk.qcow2 -serial stdio --cpu max,la57=off
DEBUG_FLAGS=

ifneq (,$(findstring --gdb,$(flags)))
	DEBUG_FLAGS=-s
endif

ifneq (,$(findstring --wait,$(flags)))
	DEBUG_FLAGS=-s -S
endif

ifneq (,$(findstring --no-reset,$(flags)))
	QEMU_FLAGS+=-no-reboot -no-shutdown
endif

ifneq (,$(findstring --achi-debug,$(flags)))
	QEMU_FLAGS+=--trace "ahci_*" --trace "handle_*" --trace "ide_*"
endif

ifneq (,$(findstring --net-debug,$(flags)))
	QEMU_FLAGS+=-device e1000e,netdev=hub0port0 -object filter-dump,id=f1,netdev=hub0port0,file=dump.pcap -netdev user,id=hub0port0,hostfwd=tcp::7777-:7777
endif

boot: Disk.qcow2
	$(QEMU) $(QEMU_FLAGS) $(DEBUG_FLAGS)

reset-compiler:
	cd compiler/; git reset --hard HEAD~1; cd ..

pull-compiler:
	cd compiler/; git pull upstream +master; cd ..