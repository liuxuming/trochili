#ifndef _TCL_COLIBRI_Led_H
#define _TCL_COLIBRI_Led_H

#define LED_ON  (1)
#define LED_OFF (0)
#define LED1    (1)
#define LED2    (2)
#define LED3    (3)
#define LED4    (4)
#define LEDX   (5)

extern void EvbLedConfig(void);
extern void EvbLedControl(int index, int cmd);
extern void EvbLedToggle(int index);

#endif /* _TCL_COLIBRI_Led_H */
