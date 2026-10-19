# Pablo OS build
#
#   make            build the bootable ISO (pablo-os.iso)
#   make run        boot it in QEMU (legacy BIOS)
#   make run-uefi   boot it in QEMU with UEFI firmware (needs the ovmf package)
#   make debug      boot paused, waiting for gdb on port 1234
#   make clean      delete build output

.SUFFIXES:

ISO := pablo-os.iso
KERNEL := build/kernel

# Pinned Limine release (prebuilt binaries are on the -binary tags).
LIMINE_TAG := v11.4.1-binary

CC := gcc
LD := ld

# Freestanding: no C library, no OS underneath us.
CFLAGS := -g -O2 -pipe -std=gnu11 -Wall -Wextra \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-PIC \
    -fno-lto \
    -m64 -march=x86-64 \
    -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel \
    -Iinclude -MMD -MP

LDFLAGS := -m elf_x86_64 -nostdlib -static \
    -z max-page-size=0x1000 -z noexecstack \
    --build-id=none \
    -T linker.ld

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=build/%.o)

QEMU := qemu-system-x86_64
QEMUFLAGS := -M q35 -m 256M -serial stdio -no-reboot -no-shutdown
OVMF := /usr/share/ovmf/OVMF.fd

.PHONY: all run run-uefi debug clean distclean

all: $(ISO)

# --- Kernel ---------------------------------------------------------------

build/%.o: src/%.c GNUmakefile
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

-include $(OBJS:.o=.d)

# --- Limine -----------------------------------------------------------------

limine/limine:
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git \
	    --branch=$(LIMINE_TAG) --depth=1 limine
	$(MAKE) -C limine

# --- Bootable ISO -----------------------------------------------------------
# The result is a "hybrid" image: it boots as a CD in QEMU, and can also be
# written byte-for-byte to a USB stick to boot real hardware (BIOS or UEFI).

$(ISO): $(KERNEL) limine.conf limine/limine
	rm -rf build/iso_root
	mkdir -p build/iso_root/boot/limine build/iso_root/EFI/BOOT
	cp $(KERNEL) build/iso_root/boot/kernel
	cp limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin \
	    limine/limine-uefi-cd.bin build/iso_root/boot/limine/
	cp limine/BOOTX64.EFI limine/BOOTIA32.EFI build/iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
	    -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	    -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
	    -efi-boot-part --efi-boot-image --protective-msdos-label \
	    build/iso_root -o $@
	./limine/limine bios-install $@

# --- Running ------------------------------------------------------------------

run: $(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO)

run-uefi: $(ISO)
	$(QEMU) $(QEMUFLAGS) -bios $(OVMF) -cdrom $(ISO)

debug: $(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO) -s -S

clean:
	rm -rf build $(ISO)

distclean: clean
	rm -rf limine
