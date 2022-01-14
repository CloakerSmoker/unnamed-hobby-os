# unnamed project

Expects to be cloned with the structure
* any_name/
    * src
        * \<this-repo\>
    * `Ext2Tool.elf` (compile `./src/host/Ext2Tool.rlx`, used to create the ext2 disk image)
* `nasm` (somewhere on path)

Build with `source ./src/build.fish`, which will:

* assemble the 1st stage bootloader
* compile the 2nd stage bootloader
* compile the kernel,
* compile user mode test programs

and then

* create a 2mb ext2 disk image (`disk.img`)
* import the 1st stage bootloader to the boot sector
* import the 2nd stage bootloader to inode 5 (static index referenced by the 1st stage)
* import the kernel executable to `kernel.elf`
* import test files

which will then get `disk.img` (hopefully) ready to boot.

## Configuration

(Spoiler: there isn't much)

Most options live in `./src/kernel/Main.rlx` as global variables (but end up getting evaulated at compile time, don't worry). 

The big one is `USE_BOCHS_PORT_HACK` which needs to be disabled when compiling for anything but Bochs. Disclaimer: I haven't ran this on anything but Bochs.

## Other

The 1st stage bootloader doesn't do much, it just sets up a barebones 64 bit mode environment (first 4mb identity mapped), and then reads the disk to figure out what the first block of inode 5 is (the 2nd stage). Then, it reads the first 13 blocks of the 2nd stage bootloader into memory and starts running it.

Hopefully the 2nd stage never gets bigger than... (13 * 1024) bytes. To keep it small, it uses a trimmed down read-only version of the ext2 library, and does minimal error checking and only prints via the Bochs port.

The 2nd stage pretty much just finds `kernel.elf`, and then maps its sections into memory and jumps to the entrypoint.