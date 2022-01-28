rm EFIBoot.img

nrlx -i ./src/bootloader/EFIBoot.rlx -o ./build/EFIBoot.efi --pe --crlf --pe-subsystem 10; or exit

echo "
format 80 m
create
name \"EFI System\"
type system
start 0x32
end 70 m
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

mv EFIBoot.img ./build/EFIBoot.img

echo "Done!"