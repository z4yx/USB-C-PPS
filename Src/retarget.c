#include "main.h"
extern UART_HandleTypeDef DBG_UART;

int _write (int fd, char *ptr, int len);
int _read (int fd, char *ptr, int len);
void _ttywrch(int ch);

/* Retargeting functions for gcc-arm-embedded */

int _write (int fd, char *ptr, int len)
{
  /* Write "len" of char from "ptr" to file id "fd"
   * Return number of char written.
   * Need implementing with UART here. */
  int i;
  for (i = 0; i < len; ++i)
  {
    _ttywrch(ptr[i]);
  }
  return len;
}

int _read (int fd, char *ptr, int len)
{
  /* Read "len" of char to "ptr" from file id "fd"
   * Return number of char read.
   * Need implementing with UART here. */
  return 0;
}

void _ttywrch(int ch) {
  /* Write one char "ch" to the default console
   * Need implementing with UART here. */
  HAL_UART_Transmit(&DBG_UART, &ch, 1, 1000);
}