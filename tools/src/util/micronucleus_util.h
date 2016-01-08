#ifndef MICRONUCLEUS_UTIL_H
#define MICRONUCLEUS_UTIL_H

#if defined WIN
  #include <lusb0_usb.h>  // this is libusb, see http://libusb.sourceforge.net/
#else
  #include <usb.h>        // this is libusb, see http://libusb.sourceforge.net/
#endif

// #include <assert.h>
// #include <stdio.h>
// #include <stdlib.h>

#define MICRONUCLEUS_VENDOR_ID   0x16D0
#define MICRONUCLEUS_PRODUCT_ID  0x0753
#define MICRONUCLEUS_USB_TIMEOUT 0xFFFF
#define MICRONUCLEUS_MAX_MAJOR_VERSION 2

#define MICRONUCLEUS_COMMANDLINE_VERSION "2.0b5"

struct MicronucleusVersion {
  unsigned char major, minor;
};

class Micronucleus {
public:
  bool connected;
  usb_dev_handle *device;
  MicronucleusVersion version;
  unsigned int flashSize;       // programmable size (in bytes) of progmem
  unsigned int pageSize;        // size (in bytes) of page
  unsigned int bootloaderStart; // Start of the bootloader
  unsigned int pages;           // total number of pages to program
  unsigned int writeSleep;      // milliseconds
  unsigned int eraseSleep;      // milliseconds
  unsigned int signature;       // only used in protocol v2
  void (*callbackFn)(float);

  Micronucleus();
  int connect(bool);
  int erase();
  int write(unsigned char *, unsigned int);
  int run();
};

#endif