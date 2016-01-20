#ifndef __TinyKeyboard_h__
#define __TinyKeyboard_h__

#include <Arduino.h>
#include <Print.h>
#include <avr/wdt.h>
#include <util/delay.h>

extern "C" {
  #include <usbdrv.h>
}

#include "ascii_keycode_table.h"


#define LEFT_CONTROL  _BV(0)
#define LEFT_SHIFT    _BV(1)
#define LEFT_ALT      _BV(2)
#define LEFT_GUI      _BV(3)
#define RIGHT_CONTROL _BV(4)
#define RIGHT_SHIFT   _BV(5)
#define RIGHT_ALT     _BV(6)
#define RIGHT_GUI     _BV(7)


USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]);

/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and but we do allow
 * simultaneous key presses. 
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */
/*
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
*/

/* USB HID report descriptor for boot protocol keyboard
 * see HID1_11.pdf appendix B section 1
 * USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH is defined in usbconfig (should be 173)
 */
const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
  0x05, 0x01,           // USAGE_PAGE (Generic Desktop)
  0x09, 0x02,           // USAGE (Mouse)
  0xa1, 0x01,           // COLLECTION (Application)
  0x09, 0x01,           //   USAGE (Pointer)
  0xA1, 0x00,           //   COLLECTION (Physical)
  0x85, REPID_MOUSE,    //     REPORT_ID
  0x05, 0x09,           //     USAGE_PAGE (Button)
  0x19, 0x01,           //     USAGE_MINIMUM
  0x29, 0x03,           //     USAGE_MAXIMUM
  0x15, 0x00,           //     LOGICAL_MINIMUM (0)
  0x25, 0x01,           //     LOGICAL_MAXIMUM (1)
  0x95, 0x03,           //     REPORT_COUNT (3)
  0x75, 0x01,           //     REPORT_SIZE (1)
  0x81, 0x02,           //     INPUT (Data,Var,Abs)
  0x95, 0x01,           //     REPORT_COUNT (1)
  0x75, 0x05,           //     REPORT_SIZE (5)
  0x81, 0x03,           //     INPUT (Const,Var,Abs)
  0x05, 0x01,           //     USAGE_PAGE (Generic Desktop)
  0x09, 0x30,           //     USAGE (X)
  0x09, 0x31,           //     USAGE (Y)
  0x15, 0x81,           //     LOGICAL_MINIMUM (-127)
  0x25, 0x7F,           //     LOGICAL_MAXIMUM (127)
  0x75, 0x08,           //     REPORT_SIZE (8)
  0x95, 0x02,           //     REPORT_COUNT (2)
  0x81, 0x06,           //     INPUT (Data,Var,Rel)
  0xC0,                 //   END_COLLECTION
  0xC0,                 // END COLLECTION

  0x05, 0x01,           // USAGE_PAGE (Generic Desktop)
  0x09, 0x06,           // USAGE (Keyboard)
  0xA1, 0x01,           // COLLECTION (Application)
  0x85, REPID_KEYBOARD, // REPORT_ID
  0x75, 0x01,           //   REPORT_SIZE (1)
  0x95, 0x08,           //   REPORT_COUNT (8)
  0x05, 0x07,           //   USAGE_PAGE (Keyboard)(Key Codes)
  0x19, 0xE0,           //   USAGE_MINIMUM (Keyboard LeftControl)(224)
  0x29, 0xE7,           //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
  0x15, 0x00,           //   LOGICAL_MINIMUM (0)
  0x25, 0x01,           //   LOGICAL_MAXIMUM (1)
  0x81, 0x02,           //   INPUT (Data,Var,Abs) ; Modifier byte
  0x95, 0x01,           //   REPORT_COUNT (1)
  0x75, 0x08,           //   REPORT_SIZE (8)
  0x81, 0x03,           //   INPUT (Cnst,Var,Abs) ; Reserved byte
  0x95, 0x05,           //   REPORT_COUNT (5)
  0x75, 0x01,           //   REPORT_SIZE (1)
  0x05, 0x08,           //   USAGE_PAGE (LEDs)
  0x19, 0x01,           //   USAGE_MINIMUM (Num Lock)
  0x29, 0x05,           //   USAGE_MAXIMUM (Kana)
  0x91, 0x02,           //   OUTPUT (Data,Var,Abs) ; LED report
  0x95, 0x01,           //   REPORT_COUNT (1)
  0x75, 0x03,           //   REPORT_SIZE (3)
  0x91, 0x03,           //   OUTPUT (Cnst,Var,Abs) ; LED report padding
  0x95, 0x05,           //   REPORT_COUNT (5)
  0x75, 0x08,           //   REPORT_SIZE (8)
  0x15, 0x00,           //   LOGICAL_MINIMUM (0)
  0x26, 0xA4, 0x00,     //   LOGICAL_MAXIMUM (164)
  0x05, 0x07,           //   USAGE_PAGE (Keyboard)(Key Codes)
  0x19, 0x00,           //   USAGE_MINIMUM (Reserved (no event indicated))(0)
  0x2A, 0xA4, 0x00,     //   USAGE_MAXIMUM (Keyboard Application)(164)
  0x81, 0x00,           //   INPUT (Data,Ary,Abs)
  0xC0,                 // END_COLLECTION

  0x05, 0x0C,           // USAGE_PAGE (Consumer Devices)
  0x09, 0x01,           // USAGE (Consumer Control)
  0xA1, 0x01,           // COLLECTION (Application)
  0x85, REPID_MMKEY,    //   REPORT_ID
  0x19, 0x00,           //   USAGE_MINIMUM (Unassigned)
  0x2A, 0x3C, 0x02,     //   USAGE_MAXIMUM
  0x15, 0x00,           //   LOGICAL_MINIMUM (0)
  0x26, 0x3C, 0x02,     //   LOGICAL_MAXIMUM
  0x95, 0x01,           //   REPORT_COUNT (1)
  0x75, 0x10,           //   REPORT_SIZE (16)
  0x81, 0x00,           //   INPUT (Data,Ary,Abs)
  0xC0,                 // END_COLLECTION

  0x05, 0x01,             // USAGE_PAGE (Generic Desktop)
  0x09, 0x80,             // USAGE (System Control)
  0xA1, 0x01,             // COLLECTION (Application)
  0x85, REPID_SYSCTRLKEY, //   REPORT_ID
  0x95, 0x01,             //   REPORT_COUNT (1)
  0x75, 0x02,             //   REPORT_SIZE (2)
  0x15, 0x01,             //   LOGICAL_MINIMUM (1)
  0x25, 0x03,             //   LOGICAL_MAXIMUM (3)
  0x09, 0x82,             //   USAGE (System Sleep)
  0x09, 0x81,             //   USAGE (System Power)
  0x09, 0x83,             //   USAGE (System Wakeup)
  0x81, 0x60,             //   INPUT
  0x75, 0x06,             //   REPORT_SIZE (6)
  0x81, 0x03,             //   INPUT (Cnst,Var,Abs)
  0xC0,                   // END_COLLECTION
};

static unsigned char idleRate = 500 / 4; // see HID1_11.pdf section 7.2.4
static unsigned char protocol = 0;       // see HID1_11.pdf section 7.2.6
static unsigned char ledState = 0;

class TinyKeyboard : public Print {
public:
  TinyKeyboard() {
    wdt_disable();
    noInterrupts();

    usbInit();

    usbDeviceDisconnect();
    _delay_ms(250);
    usbDeviceConnect();

    interrupts();
  }
  
  void update() {
    usbPoll();
  }
  
  void delay(unsigned long ms) {
    for (ms += millis(); millis() < ms; update());
  }
  
  size_t write(uint8_t ch) {
    if (ch & 0x80) return 0;
    uint8_t data = pgm_read_byte_near(ascii_to_keycode + ch);
    sendKeyStroke(data & 0x7F, (data & 0x80) ? RIGHT_SHIFT : 0);
    sendKeyStroke(0);
    return 1;
  }

  void sendKeyStroke(uint8_t keyStroke) {
    sendKeyStroke(keyStroke, 0);
  }

  void sendKeyStroke(uint8_t keyStroke, uint8_t modifiers) {
    while (!usbInterruptIsReady()) update();
    
    reportBuffer[0] = modifiers;
    reportBuffer[1] = keyStroke;

    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }
    
private:
  unsigned char reportBuffer[2];

  using Print::write;

  friend USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]);
} Keyboard;

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (usbRequest_t *)data;

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    /* class request type */

    switch (rq->bRequest) {
      case USBRQ_HID_GET_IDLE: {
        usbMsgPtr = &idleRate;
        return 1;
      }

      case USBRQ_HID_SET_IDLE: {
        idleRate = rq->wValue.bytes[1];
        return 0;
      }

      case USBRQ_HID_GET_PROTOCOL: {
        usbMsgPtr = &protocol;
        return 1;
      }

      case USBRQ_HID_SET_PROTOCOL: {
        protocol = rq->wValue.bytes[1];
        return 0;
      }

      case USBRQ_HID_GET_REPORT: {
        usbMsgPtr = Keyboard.reportBuffer;

        Keyboard.reportBuffer[0] = rq->wValue.bytes[0];
        memset(Keyboard.reportBuffer + 1, 0, sizeof(Keyboard.reportBuffer) - 1); // clear the report
        // determine the return data length based on which report ID was requested
        if (rq->wValue.bytes[0] == REPID_MOUSE)      return REPSIZE_MOUSE;
        if (rq->wValue.bytes[0] == REPID_KEYBOARD)   return REPSIZE_KEYBOARD;
        if (rq->wValue.bytes[0] == REPID_MMKEY)      return REPSIZE_MMKEY;
        if (rq->wValue.bytes[0] == REPID_SYSCTRLKEY) return REPSIZE_SYSCTRLKEY;
        return 8; // default
      }

      case USBRQ_HID_SET_REPORT: {
        if (rq->wLength.word == 1) // check data is available
        {
          // 1 byte, we don't check report type (it can only be output or feature)
          // we never implemented "feature" reports so it can't be feature
          // so assume "output" reports
          // this means set LED status
          // since it's the only one in the descriptor
          return USB_NO_MSG; // send nothing but call usbFunctionWrite
        }
        else // no data or do not understand data, ignore
        {
          return 0; // send nothing
        }
      }

      default: {
        /* no vendor specific requests implemented */
        return 0;
      }

    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      /* wValue: ReportType (highbyte), ReportID (lowbyte)
         We only have one report type, so don't look at wValue */
      usbMsgPtr = Keyboard.reportBuffer;
      return sizeof(Keyboard.reportBuffer);
    }
  }
}

#endif // __TinyKeyboard_h__