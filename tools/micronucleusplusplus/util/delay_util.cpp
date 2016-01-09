#include <delay_util.h>

/* Delay in miliseconds */
void delay(unsigned long duration) {
  #if defined _WIN32 || defined _WIN64
    // use windows sleep api with milliseconds
    Sleep(duration * 2);
  #else
    // use standard unix api with microseconds
    usleep(duration*1000);
  #endif
}
