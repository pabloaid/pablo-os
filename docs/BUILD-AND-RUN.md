# Building and running Pablo OS

This page walks through building the OS, running it in QEMU, and booting it on a real PC from a USB stick.

## Before you start

Install the tools once (Ubuntu):

```sh
sudo apt install build-essential git xorriso qemu-system-x86 ovmf gdb
```

## Building and running in QEMU

Run these from the project folder (the one with `GNUmakefile` in it). `make` reads `GNUmakefile`, which lists each file to build, what it is built from, and the command that builds it.

| Command | What it does |
| --- | --- |
| `make` | Builds everything and produces `pablo-os.iso`. It compiles each `src/*.c` file to an object file in `build/`, links them into `build/kernel` using `linker.ld`, then packs the kernel, `limine.conf` and Limine's boot files into the ISO. On the first run it also clones Limine's prebuilt binaries into `limine/`. |
| `make run` | Builds if needed, then boots the ISO in QEMU with legacy BIOS firmware. Text from the serial port shows up in your terminal. Close the QEMU window or press Ctrl+C in the terminal to stop it. |
| `make run-uefi` | The same, but QEMU starts with UEFI firmware (OVMF), which is how modern PCs boot. |
| `make debug` | Boots QEMU paused, waiting for a debugger on port 1234. In a second terminal, run `gdb build/kernel -ex "target remote :1234"`, then for example `break kmain` and `continue` to step through the kernel. |
| `make clean` | Deletes `build/` and `pablo-os.iso`, so the next `make` rebuilds from scratch. |
| `make distclean` | Same as `clean`, and also deletes the cloned `limine/` folder. |

`make` only rebuilds what changed since the last build, by comparing file timestamps. If nothing changed, it prints `make: Nothing to be done for 'all'`, which means the ISO is already up to date. You never need to run `make clean` after quitting QEMU.

## Booting a real PC from a USB stick

> ⚠️ This erases everything on the USB stick. Writing to the wrong device wipes that disk instead, so check the name with `lsblk` every time before running `dd`.

Tested on a Lenovo laptop that dual-boots Windows and Ubuntu with GRUB. The kernel only reads the screen and serial port and has no disk drivers, so it can't touch your installed systems.

**1. Build the ISO.**

```sh
make
```

**2. Find the USB stick's device name.** Plug it in, then run:

```sh
lsblk -o NAME,SIZE,TRAN,LABEL,MOUNTPOINTS
```

Linux names whole disks `sda`, `sdb`, `nvme0n1` and so on, and partitions on them `sda1`, `sda2`, `nvme0n1p1`. Look for the disk with `usb` in the `TRAN` (transport) column and the size of your stick, and use the **whole disk** name, such as `sda`, not a partition like `sda1`. The `nvme0n1` disk is the laptop's internal SSD with Windows and Ubuntu on it. Never write to it.

**3. Write the ISO to the stick.** Replace `sdX` with the name you found:

```sh
sudo umount /dev/sdX1        # unmount it if Ubuntu auto-mounted it (an error here is fine)
sudo dd if=pablo-os.iso of=/dev/sdX bs=4M status=progress oflag=sync
sync
```

`dd` copies raw bytes from one file to another, with no notion of files or folders. Each part of the command:

| Part | Meaning |
| --- | --- |
| `sudo` | Run as administrator, because ordinary users can't write directly to a disk. |
| `if=pablo-os.iso` | Input file: read from the ISO. |
| `of=/dev/sdX` | Output file: write to the USB disk. On Linux, every disk appears as a special file in `/dev`, and writing to that file writes straight onto the disk, starting at its first byte. |
| `bs=4M` | Block size: copy 4 MB at a time instead of the default 512 bytes, which is much faster. |
| `status=progress` | Print how many bytes have been copied while it runs. |
| `oflag=sync` | Wait for each block to really reach the stick before writing the next, so when `dd` finishes the data is on the stick and not still sitting in RAM. |

The final `sync` flushes anything still cached, so it is safe to unplug the stick afterward.

The stick ends up as an exact byte-for-byte copy of the ISO. Its first sector holds the BIOS boot code that `limine bios-install` added, and it also contains a small FAT partition with `EFI/BOOT/BOOTX64.EFI`, which is what UEFI firmware looks for. The old files and the `UBUNTU 22_0` label are gone.

**4. Boot from it.**

- **Turn off Secure Boot.** UEFI firmware only runs bootloaders signed by keys it trusts, and Limine isn't signed. Open the firmware setup (F2 or Del while powering on, or run `systemctl reboot --firmware-setup` from Ubuntu) and disable Secure Boot. If Windows uses BitLocker, it may ask for its recovery key on the next Windows boot; find the key at [aka.ms/myrecoverykey](https://aka.ms/myrecoverykey) before changing anything.
- **Open the one-time boot menu.** On a Lenovo, press F12 repeatedly right after powering on (other brands use F11, F8 or Esc). GRUB's menu won't list the stick, because GRUB only knows about Windows and Ubuntu; this menu belongs to the firmware and comes before GRUB.
- **Pick the stick**, shown as something like "EFI USB Device". Limine shows its menu for 3 seconds, then Pablo OS starts.
- **To get back**, hold the power button to turn off, unplug the stick, and power on. GRUB comes up as usual. You can turn Secure Boot back on afterward.

To reuse the stick for normal files, reformat it with Ubuntu's Disks app. On Windows, [Rufus](https://rufus.ie) can write the ISO in "DD image" mode instead of using `dd`.
