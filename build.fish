nasm ./src/bootloader/Boot1.asm -o boot1.bin; or exit

set Options "--elf" "--crlf"

./new_compiler.elf -i ./src/bootloader/Boot2.rlx -o boot2.bin --bin --crlf; or exit
./new_compiler.elf -i ./src/kernel/Main.rlx -o kernel.elf $Options --debug --dwarf; or exit
./new_compiler.elf -i ./src/user/TestProgram.rlx -o TestProgram.elf $Options; or exit
rlx -i ./src/user/Write.rlx -o Write.elf $Options --debug; or exit
rm disk.img
echo "
format
size 2 m
done
import-to-boot-sector boot1.bin
import-to-node boot2.bin 5
link-to-node boot2.bin 5
import kernel.elf kernel.elf
import TestFile.txt test.txt
import TestProgram.elf test.elf
import Write.elf write
quit
" | ./Ext2Tool.elf disk.img
