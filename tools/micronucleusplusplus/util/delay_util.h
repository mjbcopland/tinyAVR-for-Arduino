#ifndef DELAY_UTIL_H
#define DELAY_UTIL_H

#if defined WIN
  #include <windows.h>
#else
  #include <unistd.h>
#endif

/* Delay in miliseconds */
void delay(unsigned long duration);

#endif
