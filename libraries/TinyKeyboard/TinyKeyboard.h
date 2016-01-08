#ifndef __TinyKeyboard_h__
#define __TinyKeyboard_h__

#include <util/delay.h>
extern "C" {
#include <usbdrv.h>
}
#include "usage_page.h"
#include "ascii_keycode_table.h"

extern "C" {
  USB_PUBLIC usbMsgLen_t usbFunctionSetup(unsigned char data[8]);
}

/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and but we do allow
 * simultaneous key presses. 
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */

const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] PROGMEM = {
  0x05, 0x01, // USAGE_PAGE (Generic Desktop) 
  0x09, 0x06, // USAGE (Keyboard) 
  0xa1, 0x01, // COLLECTION (Application) 
  0x05, 0x07, //   USAGE_PAGE (Keyboard) 
  0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl) 
  0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI) 
  0x15, 0x00, //   LOGICAL_MINIMUM (0) 
  0x25, 0x01, //   LOGICAL_MAXIMUM (1) 
  0x75, 0x01, //   REPORT_SIZE (1) 
  0x95, 0x08, //   REPORT_COUNT (8) 
  0x81, 0x02, //   INPUT (Data,Var,Abs) 
  0x95, 0x01, //   REPORT_COUNT (simultaneous keystrokes) 
  0x75, 0x08, //   REPORT_SIZE (8) 
  0x25, 0x65, //   LOGICAL_MAXIMUM (101) 
  0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated)) 
  0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application) 
  0x81, 0x00, //   INPUT (Data,Ary,Abs) 
  0xc0        // END_COLLECTION 
};

#define LEFT_CONTROL  _BV(0)
#define LEFT_SHIFT    _BV(1)
#define LEFT_ALT      _BV(2)
#define LEFT_GUI      _BV(3)
#define RIGHT_CONTROL _BV(4)
#define RIGHT_SHIFT   _BV(5)
#define RIGHT_ALT     _BV(6)
#define RIGHT_GUI     _BV(7)

class TinyKeyboard : public Print {
public:
  TinyKeyboard() {
    noInterrupts();

    usbDeviceDisconnect();
    _delay_ms(250);
    usbDeviceConnect();

    usbInit();
      
    interrupts();

    // TODO: Remove the next two lines once we fix
    //       missing first keystroke bug properly.
    memset(reportBuffer, 0, sizeof(reportBuffer));      
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }
  
  void update() {
    usbPoll();
  }
  
  // delay while updating until we are finished delaying
  void delay(unsigned long ms) {
    for (ms += millis(); millis() < ms; update()) continue;
  }
  
  size_t write(uint8_t ch) {
    uint8_t data = pgm_read_byte_near(ascii_to_keycode + ch);
    sendKeyStroke(data & 0x7F, (data & 0x80) ? RIGHT_SHIFT : 0);
    //sendKeyStroke(KeyCodes::Keyboard_A, LEFT_SHIFT);
    return 1;
  }
    
private:
  void sendKeyStroke(uint8_t keyStroke) {
    sendKeyStroke(keyStroke, 0);
  }

  void sendKeyStroke(uint8_t keyStroke, uint8_t modifiers) {
    while (!usbInterruptIsReady()) {
      // Note: We wait until we can send keystroke
      //       so we know the previous keystroke was
      //       sent.
      update();
      _delay_ms(5);
    }
    
    reportBuffer[0] = modifiers;
    reportBuffer[1] = keyStroke;
    
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
    
    while (!usbInterruptIsReady()) {
      // Note: We wait until we can send keystroke
      //       so we know the previous keystroke was
      //       sent.
      update();
      _delay_ms(5);
    }
      
    // This stops endlessly repeating keystrokes:
    memset(reportBuffer, 0, sizeof(reportBuffer));      
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }

  friend USB_PUBLIC unsigned char usbFunctionSetup(unsigned char data[8]);
  
  unsigned char reportBuffer[2];

  using Print::write;
} Keyboard;

#ifdef __cplusplus
extern "C"{
#endif 
static unsigned char idleRate;

USB_PUBLIC usbMsgLen_t usbFunctionSetup(unsigned char data[8]) {
  //usbRequest_t *rq = (usbRequest_t *)((void *)data);
  usbRequest_t *rq = (usbRequest_t *)data;

  usbMsgPtr = Keyboard.reportBuffer;
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    /* class request type */

    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      /* wValue: ReportType (highbyte), ReportID (lowbyte) */

      /* we only have one report type, so don't look at wValue */
      // TODO: Ensure it's okay not to return anything here?    
      return 0;

    } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
      //usbMsgPtr = &idleRate;
      //return 1;
      return 0;
      
    } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
      idleRate = rq->wValue.bytes[1];
      
    }
  } else {
    /* no vendor specific requests implemented */
  }
  
  return 0;
}
#ifdef __cplusplus
} // extern "C"
#endif


#endif // __TinyKeyboard_h__
