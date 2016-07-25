#ifndef _TCL_COLIBRI_UART_H
#define _TCL_COLIBRI_UART_H


extern void EvbUart1Config(void);
extern void EvbUart1ReadByte(char* c);
extern void EvbUart1WriteByte(char c);
extern void EvbUart1WriteStr(const char* str);

#endif /* _TCL_COLIBRI_UART_H */
