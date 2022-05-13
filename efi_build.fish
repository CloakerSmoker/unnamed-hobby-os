rm EFIBoot.img
rm EFIBoot.qcow2
rm src/EFIBoot.qcow2

nrlx -i ./src/bootloader/EFIBoot.rlx -o ./build/EFIBoot.efi --pe --crlf --pe-subsystem 10; or exit

set Options "--elf" "--crlf" "--debug" "--dwarf"

nrlx -i ./src/kernel/Main.rlx      -o ./build/kernel.elf      $Options; or exit
nrlx -i ./src/user/TestProgram.rlx -o ./build/TestProgram.elf $Options; or exit
nrlx -i ./src/user/Write.rlx       -o ./build/Write.elf       $Options; or exit

echo "
format 120 m
create
name \"EFI System\"
type system
start 0x32
end 70 m
done
create
name \"Boot\"
type custom
start 71 m
end 119 m
done
quit
" | ./GPTTool.elf 'File(EFIBoot.img,512)'

echo "
format 64 m
mkdir EFI
cd EFI
mkdir BOOT
cd BOOT
import ./build/EFIBoot.efi BOOTX64.EFI
quit
" | ./FAT32Tool.elf 'File(EFIBoot.img,512)>GPT(0)'

echo "
format 32 m
import ./build/kernel.elf kernel.elf
import TestFile.txt test.txt
import Test.rlx test.rlx
import dum.six dum.six
import graphic_design_is_my_passion.six logo.six
mkdir dev
cd dev
mknod console c 4 1
cd ..
mkdir bin
cd bin
import ./build/TestProgram.elf test.elf
import ./build/Write.elf write
import busybox_CLEAR linux_clear
import busybox_CAT linux_cat
import busybox_ED linux_ed
import busybox_VI linux_vi
import new_compiler.elf compiler
import Signaler.elf signaler.elf
quit
" | ./Ext2Tool.elf 'File(EFIBoot.img,512)>GPT(1)'

qemu-img convert -f raw -O qcow2 EFIBoot.img EFIBoot.qcow2
cp EFIBoot.qcow2 src/

echo "Done!"