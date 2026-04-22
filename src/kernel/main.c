#include <stdint.h>

void kmain() {
    // Pointer to the start of VGA text buffer
    volatile char* video_memory = (volatile char*) 0xB8000;

    // Write "Hello Kernel" to row 10, column 0
    // Each row is 80 characters, each character is 2 bytes (char + attr)
    int offset = 5 * 80 * 2;
    char *msg = "Hello Kernel";
    
    for (int i = 0; msg[i] != '\0'; i++) {
        video_memory[offset + i * 2] = msg[i];
        video_memory[offset + i * 2 + 1] = 0x0A; // Green on Black
    }

    while(1);
}
