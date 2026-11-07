<p align="center"><img src="assets/banner.svg" alt="DevPulse banner" width="100%"></p>

<h1 align="center">DevPulse</h1>
<p align="center"><b>Build AI systems. Ship faster.</b><br>Portfolio site + <b>DevPulse OS V-1</b>, a small operating system you can download.</p>

<p align="center">
  <a href="https://abhimanukd.github.io/DevPulse/"><b>Live site</b></a> ·
  <a href="downloads/devpulse-os-v1.iso">Download ISO</a> ·
  <a href="os/">OS source code</a> ·
  <a href="https://github.com/AbhimanuKD/">GitHub profile</a>
</p>

## DevPulse OS V-1

A 32-bit operating system written from scratch in C and assembly. It boots on a PC with legacy BIOS and runs in **128 MB of RAM**. The kernel is about 9 KB.

| Boot and RAM check | Help command | Calculator and files |
|---|---|---|
| ![Boot](assets/os-boot.png) | ![Help](assets/os-help.png) | ![Session](assets/os-session.png) |

**Download**
- [`downloads/devpulse-os-v1.iso`](downloads/devpulse-os-v1.iso): for VirtualBox, VMware, QEMU
- [`downloads/devpulse-os-v1.img`](downloads/devpulse-os-v1.img): raw disk image

**Run with QEMU**
```
qemu-system-i386 -m 128 -cdrom devpulse-os-v1.iso -boot d
```
Full steps, limits and build instructions are in [`os/README.md`](os/README.md).

**Honest limits:** it cannot run Windows or Linux programs, files are lost on reboot, and it is tested only in QEMU, not on real hardware.

## The website

- One file: `index.html` (screenshots are built in, no extra image files needed)
- Sections: hero, AI workspace (prompt builder and console), projects, DevPulse OS (screenshots, downloads, shell simulator), profile
- No build step. Works on phones and desktops.

## Files

```
DevPulse/
├── index.html
├── README.md
├── assets/        # banner, favicon, OS screenshots
├── downloads/     # devpulse-os-v1.iso and .img
└── os/            # OS source code, Makefile, README
```

## Author

Abhimanu Kumar · https://github.com/AbhimanuKD
