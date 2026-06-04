#include <stdint.h>
#include <terminal.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <keyboard.h>
#include <timer.h>
#include <pmm.h>
#include <vmm.h>
#include <kheap.h>
#include <vbe.h>
#include <string.h>
#include <task.h>
#include <monitor.h>
#include <vfs.h>
#include <initrd.h>

/* Defined in pmm.c but not in header to keep it clean */
extern void pmm_add_region(uint32_t start, uint32_t length);

/* Defined in linker script */
extern uint32_t _kernel_virtual_end;

#define BUFFER_SIZE 5
#define COND_NOT_EMPTY 0
#define COND_NOT_FULL  1

monitor_t buffer_monitor;
int buffer[BUFFER_SIZE];
int count = 0;
int head = 0;
int tail = 0;

void producer() {
    int item = 100;
    while(1) {
        sleep(300); // Produce quickly

        monitor_enter(&buffer_monitor);
        
        while (count == BUFFER_SIZE) {
            printf("[P: buffer full, waiting...]");
            monitor_wait(&buffer_monitor, COND_NOT_FULL);
        }

        buffer[tail] = item++;
        tail = (tail + 1) % BUFFER_SIZE;
        count++;
        
        printf("[P: produced, count %d]", count);
        
        monitor_notify(&buffer_monitor, COND_NOT_EMPTY);
        monitor_exit(&buffer_monitor);
    }
}

void consumer() {
    while(1) {
        sleep(800); // Consume slowly to force buffer to fill up

        monitor_enter(&buffer_monitor);

        while (count == 0) {
            printf("[C: buffer empty, waiting...]");
            monitor_wait(&buffer_monitor, COND_NOT_EMPTY);
        }

        int item = buffer[head];
        head = (head + 1) % BUFFER_SIZE;
        count--;

        printf("[C: consumed %d, count %d]", item, count);

        monitor_notify(&buffer_monitor, COND_NOT_FULL);
        monitor_exit(&buffer_monitor);
    }
}

void shell_task() {
    char cmd_buffer[128];
    int cmd_idx = 0;

    printf("Shell started. Type 'ls', 'cat <file>', or just type characters!\n> ");
    while(1) {
        char c = getchar();
        
        if (c == '\n') {
            cmd_buffer[cmd_idx] = '\0';
            
            if (strcmp(cmd_buffer, "ls") == 0) {
                int i = 0;
                struct vfs_dirent *node = 0;
                while ((node = vfs_readdir(fs_root, i)) != 0) {
                    printf("%s  ", node->name);
                    vfs_node_t *fsnode = vfs_finddir(fs_root, node->name);
                    if ((fsnode->flags & 0x7) == VFS_DIRECTORY)
                        printf("(dir)\n");
                    else
                        printf("(file, size %d)\n", fsnode->length);
                    i++;
                }
            } else if (strncmp(cmd_buffer, "cat ", 4) == 0) {
                char *filename = cmd_buffer + 4;
                vfs_node_t *fsnode = vfs_finddir(fs_root, filename);
                if (fsnode) {
                    char *buf = (char*)kmalloc(fsnode->length + 1);
                    uint32_t sz = vfs_read(fsnode, 0, fsnode->length, (uint8_t*)buf);
                    buf[sz] = '\0';
                    printf("%s\n", buf);
                    kfree(buf);
                } else {
                    printf("File not found: %s\n", filename);
                }
            } else if (cmd_idx > 0) {
                printf("Unknown command: %s\n", cmd_buffer);
            }
            
            printf("> ");
            cmd_idx = 0;
        } else if (c == '\b') {
            if (cmd_idx > 0) {
                cmd_idx--;
                /* terminal_putchar('\b') followed by space and '\b' 
                   would handle visual delete, but terminal_putchar 
                   doesn't support backspace yet. 
                   For now, just decrement buffer index. */
            }
        } else {
            if (cmd_idx < 127) {
                cmd_buffer[cmd_idx++] = c;
            }
        }
    }
}

void kmain() {
    gdt_init();
    idt_init();
    isr_install();

    /* 1. Initialize PMM Metadata */
    pmm_init(128 * 1024 * 1024, 0x400000);

    /* 2. Add available memory to the Buddy System */
    pmm_add_region(0x500000, (128 - 5) * 1024 * 1024);

    /* 3. Initialize Virtual Memory and Enable Paging */
    vmm_init();

    /* 4. Initialize Kernel Heap */
    kheap_init();

    timer_init(100); // 100 Hz

    struct vbe_mode_info* vbe = (struct vbe_mode_info*) 0xC0000500;
    terminal_initialize(vbe);

    printf("Bab-OS Kernel Booting...\n");

    /* Initialize VFS and Initrd */
    /* We've padded the image so that initrd starts at physical 0x40000.
       In higher-half, this is 0xC0040000. */
    fs_root = initialise_initrd(0xC0040000);
    printf("VFS Initialized. Root at %x\n", fs_root);

    keyboard_init();

    monitor_init(&buffer_monitor);

    /* 5. Initialize Multitasking */
    task_init();
    task_create(producer);
    task_create(consumer);
    task_create(shell_task);

    printf("System Ready. Multitasking active!\n");

    // Enable interrupts
    __asm__ volatile("sti");

    while(1) {
        __asm__ volatile("hlt");
    }
}
