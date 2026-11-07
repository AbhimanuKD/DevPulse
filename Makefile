# DevPulse OS V-1 - build with:  make      run with:  make run
CFLAGS = -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -fno-builtin -nostdlib -O2 -Wall -Wno-array-bounds -fno-delete-null-pointer-checks
IMG    = devpulse-os-v1.img

all: $(IMG)

build/entry.o: src/entry.asm
	@mkdir -p build
	nasm -f elf32 $< -o $@

build/kernel.o: src/kernel.c
	@mkdir -p build
	gcc $(CFLAGS) -c $< -o $@

build/kernel.bin: build/entry.o build/kernel.o src/linker.ld
	ld -m elf_i386 -T src/linker.ld --oformat binary -o $@ build/entry.o build/kernel.o

$(IMG): build/kernel.bin src/boot.asm
	nasm -f bin -dKSECT=$$(( ($$(stat -c %s build/kernel.bin) + 511) / 512 )) src/boot.asm -o build/boot.bin
	cat build/boot.bin build/kernel.bin > $@
	truncate -s 1M $@
	@echo "Built $@ ($$(stat -c %s build/kernel.bin) bytes of kernel)"

run: $(IMG)
	qemu-system-i386 -m 128 -drive format=raw,file=$(IMG)

run-terminal: $(IMG)
	qemu-system-i386 -m 128 -drive format=raw,file=$(IMG) -nographic

clean:
	rm -rf build $(IMG)
