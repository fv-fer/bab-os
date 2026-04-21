#include <stdint.h>

void kmain() {
    // Pointer to the start of VGA text buffer
    volatile char* video_memory = (volatile char*) 0xB8000;

    // Directly write 'H' with white color (0x0F)
    video_memory[0] = 'H';
    video_memory[1] = 0x0F;
    video_memory[2] = 'e';
    video_memory[3] = 0x0F;
    video_memory[4] = 'l';
    video_memory[5] = 0x0F;
    video_memory[6] = 'l';
    video_memory[7] = 0x0F;
    video_memory[8] = 'o';
    video_memory[9] = 0x0F;

    while(1);
}
