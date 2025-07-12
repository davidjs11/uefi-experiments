#   Makefile   #

# CHANGE THIS TO YOUR OWN SETTINGS !
FIRMWARE_BIN	:= ovmf/ovmfx64.bin
GNU_EFI_DIR		:= gnu-efi
DEF_TARGET		:= img/menu.img		# default target, changeable

SRC				:= src
TMP				:= tmp
IMG       		:= img
INCLUDE 		:= include

SOURCES			:= $(wildcard $(SRC)/*.c)
OBJECTS			:= $(patsubst $(SRC)/%.c,$(TMP)/%.o,$(SOURCES))
EFI_IMAGES		:= $(patsubst $(SRC)/%.c,$(TMP)/%.efi,$(SOURCES))
BOOT_IMAGES		:= $(patsubst $(SRC)/%.c,$(IMG)/%.img,$(SOURCES))

# virtual machine options
VM				:= qemu-system-x86_64
MEMORY			:= 256M

# compiler options
CC				:= gcc
CFLAGS			:= -I$(GNU_EFI_DIR)/inc -I$(INCLUDE)
CFLAGS			+= -fpic -ffreestanding -fno-stack-protector -fno-stack-check \
				   -fshort-wchar -mno-red-zone -maccumulate-outgoing-args

# linker options
LD				:= ld
LDFLAGS			:= -L$(GNU_EFI_DIR)/x86_64/lib -L$(GNU_EFI_DIR)/x86_64/gnuefi \
				   -T$(GNU_EFI_DIR)/gnuefi/elf_x86_64_efi.lds
LDFLAGS			+= -shared -Bsymbolic

# rules
all: $(BOOT_IMAGES)

# source to object
$(TMP)/%.o: $(SRC)/%.c
	@echo " - $*: compiling..."
	@mkdir -p $(TMP)
	@$(CC) $(CFLAGS) -c $< -o $@

# object to ELF
$(TMP)/%.so: $(TMP)/%.o
	@echo " - $*: linking..."
	@mkdir -p $(TMP)
	@$(LD) $(LDFLAGS) $(GNU_EFI_DIR)/x86_64/gnuefi/crt0-efi-x86_64.o \
		$< -o $@ -lgnuefi -lefi

# ELF to PE32+
$(TMP)/%.efi: $(TMP)/%.so
	@echo " - $*: creating PE32+ executable..."
	@mkdir -p $(TMP)
	@objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym \
		-j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
		--target=efi-app-x86_64 --subsystem=10 $< $@

# PE32+ to EFI System Partition image
$(IMG)/%.img: $(TMP)/%.efi
	@echo " - $*: creating bootable image..."
	@mkdir -p $(IMG)
	@dd if=/dev/zero of=$@ bs=1k count=1440
	@mformat -i $@ -f 1440 ::
	@mmd -i $@ ::/EFI
	@mmd -i $@ ::/EFI/BOOT
	@mcopy -i $@ $< ::/EFI/BOOT/BOOTX64.EFI
	@mcopy -i $@ DATA.TXT ::/

.PHONY: run
run: $(if $(TARGET),$(TARGET),$(DEF_TARGET))
	@echo "running $(if $(TARGET),$(TARGET),$(DEF_TARGET))..."
	@$(VM) \
		-m $(MEMORY) \
		-drive if=pflash,format=raw,readonly=on,file=$(FIRMWARE_BIN) \
		-drive format=raw,file=$(if $(TARGET),$(TARGET),$(DEF_TARGET)) \
		-net none

.PHONY: clean
clean:
	@rm -rf $(TMP) $(IMG)
