CC = aarch64-linux-gnu-gcc
OBJCOPY = aarch64-linux-gnu-objcopy

CFLAGS = -Wall -O2 -ffreestanding -nostdlib -nostartfiles -mgeneral-regs-only -Ikernel
LDFLAGS = -T linker.ld -ffreestanding -nostdlib -nostartfiles

BUILD_DIR = build

OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/framebuffer.o \
	$(BUILD_DIR)/mailbox.o \
	$(BUILD_DIR)/terminal.o \
	$(BUILD_DIR)/shell.o \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/storage.o \
	$(BUILD_DIR)/pcie.o \
	$(BUILD_DIR)/usb.o \
	$(BUILD_DIR)/hid_keyboard.o

all: $(BUILD_DIR)/kernel8.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot/boot.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c boot/boot.S -o $(BUILD_DIR)/boot.o

$(BUILD_DIR)/%.o: kernel/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel8.img: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) $(BUILD_DIR)/kernel.elf -O binary $(BUILD_DIR)/kernel8.img

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.img kernel.elf kernel.img kernel8.img