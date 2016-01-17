#ifndef __bootloaderconfig_h_included__
#define __bootloaderconfig_h_included__



/* ---------------------------- Hardware Config ---------------------------- */

#define USB_CFG_IOPORTNAME  B

#define USB_CFG_DMINUS_BIT  3

#define USB_CFG_DPLUS_BIT   4

#define USB_CFG_CLOCK_KHZ   (F_CPU/1000)



/* ----------------------- Optional MCU Description ------------------------ */

// setup interrupt for Pin Change for D+
#define USB_INTR_CFG          PCMSK
#define USB_INTR_CFG_SET      _BV(USB_CFG_DPLUS_BIT)
#define USB_INTR_CFG_CLR      0
#define USB_INTR_ENABLE       GIMSK
#define USB_INTR_ENABLE_BIT   PCIE
#define USB_INTR_PENDING      GIFR
#define USB_INTR_PENDING_BIT  PCIF
#define USB_INTR_VECTOR       PCINT0_vect



#endif // __bootloader_h_included__