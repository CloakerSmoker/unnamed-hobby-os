# Config-ish

ROOT_EXTRA= TestFile.txt test.txt sponge.six dum.six

RLX_FLAGS?= --crlf --dwarf --debug --silent
EFI_RLX_FLAGS?= $(RLX_FLAGS) --pe-reloc --pe --efi --platform-dir src/efi
ELF_RLX_FLAGS?= $(RLX_FLAGS) --linux
KERNEL_RLX_FLAGS?= $(RLX_FLAGS) --standalone-elf --symbols --platform kernel --platform-dir src/kernel/lib
TRAMPOLINE_RLX_FLAGS?= $(RLX_FLAGS) --standalone-elf --platform trampoline --platform-dir src/trampoline/lib

RLX?=compiler/build/linux_compiler.elf

BUILD=build

# Files to clean

CLEAN_FILES=
LIGHT_CLEAN_FILES=

# Busybox

BUSYBOX=$(BUILD)/busybox
BUSYBOX_VER=1.35.0
BUSYBOX_SRC=$(BUSYBOX)/busybox-$(BUSYBOX_VER)

CLEAN_FILES= $(BUSYBOX_SRC)

$(BUSYBOX)/busybox-$(BUSYBOX_VER).tar.bz2:
	wget https://www.busybox.net/downloads/busybox-$(BUSYBOX_VER).tar.bz2 -P $(BUSYBOX)

$(BUSYBOX)/busybox-$(BUSYBOX_VER).tar: $(BUSYBOX)/busybox-$(BUSYBOX_VER).tar.bz2
	bunzip2 -k $(BUSYBOX)/busybox-$(BUSYBOX_VER).tar.bz2

$(BUSYBOX_SRC)/README: $(BUSYBOX)/busybox-$(BUSYBOX_VER).tar
	mkdir -p $(BUSYBOX_SRC)
	tar -mxf $(BUSYBOX)/busybox-$(BUSYBOX_VER).tar --directory=$(BUSYBOX)

$(BUSYBOX_SRC)/busybox.links: $(BUSYBOX_SRC)/README src/host/busybox.config
	cp src/host/busybox.config $(BUSYBOX_SRC)/.config
	cd $(BUSYBOX_SRC); make all
	cd $(BUSYBOX_SRC); make busybox.links
	cd $(BUSYBOX_SRC); ./make_single_applets.sh

# Doomgeneric

DOOM_GENERIC=$(BUILD)/doomgeneric
DOOM_GENERIC_SRC=$(DOOM_GENERIC)/doomgeneric

$(DOOM_GENERIC):
	-git clone https://github.com/ozkl/doomgeneric $@

$(DOOM_GENERIC_SRC)/Makefile.uhos: ./src/host/doomgeneric/Makefile.uhos
	cp ./src/host/doomgeneric/Makefile.uhos $@

$(DOOM_GENERIC_SRC)/doomgeneric_uhos.c: ./src/host/doomgeneric/doomgeneric_uhos.c
	cp ./src/host/doomgeneric/doomgeneric_uhos.c $@

$(BUILD)/doom.elf: $(DOOM_GENERIC)
$(BUILD)/doom.elf: $(DOOM_GENERIC_SRC)/Makefile.uhos
$(BUILD)/doom.elf: $(DOOM_GENERIC_SRC)/doomgeneric_uhos.c
#$(BUILD)/doomgeneric.elf: $(DOOM_GENERIC_SRC)/doomgeneric
	-rm -rf $(DOOM_GENERIC_SRC)/build
	-rm $(DOOM_GENERIC_SRC)/doomgeneric
	cd $(DOOM_GENERIC_SRC); make --makefile=Makefile.uhos
	cp $(DOOM_GENERIC_SRC)/doomgeneric $@

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

define MAKE_DISK_SCRIPT =
dd bs=1M count=128M if=/dev/zero of=/host/build/Disk.img
loop open /host/build/Disk.img

gpt /dev/loop0 format 128M

gpt /dev/loop0 set disk guid random

gpt /dev/loop0 create 0
gpt /dev/loop0 set partition 0 start 10M
gpt /dev/loop0 set partition 0 end 50M
gpt /dev/loop0 set partition 0 name "EFI System"
gpt /dev/loop0 set partition 0 type system
#gpt /dev/loop0 set partition 0 guid random
gpt /dev/loop0 set partition 0 guid {4D5DA455-8101-EDEF-B110-1E59234ABFDF}

gpt /dev/loop0 create 1
gpt /dev/loop0 set partition 1 start 51M
gpt /dev/loop0 set partition 1 end 127M
gpt /dev/loop0 set partition 1 name "Boot"
gpt /dev/loop0 set partition 1 type custom
#gpt /dev/loop0 set partition 1 guid random
gpt /dev/loop0 set partition 1 guid {E8AB429D-FF25-7E05-9F30-B0C209E4F834}

gpt /dev/loop0 show partitions

gpt /dev/loop0 scan

format fat32 /dev/loop0p0 40M
format ext2 /dev/loop0p1 70M

mount ext2 /dev/loop0p1 /root

install /host/build/Trampoline.elf /root/Trampoline.elf
install /host/build/Kernel.elf /root/Kernel.elf

install /host/build/doom.elf /root/doom/doom.elf
install /host/misc/files/DOOM.WAD /root/doom/DOOM.WAD

install /host/misc/files/sponge.six /root/usr/share/demo/sponge.six
install /host/misc/files/dum.six /root/usr/share/demo/dum.six

$(shell python3 src/host/busybox.py --install $(BUSYBOX_SRC) | tr '\n' '\1')

exit
endef

#loop open /host/build/Disk.img
#gpt /dev/loop0 scan
#mount ext2 /dev/loop0p1 /root
#install /host/cat /root/usr/bin/cat

export MAKE_DISK_SCRIPT

define MAKE_USB_SCRIPT =
dd bs=1M count=128M if=/host/build/Disk.img of=/host/build/USB.img

loop open /host/build/USB.img

gpt /dev/loop0 set partition 0 type {EBD0A0A2-B9E5-4433-87C0-68B6B72699C7}
gpt /dev/loop0 set partition 0 name "Main Data Partition"

exit
endef

export MAKE_USB_SCRIPT

define GET_GUIDS_SCRIPT =
loop open /host/build/Disk.img
@echo "--efi-system-guid "
@gpt /dev/loop0 show partition 0 guid

@echo "--root-guid "
@gpt /dev/loop0 show partition 1 guid
exit
endef

export GET_GUIDS_SCRIPT

$(BUILD)/Disk.img: $(BUILD)/GPTTool.elf
$(BUILD)/Disk.img: $(BUILD)/FAT32Tool.elf $(BUILD)/Ext2Tool.elf
$(BUILD)/Disk.img: $(BUILD)/Boot.efi $(BUILD)/Trampoline.elf $(BUILD)/Kernel.elf
$(BUILD)/Disk.img: $(BUILD)/HostFileShell.elf
$(BUILD)/Disk.img: $(BUILD)/doom.elf
$(BUILD)/Disk.img: $(BUSYBOX_SRC)/busybox.links $(shell python3 src/host/busybox.py --src $(BUSYBOX_SRC))
$(BUILD)/Disk.img:
	rm -f $@

	echo "$$MAKE_DISK_SCRIPT" | tr '\1' '\n' | $(BUILD)/HostFileShell.elf --script
	echo "$$GET_GUIDS_SCRIPT" | tr '\1' '\n' | $(BUILD)/HostFileShell.elf --silent | tr '\n' ' ' > $(BUILD)/KernelCommandLine.txt
	
	$(BUILD)/FAT32Tool.elf "File($@,512)>GPT(0)" \
		"disklabel HOS-BOOT" \
		"mkdir EFI" \
		"cd EFI" \
		"mkdir BOOT" \
		"cd BOOT" \
		"import $(BUILD)/Boot.efi BOOTX64.EFI" \
		"import $(BUILD)/KernelCommandLine.txt KCMDLN.TXT" \
		"quit"
	
	$(BUILD)/Ext2Tool.elf "File($@,512)>GPT(1)" \
		"mkdir dev" \
		"cd dev" \
		"mknod tty1 c 4 1" \
		"mknod ttyS0 c 4 64" \
		"hard-link console tty1" \
		"mknod pc-speaker c 10 129" \
		"quit"
	
	echo "$$MAKE_USB_SCRIPT" | tr '\1' '\n' | $(BUILD)/HostFileShell.elf --script

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

# HostFileShell

$(BUILD)/HostFileShell.elf: $(RLX)
$(BUILD)/HostFileShell.elf: $(BUILD)/HostFileShell.d
$(BUILD)/HostFileShell.elf: $(shell cat $(BUILD)/HostFileShell.d 2>/dev/null)
	$(RLX) -i ./src/host/HostFileShell.rlx -o $@ ${ELF_RLX_FLAGS}

secret-internal-deps: $(BUILD)/HostFileShell.d

$(BUILD)/HostFileShell.d: $(RLX)
	$(RLX) -i ./src/host/HostFileShell.rlx -o $@ --makedep $(ELF_RLX_FLAGS)

CLEAN_FILES+= $(BUILD)/HostFileShell.elf $(BUILD)/HostFileShell.d

# Bootloader

$(BUILD)/Boot.efi: $(RLX)
$(BUILD)/Boot.efi: $(BUILD)/Boot.d
$(BUILD)/Boot.efi: $(shell cat $(BUILD)/Boot.d 2>/dev/null)
	$(RLX) -i ./src/bootloader/EFIBoot.rlx -o $@ $(EFI_RLX_FLAGS)

secret-internal-deps: $(BUILD)/Boot.d

$(BUILD)/Boot.d: $(RLX)
	$(RLX) -i ./src/bootloader/EFIBoot.rlx -o $@ --makedep $(EFI_RLX_FLAGS)

LIGHT_CLEAN_FILES+= $(BUILD)/Boot.efi $(BUILD)/Boot.d

# Trampoline

$(BUILD)/Trampoline.elf: $(RLX)
$(BUILD)/Trampoline.elf: $(BUILD)/Trampoline.d
$(BUILD)/Trampoline.elf: $(shell cat $(BUILD)/Trampoline.d 2>/dev/null)
	$(RLX) -i ./src/trampoline/Main.rlx -o $@ $(TRAMPOLINE_RLX_FLAGS)

secret-internal-deps: $(BUILD)/Trampoline.d

$(BUILD)/Trampoline.d: $(RLX)
	$(RLX) -i ./src/trampoline/Main.rlx -o $@ --makedep $(TRAMPOLINE_RLX_FLAGS)

LIGHT_CLEAN_FILES+= $(BUILD)/Trampoline.elf $(BUILD)/Trampoline.d

# PCI ID database

$(BUILD)/pci.ids:
	cd $(BUILD); wget https://pci-ids.ucw.cz/v2.2/pci.ids

$(BUILD)/pciids.bin: $(BUILD)/pci.ids
$(BUILD)/pciids.bin: ./src/drivers/PCI/BinarizeDatabase.py
	python3 ./src/drivers/PCI/BinarizeDatabase.py $(BUILD)/pci.ids $(BUILD)/pciids.bin

LIGHT_CLEAN_FILES+= $(BUILD)/pciids.bin

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

./src/kernel/core/generated/%.rlx: ./src/kernel/core/generated/%.py
	python3 $^

gen: ./src/kernel/core/generated/*.rlx

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

STDIO=serial
QEMU?=qemu-system-x86_64
QEMU_FLAGS=-machine q35 -bios misc/files/OVMF.fd $(DISK_FLAGS) -$(STDIO) stdio --cpu max,la57=off -global hpet.msi=true -m 1G
DISK_FLAGS=-hda Disk.qcow2
DEBUG_FLAGS=
HELP_TEXT=Help:|

HELP_TEXT+=gdb: Request QEMU GDB stub|
ifneq (,$(findstring gdb,$(flags)))
	DEBUG_FLAGS=-s
endif

HELP_TEXT+=monitor: Set STDIO to QEMU monitor instead of serial|
ifneq (,$(findstring monitor,$(flags)))
	STDIO=monitor
endif

HELP_TEXT+=wait: Have QEMU wait for a connection over the GDB stub before starting|
ifneq (,$(findstring wait,$(flags)))
	DEBUG_FLAGS=-s -S
endif

HELP_TEXT+=no-reset: Prevent QEMU from rebooting/shutting down on triple fault. Instead, trap into debugger|
ifneq (,$(findstring no-reset,$(flags)))
	QEMU_FLAGS+=-no-reboot -no-shutdown
endif

HELP_TEXT+=achi-debug: Have QEMU log debug messages related to AHCI|
ifneq (,$(findstring achi-debug,$(flags)))
	QEMU_FLAGS+=--trace "ahci_*" --trace "handle_*" --trace "ide_*"
endif

HELP_TEXT+=net-user: Add a network device backed by QEMU's user network stack|
ifneq (,$(findstring net-user,$(flags)))
	COMMA:=,
	QEMU_FLAGS+=-device e1000e,netdev=hub0port0 -netdev user,id=hub0port0$(if $(HOSTFWD),$(COMMA)hostfwd=$(HOSTFWD),)
endif

HELP_TEXT+=net-tap: Add a network device backed by a TAP device (vm0)|
ifneq (,$(findstring net-tap,$(flags)))
	QEMU:=sudo $(QEMU)
	QEMU_FLAGS+=-device e1000e,netdev=hub0port0 -netdev tap,ifname=vm0,id=hub0port0
endif

HELP_TEXT+=net-capture: Capture network traffic from the other net- options into the dump.pcap file|
ifneq (,$(findstring net-capture,$(flags)))
	QEMU_FLAGS+=-object filter-dump,id=f1,netdev=hub0port0,file=dump.pcap
endif

HELP_TEXT+=usb: Add an EHCI controller, and use a USB device as the boot device instead of an AHCI device|
ifneq (,$(findstring usb,$(flags)))
	DISK_FLAGS:=-drive if=none,id=stick,format=raw,file=build/USB.img -usb -device usb-ehci,id=ehci -device usb-storage,bus=ehci.0,drive=stick
endif

HELP_TEXT+=-dint: dint
ifneq (,$(findstring dint,$(flags)))
	QEMU_FLAGS+=-d int
endif

HELP_TEXT+=usb-debug: Have QEMU dump debug messages related to EHCI|
ifneq (,$(findstring usb-debug,$(flags)))
	QEMU_FLAGS+=--trace "usb_ehci*"
endif

HELP_TEXT+=dry: Dry-run, don't actually run QEMU, just print the flags that would be passed|
ifneq (,$(findstring dry,$(flags)))
	QEMU:=echo $(QEMU)
endif

HELP_TEXT+=host-gdb: Debug QEMU itself using GDB|
ifneq (,$(findstring host-gdb,$(flags)))
	QEMU:=gdb -q --args $(QEMU)
endif

export HELP_TEXT

boot-help:
	@echo "$$HELP_TEXT" | tr '|' '\n'

boot: Disk.qcow2
	$(QEMU) $(QEMU_FLAGS) $(DEBUG_FLAGS)

create-tap:
	sudo ip tuntap add vm0 mode tap
	sudo ip link set vm0 master vmbr0
	sudo ip link set vm0 up

delete-tap:
	sudo ip tuntap del vm0 mode tap

reset-compiler:
	cd compiler/; git reset --hard HEAD~1; cd ..

pull-compiler:
	cd compiler/; git pull upstream +master; cd ..