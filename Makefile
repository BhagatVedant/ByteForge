CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm1176jzf-s -ffreestanding -nostdlib -nostartfiles -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

BUILD_DIR = build

SRC = boot/boot.S \
      kernel/kernel.c \
      kernel/mailbox.c \
      kernel/framebuffer.c

OBJ = $(BUILD_DIR)/boot.o \
      $(BUILD_DIR)/kernel.o \
      $(BUILD_DIR)/mailbox.o \
      $(BUILD_DIR)/framebuffer.o

ELF = $(BUILD_DIR)/kernel.elf
IMG = $(BUILD_DIR)/kernel.img

all: $(IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot/boot.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mailbox.o: kernel/mailbox.c kernel/mailbox.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: kernel/framebuffer.c kernel/framebuffer.h kernel/mailbox.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $(ELF)

$(IMG): $(ELF)
	arm-none-eabi-objcopy $(ELF) -O binary $(IMG)

clean:
	rm -rf $(BUILD_DIR)