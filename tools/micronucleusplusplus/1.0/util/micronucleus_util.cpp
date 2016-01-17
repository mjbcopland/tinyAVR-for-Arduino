#include <micronucleus_util.h>
#include <delay_util.h>
#include <cassert>
#include <iostream>
// #include <stdio.h>
// #include <stdlib.h>

using namespace std;

Micronucleus::Micronucleus() {
  device = NULL;
  callbackFn = NULL;
  connected = false;
}

int Micronucleus::connect(bool fastMode) {
  connected = false;

  usb_init();
  usb_find_busses();
  usb_find_devices();

  for (struct usb_bus *bus = usb_get_busses(); bus; bus = bus->next) {
    for (struct usb_device *dev = bus->devices; dev; dev = dev->next) {
      if (dev->descriptor.idVendor == MICRONUCLEUS_VENDOR_ID && dev->descriptor.idProduct == MICRONUCLEUS_PRODUCT_ID) {
        version.major = (dev->descriptor.bcdDevice >> 8) & 0xFF;
        version.minor = (dev->descriptor.bcdDevice >> 0) & 0xFF;

        device = usb_open(dev);

        switch (version.major) {
          case 1: {
            unsigned char buffer[4];
            int res = usb_control_msg(device, USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, (char *)buffer, 4, MICRONUCLEUS_USB_TIMEOUT);
            
            // Device descriptor was found, but talking to it was not successful. This can happen when the device is being reset.
            if (res < 0) {
              device = NULL;
              return 2;
            }
              
            assert(res >= 4);

            flashSize = (buffer[0] << 8) | buffer[1];
            pageSize = buffer[2];
            pages = flashSize / pageSize;
            if (pages * pageSize < flashSize) pages += 1;
            
            bootloaderStart = pages * pageSize;
            
            writeSleep = buffer[3] & 0x7F;
            eraseSleep = writeSleep * pages;       
            
            signature = 0;

            break;
          }
          
          case 2: {
            unsigned char buffer[6];
            int res = usb_control_msg(device, USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, (char *)buffer, 6, MICRONUCLEUS_USB_TIMEOUT);

            // Device descriptor was found, but talking to it was not successful. This can happen when the device is being reset.
            if (res < 0) {
              device = NULL;
              return 2;
            }

            assert(res >= 6);

            flashSize = (buffer[0] << 8) | buffer[1];
            pageSize = buffer[2];
            pages = flashSize / pageSize;
            if (pages * pageSize < flashSize) pages += 1;

            bootloaderStart = pages * pageSize;

            // firmware v2 reports more aggressive write times. Add 2ms if fast mode is not used.
            writeSleep = (buffer[3] & 0x7F) + (fastMode ? 0 : 2);

            // if bit 7 of write sleep time is set, divide the erase time by four to accomodate to the 4*page erase of the ATtiny841/441
            eraseSleep = (writeSleep * pages) / (buffer[3] & 0x80 ? 4 : 1);

            signature = (buffer[4] << 8) | buffer[5];

            break;
          }
          
          default: {
            usb_close(device);
            device = NULL;

            cerr << "Warning: device with unknown version of Micronucleus detected." << endl
                 << "This tool doesn't know how to upload to the device. Updates may be available." << endl
                 << "Device reports version as: " << (int)version.major << "." << (int)version.minor << endl;

            return 1;
          }
        }

        connected = true;
        goto break_outer;
      }
    }
  }
  break_outer:

  return 0;
}

int Micronucleus::erase() {
  if (device == NULL) {
    cout << "Device is null!" << endl;
    exit(EXIT_FAILURE);
  }
  int res = usb_control_msg(device, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 2, 0, 0, NULL, 0, MICRONUCLEUS_USB_TIMEOUT);

  // give microcontroller enough time to erase all writable pages and come back online
  if (callbackFn) {
    for (int i = 0, j = pages/4; i < j; i++) {
      callbackFn((float)i / j);
      delay(eraseSleep / j);
    }
    callbackFn(1.0);
  } else {
    delay(eraseSleep);
  }


  /* Under Linux, the erase process is often aborted with errors such as:
   usbfs: USBDEVFS_CONTROL failed cmd micronucleus rqt 192 rq 2 len 0 ret -84
   This seems to be because the erase is taking long enough that the device
   is disconnecting and reconnecting.  Under Windows, micronucleus can see this
   and automatically reconnects prior to uploading the program.  To get the
   the same functionality, we must flag this state (the "-84" error result) by
   converting the return to 1 for the upper layer.

   On Mac OS a common error is -34 = epipe, but adding it to this list causes
   assert(res >= 4) in function Micronucleus::connect to fail
  */
  switch (res) {
    case -34:
      usb_close(device);
      device = NULL;
      connected = false;
    case  -5:
    case -84:
      return 1;

    default:
      return res;
  }
}

int Micronucleus::write(unsigned char *program, unsigned int programSize) {
  for (unsigned int address = 0; address < flashSize; address += pageSize) {
    unsigned char pageLength = pageSize;
    unsigned char pageBuffer[pageLength];
    unsigned int userReset;

    if (version.major == 1 && version.minor <= 2 && address / pageSize == pages - 1) pageLength = flashSize % pageSize;

    bool pageContainsData = true;

    // copy in bytes from user program
    if (address > programSize) {
      pageContainsData = false;
      memset(pageBuffer, 0xFF, pageLength);
    } else if (address + pageLength > programSize) {
      unsigned int remaining = programSize - address;
      memcpy(pageBuffer, program + address, remaining);
      memset(pageBuffer + remaining, 0xFF, pageLength - remaining);
    } else {
      memcpy(pageBuffer, program + address, pageLength);
    }

    // Reset vector patching is done in the host tool in micronucleus >=2
    if (version.major >= 2) {
      if (address == 0) {
        // save user reset vector (bootloader will patch with its vector)
        unsigned int word[2] = {
          ((unsigned int)pageBuffer[1] << 8) | pageBuffer[0],
          ((unsigned int)pageBuffer[3] << 8) | pageBuffer[2]
        };

        if (word[0] == 0x940C) {  // long jump
          userReset = word[1];
        } else if ((word[0] & 0xF000) == 0xC000) {  // rjmp
          userReset = (word[0] & 0x0FFF) + 1;
        } else {
          cerr << "The reset vector of the user program does not contain a branch instruction," << endl
               << "therefore the bootloader can not be inserted. Please rearrage your code." << endl;

          return -1;
        }

        // patch in jmp to bootloader
        if (bootloaderStart > 0x2000) {
          // jmp
          unsigned int data = 0x940C;
          pageBuffer[0] = (data >> 0) & 0xFF;
          pageBuffer[1] = (data >> 8) & 0xFF;
          pageBuffer[2] = (bootloaderStart >> 0) & 0xFF;
          pageBuffer[3] = (bootloaderStart >> 8) & 0xFF;
        } else {
          // rjmp
          unsigned int data = 0xC000 | ((bootloaderStart/2 - 1) & 0x0FFF);
          pageBuffer[0] = (data >> 0) & 0xFF;
          pageBuffer[1] = (data >> 8) & 0xFF;
        }
      }

      if (address >= bootloaderStart - pageSize) {
        unsigned int userResetAddress = (pages * pageSize) - 4;

        if (userResetAddress > 0x2000) {
          // jmp
          unsigned int data = 0x940C;
          pageBuffer[userResetAddress - address + 0] = (data >> 0) & 0xFF;
          pageBuffer[userResetAddress - address + 1] = (data >> 8) & 0xFF;
          pageBuffer[userResetAddress - address + 2] = (userReset >> 0) & 0xFF;
          pageBuffer[userResetAddress - address + 3] = (userReset >> 8) & 0xFF;
        } else {
          // rjmp
          unsigned int data = 0xC000 | ((userReset - userResetAddress/2 - 1) & 0x0FFF);
          pageBuffer[userResetAddress - address + 0] = (data >> 0) & 0xFF;
          pageBuffer[userResetAddress - address + 1] = (data >> 8) & 0xFF;
        }
      }
    }

    // always write last page so bootloader can insert the tiny vector table
    if (address >= bootloaderStart - pageSize) {
      pageContainsData = true;
    }

    if (pageContainsData) {
      switch (version.major) {
        case 1: {
          // firmware v1 transfers a page as a single block
          // ask the microcontroller to write this page's data:
          if (usb_control_msg(device, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 1, pageLength, address, (char*)pageBuffer, pageLength, MICRONUCLEUS_USB_TIMEOUT) != 0) {
            return -1;
          }
          break;
        }

        case 2: {
          // firmware v2 uses individual set up packets to transfer data
          if (usb_control_msg(device, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 1, pageLength, address, NULL, 0, MICRONUCLEUS_USB_TIMEOUT) != 0) {
            return -1;
          }

          for (int i = 0; i < pageLength; i += 4) {
            unsigned int word[2] = {
              ((unsigned int)pageBuffer[i+1] << 8) | pageBuffer[i+0],
              ((unsigned int)pageBuffer[i+3] << 8) | pageBuffer[i+2]
            };

            if (usb_control_msg(device, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 3, word[0], word[1], NULL, 0, MICRONUCLEUS_USB_TIMEOUT) != 0) {
              return -1;
            }
          }
          break;
        }

        default: {
          assert(false);
        }
      }

      // give microcontroller enough time to write this page and come back online
      delay(writeSleep);
    }

    if (callbackFn) callbackFn((float)address / flashSize);
  }

  if (callbackFn) callbackFn(1.0);

  return 0;
}

int Micronucleus::run() {
  return (usb_control_msg(device, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 4, 0, 0, NULL, 0, MICRONUCLEUS_USB_TIMEOUT) == 0 ? 0 : -1);
}