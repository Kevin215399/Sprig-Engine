#ifndef TFT_h
#define TFT_h

#include "hardware/spi.h"
#include <stdio.h>
#include <stdlib.h>

#define ST7735_TFTWIDTH_128 128  // for 1.44 and mini
#define ST7735_TFTWIDTH_80 80    // for mini
#define ST7735_TFTHEIGHT_128 128 // for 1.44" display
#define ST7735_TFTHEIGHT_160 160 // for 1.8" and mini display

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define ORANGE 0xFC00

#define TRANSPARENT 1

#define ST7735_FRMCTR1 0xB1
#define ST7735_DISSET5 0xB6

#define ST7735_INVCTR 0xB4

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2

#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4

#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3

uint8_t Bcmd[] = {
    // Init commands for black
    21,                           // 15 commands in list:
    ST77XX_SWRESET, ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
    150,                          //     150 ms delay
    ST77XX_SLPOUT, ST_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
    255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,            //  3: Framerate ctrl - normal mode, 3 arg:
    0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,            //  4: Framerate ctrl - idle mode, 3 args:
    0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,            //  5: Framerate - partial mode, 6 args:
    0x01, 0x2C, 0x2D,             //     Dot inversion mode
    0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR, 1,             //  6: Display inversion ctrl, 1 arg:
    0x07,                         //     No inversion
    ST7735_PWCTR1, 3,             //  7: Power control, 3 args, no delay:
    0xA2,
    0x02,             //     -4.6V
    0x84,             //     AUTO mode
    ST7735_PWCTR2, 1, //  8: Power control, 1 arg, no delay:
    0xC5,             //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3, 2, //  9: Power control, 2 args, no delay:
    0x0A,             //     Opamp current small
    0x00,             //     Boost frequency
    ST7735_PWCTR4, 2, // 10: Power control, 2 args, no delay:
    0x8A,             //     BCLK/2,
    0x2A,             //     opamp current small & medium low
    ST7735_PWCTR5, 2, // 11: Power control, 2 args, no delay:
    0x8A, 0xEE,
    ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
    0x0E,
    ST77XX_INVOFF, 0, // 13: Don't invert display, no args
    ST77XX_MADCTL, 1, // 14: Mem access ctl (directions), 1 arg:
    0xC8,             //     row/col addr, bottom-top refresh
    ST77XX_COLMOD, 1, // 15: set color mode, 1 arg, no delay:
    0x05,             //     16-bit color
    // Rcmd2
    ST77XX_CASET, 4, //  1: Column addr set, 4 args, no delay:
    0x00, 0x00,      //     XSTART = 0
    0x00, 0x7F,      //     XEND = 127
    ST77XX_RASET, 4, //  2: Row addr set, 4 args, no delay:
    0x00, 0x00,      //     XSTART = 0
    0x00, 0x9F,      //     XEND = 159
    // Rcmd 3
    ST7735_GMCTRP1, 16,     //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
    0x02, 0x1c, 0x07, 0x12, //     (Not entirely necessary, but provides
    0x37, 0x32, 0x29, 0x2d, //      accurate colors)
    0x29, 0x25, 0x2B, 0x39,
    0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16,     //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
    0x03, 0x1d, 0x07, 0x06, //     (Not entirely necessary, but provides
    0x2E, 0x2C, 0x29, 0x2D, //      accurate colors)
    0x2E, 0x2E, 0x37, 0x3F,
    0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON, ST_CMD_DELAY,  //  3: Normal display on, no args, w/delay
    10,                          //     10 ms delay
    ST77XX_DISPON, ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
    100};

bool font[] = {
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, // A
    1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, // B
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, // C
    1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, // D
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, // E
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, // F
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // G
    1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, // H
    0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, // I
    0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, // J
    1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, // K
    1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, // L
    1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, // M
    1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, // N
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // O
    1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, // P
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, // Q
    1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, // R
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, // S
    1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, // T
    1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // U
    1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, // V
    1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, // W
    1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, // X
    1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, // Y
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, // Z

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, // a
    1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, // b
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // c
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, // d
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, // e
    0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, // f
    0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, // g
    1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, // h
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, // i
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, // j
    1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, // k
    0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, // l
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, // m
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, // n
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // o
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, // p
    0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, // q
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, // r
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, // s
    0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, // t
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, // u
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, // v
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, // w
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, // x
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, // y
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, // z

    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // 0
    0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, // 1
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, // 2
    1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, // 3
    0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, // 4
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, // 5
    0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // 6
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, // 7
    0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, // 8
    0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, // 9

    0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // !
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, // @
    0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, // #
    0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, // $
    0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, // %
    0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // ^
    0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, // &
    0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // *
    0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, // (
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, // )
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // -
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // +
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, // _
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // =
    0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, // {
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, // }
    0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, // [
    0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, // ]
    0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, // |
    0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, //  back slash
    0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, // /
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, // ;
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, // :
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // '
    0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // "
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, // ,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, // .
    0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, // ?
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, // <
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, // >
    0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // `
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // ~
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // ' '
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, // ^^ uppercase
    1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, // vv lowercase
    0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, // return
};

#define UPPERCASE_SYB 128
#define LOWERCASE_SYB 129
#define RETURN_SYB 130

char fontKeys[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '-', '+', '_', '=', '{', '}', '[', ']', '|', '\\', '/',
    ';', ':', '\'', '\"', ',', '.', '?', '<', '>', '`', '~', ' ',
    UPPERCASE_SYB, LOWERCASE_SYB, RETURN_SYB};

#define SPI_PORT spi0
#define PIN_MISO 16
#define TFT_CS 20
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_DC 22
#define PIN_RST 26

#define BACK_LIGHT 17

#define X_OFFSET 26
#define Y_OFFSET 1

void SendCommandAndData(uint8_t cmd, uint8_t *data, uint16_t len)
{
    // printf("Cmd %u\n", cmd);
    gpio_put(TFT_CS, 0);                   // Select the TFT
    gpio_put(PIN_DC, 0);                   // Set DC low for command mode
    spi_write_blocking(SPI_PORT, &cmd, 1); // Send command
    if (len > 0)
    {
        gpio_put(PIN_DC, 1);                     // Set DC high for data mode
        spi_write_blocking(SPI_PORT, data, len); // Send data if any
    }
    gpio_put(TFT_CS, 1); // Deselect the TFT
}
void SendCommand(uint8_t cmd)
{
    SendCommandAndData(cmd, NULL, 0); // Send command with no data
}
void SendData(uint8_t *data, uint16_t len)
{
    gpio_put(TFT_CS, 0); // Select the TFT
    gpio_put(PIN_DC, 1); // Set DC high for data mode

    spi_write_blocking(SPI_PORT, data, len); // Send data
    gpio_put(TFT_CS, 1);                     // Deselect the TFT
}

void SetWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    SendCommand(0x2A); // Column addr set
    uint8_t data[] = {0x00, x, 0x00, (x + w - 1)};
    SendData(data, 4);

    SendCommand(0x2B); // Row addr set
    uint8_t data2[] = {0x00, y, 0x00, (y + h - 1)};
    SendData(data2, 4);

    SendCommand(0x2C); // Write to RAM
}

uint16_t RGBTo16(uint8_t r, uint8_t g, uint8_t b)
{
    // Convert RGB888 to RGB565
    return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
}

int max(int a, int b)
{
    if (a < b)
        return b;
    return a;
}
int min(int a, int b)
{
    if (a > b)
        return b;
    return a;
}
uint8_t leftBound = 0;
uint8_t rightBound = 160;
uint8_t topBound = 0;
uint8_t bottomBound = 128;

void SetPixel(int x, int y, uint16_t color)
{
    if (x > rightBound)
        return;
    if (x < leftBound)
        return;
    if (y < topBound)
        return;
    if (y > bottomBound)
        return;
    SetWindow(x, y, 1, 1);
    uint8_t data[] = {color >> 8, color & 0xFF};
    SendData(data, 2);
}

void FillAll(uint16_t color)
{
    SetWindow(0, 0, 160, 128);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    uint8_t buf[128 * 2]; // Buffer for efficiency
    for (int i = 0; i < 128 * 2; i += 2)
    {
        buf[i] = hi;
        buf[i + 1] = lo;
    }

    for (int i = 0; i < 160; i++)
    {
        SendData(buf, 128 * 2);
    }
}

void Rectangle(uint16_t color, int x, int y, uint8_t w, uint8_t h)
{
    if(x>rightBound){
        return;
    }
    if(y>bottomBound){
        return;
    }

    if(x+w<leftBound){
        return;
    }
    if(y+h<topBound){
        return;
    }

    if(x<leftBound){
        w+=x-leftBound;
        x=leftBound;
    }
    if(y<topBound){
        h+=y-topBound;
        y=topBound;
    }

    w = min(w,rightBound-x);
    h = min(h,bottomBound-y);


    SetWindow(x, y, w, h);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    uint8_t buf[h * 2]; // Buffer for efficiency
    for (int i = 0; i < h * 2; i += 2)
    {
        buf[i] = hi;
        buf[i + 1] = lo;
    }

    for (int i = 0; i < w; i++)
    {
        SendData(buf, h * 2);
    }
}

void Clear()
{
    FillAll(RGBTo16(0, 0, 0));
}

const uint8_t FONT_WIDTHS[] = {
    5,
    12,
    19,
};
const uint8_t FONT_HEIGHTS[] = {
    7,
    16,
    25,
};

void WriteLetter(char letter, int startX, int startY, uint8_t scale, uint16_t color, uint16_t backgroundColor)
{
    if (backgroundColor != TRANSPARENT)
    {
        uint8_t bufferWidth = 5 * scale + (scale - 1) * 2;
        uint8_t bufferHeight = 7 * scale + (scale - 1) * 2;

        for (int x = 0; x < bufferWidth; x++)
        {
            for (int y = 0; y < bufferHeight; y++)
            {
                SetPixel(startX + x, startY + y, backgroundColor);
            }
        }
    }

    uint8_t letterIndex = 255;
    for (int i = 0; i < sizeof(fontKeys) / sizeof(char); i++)
    {
        if (fontKeys[i] == letter)
        {
            letterIndex = i;
            break;
        }
    }
    if (letterIndex == 255)
    {
        return;
    }
    /*uint8_t bufferWidth = 5 * scale + (scale - 1) * 2;
    uint8_t bufferHeight = 7 * scale + (scale - 1) * 2;
    SetWindow(x, y, bufferWidth, bufferHeight);

    bool *buffer = (bool *)malloc(bufferWidth * bufferHeight * sizeof(bool));

    for (int i = 0; i < bufferWidth * bufferHeight; i++)
    {
        buffer[i] = 0; // Initialize buffer
    }*/
    for (int y = 0; y < 7; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (font[letterIndex * 35 + x + (y * 5)])
            {
                for (int dx = -scale + 1; dx < scale; dx++)
                {
                    for (int dy = -scale + 1; dy < scale; dy++)
                    {
                        // buffer[(x * scale + dx + (scale - 1)) + ((y * scale + dy + (scale - 1)) * bufferWidth)] = 1;
                        SetPixel(x * scale + dx + startX + (scale - 1), y * scale + dy + startY + (scale - 1), color);
                    }
                }
            }
        }
    }
    // free(buffer);
}

// Returns the number of lines
uint8_t WriteWord(char *word, int len, int x, int y, uint8_t scale, uint16_t color, uint16_t background)
{
    uint8_t letterWidth = 5 * scale + (scale - 1) * 2 + 1;
    uint8_t currentLine = 0;
    int xOffset = 0;
    for (int i = 0; i < len; i++)
    {
        if (x + xOffset * letterWidth + FONT_WIDTHS[scale] >= 155)
        {
            xOffset = 0;
            currentLine++;
        }
        if (background != TRANSPARENT)
        {
            uint8_t bufferHeight = 7 * scale + (scale - 1) * 2;
            if (currentLine == 0)
            {
                Rectangle(background, x + xOffset * letterWidth - 1, y + (FONT_HEIGHTS[scale - 1] * currentLine) - 1, letterWidth + 2, bufferHeight + 2);
            }
            else
            {
                Rectangle(background, x + xOffset * letterWidth - 1, y + (FONT_HEIGHTS[scale - 1] * currentLine), letterWidth + 2, bufferHeight + 1);
            }
        }
        WriteLetter(word[i], x + xOffset * letterWidth, y + ((FONT_HEIGHTS[scale - 1]+1) * currentLine), scale, color, background);
        xOffset++;
    }

    return currentLine+1;
}

#ifndef initPins
#define initPins
bool pinsInitialized = false;
#endif

void InitializeTFTPins()
{

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 0); // Set RST LOW
    sleep_ms(100);
    gpio_put(PIN_RST, 1); // Set RST HIGH
    sleep_ms(100);

    gpio_init(BACK_LIGHT);
    gpio_set_dir(BACK_LIGHT, GPIO_OUT);
    gpio_put(BACK_LIGHT, 1); // Turn on backlight

    spi_init(SPI_PORT, 24 * 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(TFT_CS);
    gpio_set_dir(TFT_CS, GPIO_OUT);
    gpio_put(TFT_CS, 1);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_put(PIN_DC, 0); // Set DC low for command mode

    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    pinsInitialized = true;
}

void InitializeTFT()
{
    if (!pinsInitialized)
        InitializeTFTPins();

    uint8_t numCommands = Bcmd[0];
    uint8_t cmd;
    uint8_t numArgs;
    uint16_t ms;
    uint8_t index = 1;
    while (numCommands--)
    {
        printf("Index: %u\n", index);
        cmd = Bcmd[index++];

        numArgs = Bcmd[index++];
        ms = numArgs & ST_CMD_DELAY;
        numArgs &= ~ST_CMD_DELAY;

        printf("num Args: %u\n", numArgs);

        SendCommandAndData(cmd, &Bcmd[index], numArgs);
        index += numArgs;

        if (ms)
        {
            ms = Bcmd[index++];
            if (ms == 255)
            {
                ms = 500;
            }
            sleep_ms(ms);
        }
    }

    uint8_t data = 0x60;
    SendCommandAndData(ST77XX_MADCTL, &data, 1);

    FillAll(GREEN);

    printf("Init done\n");
}

#endif // TFT