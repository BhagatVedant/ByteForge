/*
    File: kernel.c

    This is the main kernel entry point for the OS ByteForge.

    The kernel initializes the framebuffer, terminal, shell, keyboard queue,
    and storage system. Then it runs a scripted demo by simulating typed
    commands through the keyboard queue.

    The delays are there so the demo is readable during the class presentation.
*/

#include "framebuffer.h"
#include "terminal.h"
#include "shell.h"
#include "input.h"
#include "keyboard.h"
#include "storage.h"

/*
    Simple delay function to slow down the demo for readability.

    The timer isn't real but rather it works on wasting CPU cycles to create a delay.
*/
static void delay(volatile unsigned int count) {
    while (count--) {
        asm volatile("nop");
    }
}

// Startup delay to let the monitor load before the demo starts
static void startup_delay(void) {
    delay(8000000);
}

// Demo function for the cursor to blink while waiting
static void demo_wait(unsigned int cycles) {
    volatile unsigned int blink_counter = 0;

    for (unsigned int i = 0; i < cycles; i++) {
        blink_counter++;

        if (blink_counter >= 50000) {
            terminal_toggle_cursor();
            blink_counter = 0;
        }
    }
}

/*
    Typing one command into the OS like a keyboard would to replicate real human input.

    Each character is pushed into the keyboard queue, then keyboard_poll()
    sends it into the input system. This makes the demo look like someone
    is typing commands live.
*/
static void demo_type_command(const char *command) {
    while (*command) {
        keyboard_push_char(*command);
        keyboard_poll();

        delay(120000);

        command++;
    }

    keyboard_push_char('\n');
    keyboard_poll();

    demo_wait(2500000);
}

// Main OS entry point
void kernel_main(void) {
    volatile unsigned int blink_counter = 0;

    startup_delay();

    if (framebuffer_init()) {
        terminal_init();
        shell_init();
        input_init();
        keyboard_init();
        storage_init();

        shell_prompt();

        // Short delay before starting to type commands for the demo
        demo_wait(2000000);

        /*
            Demo flow.

            This shows the full storage workflow:
            status -> devices -> mount -> list -> metadata -> open/read
            -> error handling -> unmount -> blocked access after unmount.
        */
        
        demo_type_command("help");
        demo_type_command("status");
        demo_type_command("devices");
        demo_type_command("mount usb0");
        demo_type_command("ls");
        demo_type_command("info hello.txt");
        demo_type_command("open hello.txt");
        demo_type_command("open notes.txt");
        demo_type_command("cat resume.txt");
        demo_type_command("open fake.txt");
        demo_type_command("unmount");
        demo_type_command("ls");

        terminal_set_color(0x00AAFFAA);
        terminal_write("Demo complete.\n");

        terminal_set_color(0x00FFFFFF);
        shell_prompt();
    }

    // Keeping the OS alive and keep the cursor blinking
    while (1)
    {
        keyboard_poll();

        blink_counter++;

        if (blink_counter >= 50000)
        {
            terminal_toggle_cursor();
            blink_counter = 0;
        }
    }
}