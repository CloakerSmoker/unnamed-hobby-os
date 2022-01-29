rm EFIBoot.img

nrlx -i ./src/bootloader/EFIBoot.rlx -o ./build/EFIBoot.efi --pe --crlf --pe-subsystem 10; or exit

set Options "--elf" "--crlf"

nrlx -i ./src/kernel/Main.rlx      -o ./build/kernel.elf      $Options --debug --dwarf; or exit
nrlx -i ./src/user/TestProgram.rlx -o ./build/TestProgram.elf $Options; or exit
nrlx -i ./src/user/Write.rlx       -o ./build/Write.elf       $Options --debug --dwarf; or exit
-i ./src/kernel/Main.rlx -o ./build/kernel.elf --crlf --elf --debug --dwarf
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
import ./build/TestProgram.elf test.elf
import ./build/Write.elf write
import TestFile.txt test.txt
quit
" | ./Ext2Tool.elf 'File(EFIBoot.img,512)>GPT(1)'

echo "Done!"