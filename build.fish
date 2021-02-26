./nasm.exe ./src/Boot.asm -o boot1.bin
./new_compiler.exe -i ./src/BootLoader.rlx -o boot2.bin --bin
./new_compiler.exe -i ./src/Kernel.rlx -o kernel.bin --bin
rm disk.img
echo "
format
size 2 m
done
import-to-boot-sector boot1.bin
import-to-node boot2.bin 5
import-to-node kernel.bin 6
link-to-node boot2.bin 5
link-to-node kernel.bin 6
import TestFile.txt Test.txt
quit
" | ./Ext2Tool.exe disk.img