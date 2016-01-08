#ifndef __bootloaderconfig_h_included__
#define __bootloaderconfig_h_included__



/* ---------------------------- Hardware Config ---------------------------- */

#define USB_CFG_IOPORTNAME  B

#define USB_CFG_DMINUS_BIT  0

#define USB_CFG_DPLUS_BIT   1

#define USB_CFG_CLOCK_KHZ   (F_CPU/1000)



/* ----------------------- Optional MCU Description ------------------------ */

// setup interrupt for Pin Change for D+
#define USB_INTR_CFG          PCMSK1
#define USB_INTR_CFG_SET      _BV(USB_CFG_DPLUS_BIT)
#define USB_INTR_CFG_CLR      0
#define USB_INTR_ENABLE       GIMSK
#define USB_INTR_ENABLE_BIT   PCIE1
#define USB_INTR_PENDING      GIFR
#define USB_INTR_PENDING_BIT  PCIF1
#define USB_INTR_VECTOR       PCINT1_vect
    


#endif // __bootloader_h_included__