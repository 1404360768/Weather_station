#ifndef __FONTS_H
#define __FONTS_H

#define MAX_HEIGHT_FONT 41
#define MAX_WIDTH_FONT 32
#define OFFSET_BITMAP

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

  //ASCII
  typedef struct
  {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
  } sFONT;

    //ASCII
  typedef struct
  {
    const uint16_t *data;
    uint16_t width;
    uint16_t height;
  } ssFONT;

  //GB2312
  typedef struct
  {
    unsigned char index[2];
    const char matrix[MAX_HEIGHT_FONT * MAX_WIDTH_FONT / 8];
  } CH_CN;

  typedef struct
  {
    const CH_CN *table;
    uint16_t size;
    uint16_t ASCII_Width;
    uint16_t Width;
    uint16_t Height;

  } cFONT;

  extern sFONT Font24;
  extern sFONT Font20;
  extern sFONT Font16;
  extern sFONT Font12;
  extern sFONT Font8;

  extern ssFONT Font_7x10;
  extern ssFONT Font_11x18;
  extern ssFONT Font_16x26;

  extern cFONT Font12CN;
  extern cFONT Font24CN;
#ifdef __cplusplus
}
#endif

#endif /* __FONTS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
