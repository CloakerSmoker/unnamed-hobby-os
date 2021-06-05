./nasm.exe ./src/bootloader/Boot1.asm -o boot1.bin
./new_compiler.exe -i ./src/bootloader/Boot2.rlx -o boot2.bin --bin --crlf
./new_compiler.exe -i ./src/kernel/Main.rlx -o kernel.bin --elf --crlf --debug --dwarf
rm disk.img
echo "
format
size 2 m
done
import-to-boot-sector boot1.bin
import-to-node boot2.bin 5
link-to-node boot2.bin 5
import kernel.bin kernel.bin
import TestFile.txt test.txt
quit
" | ./Ext2Tool.exe disk.img
