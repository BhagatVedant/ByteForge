CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm1176jzf-s -ffreestanding -nostdlib -nostartfiles -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

BUILD_DIR = build

OBJ = $(BUILD_DIR)/boot.o \
      $(BUILD_DIR)/kernel.o \
      $(BUILD_DIR)/mailbox.o \
      $(BUILD_DIR)/framebuffer.o \
      $(BUILD_DIR)/terminal.o \
      $(BUILD_DIR)/shell.o \
      $(BUILD_DIR)/input.o

ELF = $(BUILD_DIR)/kernel.elf
IMG = $(BUILD_DIR)/kernel.img

all: $(IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot/boot.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.c kernel/framebuffer.h kernel/terminal.h kernel/shell.h kernel/input.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mailbox.o: kernel/mailbox.c kernel/mailbox.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: kernel/framebuffer.c kernel/framebuffer.h kernel/mailbox.h kernel/font.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/terminal.o: kernel/terminal.c kernel/terminal.h kernel/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell.o: kernel/shell.c kernel/shell.h kernel/terminal.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/input.o: kernel/input.c kernel/input.h kernel/terminal.h kernel/shell.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $(ELF)

$(IMG): $(ELF)
	arm-none-eabi-objcopy $(ELF) -O binary $(IMG)

clean:
	rm -rf $(BUILD_DIR)