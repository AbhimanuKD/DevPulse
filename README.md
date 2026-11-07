# DevPulse OS V-1

A small 32-bit operating system, written from scratch in C and assembly.
It boots on a PC (BIOS) and runs in **128 MB of RAM**. It uses about 9 KB of that.

## What V-1 can do

- Boots from its own boot sector. No Linux, no Windows underneath.
- Text screen (80x25) with a blue "blueprint" look
- Keyboard input (PS/2) and serial input
- A shell with these commands:

| Command | What it does |
|---|---|
| `help` | List all commands |
| `about` | About this OS |
| `mem` | Show RAM and kernel size, and check the 128 MB target |
| `cpu` | Show CPU vendor |
| `time` / `uptime` | Clock and time since boot |
| `calc 12 * 3` | Calculator (`+ - * / %`) |
| `echo hello` | Print text |
| `color 14` | Change text color (0-15) |
| `write notes hi` | Save a small text file (kept in RAM) |
| `ls`, `cat notes`, `rm notes` | List, read, delete files |
| `clear`, `reboot`, `halt` | Clear screen, restart, stop |

## What V-1 cannot do (be honest about this)

- It cannot run Windows or Linux programs. It only runs its own commands.
- Files are in RAM only. They are lost when you reboot.
- No mouse, no graphics, no network, no hard disk files, no multitasking.
- Only one program (the shell) runs. There is no way to install apps yet.

## Run it (easiest way: QEMU)

1. Install QEMU: https://www.qemu.org/download/
2. Open a terminal in this folder.
3. Run:
   ```
   qemu-system-i386 -m 128 -drive format=raw,file=devpulse-os-v1.img
   ```
   `-m 128` gives the virtual PC 128 MB of RAM.
4. Click inside the window and type `help`.

## Run it from the ISO (VirtualBox, VMware, or QEMU)

1. Make a new VM: type **Other**, **128 MB** RAM. No hard disk is needed.
2. Insert `devpulse-os-v1.iso` as the CD/DVD.
3. Use **legacy BIOS** (turn EFI off). Start the VM.

QEMU: `qemu-system-i386 -m 128 -cdrom devpulse-os-v1.iso -boot d`

## Run it in VirtualBox with the .img instead

1. Convert the image:
   ```
   VBoxManage convertfromraw devpulse-os-v1.img devpulse-os-v1.vdi --format VDI
   ```
2. Make a new VM: type **Other**, version **Other/Unknown**, **128 MB** RAM.
3. Use the `.vdi` as the hard disk. Leave EFI **off** (this OS needs legacy BIOS).
4. Start the VM.

## Build it yourself

Needs Linux (or WSL on Windows) with `gcc`, `nasm`, `ld`, `make`:

```
sudo apt install build-essential nasm qemu-system-x86
make          # builds devpulse-os-v1.img
make iso      # builds devpulse-os-v1.iso
make run      # starts it in QEMU with 128 MB
```

## Do not write it to a real USB drive or disk unless you are sure

- Writing an image with `dd` or a flashing tool erases that whole drive.
- V-1 is only tested in QEMU. It has not been tested on real hardware.
- Many new PCs use UEFI only. V-1 needs legacy BIOS boot, so it may not start.

## Files

```
DevPulse-OS/
├── devpulse-os-v1.iso   # bootable CD image (VirtualBox, VMware, QEMU)
├── devpulse-os-v1.img   # bootable disk image
├── Makefile             # build commands
├── README.md
└── src/
    ├── boot.asm         # boot sector: checks RAM, loads kernel, enters 32-bit mode
    ├── entry.asm        # kernel start point
    ├── kernel.c         # screen, keyboard, shell, commands, RAM files
    └── linker.ld        # places the kernel at memory address 0x10000
```

## Plan for V-2

- Timer and interrupts (so the CPU can sleep instead of waiting)
- Files that survive reboot (a simple disk file system)
- Memory allocator
- Running small programs

## Author

Abhimanu Kumar - https://github.com/AbhimanuKD
