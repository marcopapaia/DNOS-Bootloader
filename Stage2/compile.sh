nasm -f elf "io.asm" -o "io.o"
nasm -f elf "isr.asm" -o "isr.o"
nasm -f elf "util.asm" -o "util.o"

/home/daniel/opt/cross/bin/i686-elf-gcc -c main.c -o main.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c display.c -o display.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c pmm.c -o pmm.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c hdd.c -o hdd.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c partitions.c -o partitions.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c idt.c -o idt.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c pic.c -o pic.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c pit.c -o pit.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c ext2.c -o ext2.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c elf.c -o elf.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra
/home/daniel/opt/cross/bin/i686-elf-gcc -c paging.c -o paging.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra

/home/daniel/opt/cross/bin/i686-elf-gcc -T linker.ld -n -o pmode.elf -ffreestanding -O0 -nostdlib main.o display.o io.o pmm.o hdd.o partitions.o isr.o idt.o pic.o pit.o ext2.o util.o elf.o paging.o -lgcc

objcopy -j .text -O binary pmode.elf pmode_text.bin
objcopy -j .rodata -O binary pmode.elf pmode_rodata.bin
objcopy -j .data -O binary pmode.elf pmode_data.bin

dd if=entry/entry.bin of=stage2.bin 
dd if=pmode_text.bin >> stage2.bin
dd if=pmode_rodata.bin >> stage2.bin
dd if=pmode_data.bin >> stage2.bin
truncate stage2.bin --size=20k 

rm *.o
