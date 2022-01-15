nasm ./src/bootloader/Boot1.asm -o ./build/boot1.bin; or exit

set Options "--elf" "--crlf"

rlx -i ./src/bootloader/Boot2.rlx -o ./build/boot2.bin       --bin --crlf; or exit
rlx -i ./src/kernel/Main.rlx      -o ./build/kernel.elf      $Options --debug --dwarf; or exit
rlx -i ./src/user/TestProgram.rlx -o ./build/TestProgram.elf $Options; or exit
rlx -i ./src/user/Write.rlx       -o ./build/Write.elf       $Options --debug --dwarf; or exit
rm ./build/disk.img
echo "
format
size 2 m
done
import-to-boot-sector ./build/boot1.bin
import-to-node ./build/boot2.bin 5
link-to-node boot2.bin 5
import ./build/kernel.elf kernel.elf
import ./build/TestProgram.elf test.elf
import ./build/Write.elf write
import TestFile.txt test.txt
quit
" | ./Ext2Tool.elf ./build/disk.img
