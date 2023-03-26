#include "LPC17xx.h"
#include "display.h"
#include "arial12x12.h"

/*********************** Hardware specific configuration **********************/

/* SPI Interface: SPI3
   PINS:
   - CS     = P0.6 (GPIO pin)
   - RS     = GND
   - WR/SCK = P0.7 (SCK1)
   - RD     = GND
   - SDO    = P0.8 (MISO1)
   - SDI    = P0.9 (MOSI1)                                                    */

#define PIN_CS      (1 << 6)

/* SPI_SR - bit definitions                                                   */
#define TFE         0x01
#define RNE         0x04
#define BSY         0x10

/*------------------------- Speed dependant settings -------------------------*/

/* If processor works on high frequency delay has to be increased, it can be
   increased by factor 2^N by this constant                                   */
#define DELAY_2N    18

/*---------------------- Graphic LCD size definitions ------------------------*/

#define WIDTH       320                 /* Screen Width (in pixels)           */
#define HEIGHT      240                 /* Screen Hight (in pixels)           */
#define BPP         16                  /* Bits per pixel                     */
#define BYPP        ((BPP+7)/8)         /* Bytes per pixel                    */

/*--------------- Graphic LCD interface hardware definitions -----------------*/

/* Pin CS setting to 0 or 1                                                   */
#define LCD_CS(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_CS) : (LPC_GPIO0->FIOCLR = PIN_CS));

#define SPI_START   (0x70)              /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)              /* WR bit 1 within start              */
#define SPI_WR      (0x00)              /* WR bit 0 within start              */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte         */

/*---------------------------- Global variables ------------------------------*/

/******************************************************************************/
static volatile unsigned short TextColor = White, BackColor = Black;


/************************ Local auxiliary functions ***************************/

/*******************************************************************************
* Delay in while loop cycles                                                   *
*   Parameter:    cnt:    number of while cycles to delay                      *
*   Return:                                                                    *
*******************************************************************************/

static void delay (int cnt) 
{
  cnt <<= DELAY_2N;
  while (cnt--);
}


/*******************************************************************************
* Send 1 byte over serial communication                                        *
*   Parameter:    byte:   byte to be sent                                      *
*   Return:               byte read while sending                              *
*******************************************************************************/

static __INLINE unsigned char spi_send (unsigned char byte) {

  LPC_SSP1->DR = byte;
  while (!(LPC_SSP1->SR & RNE));        /* Wait for send to finish            */
  return (LPC_SSP1->DR);
}


/*******************************************************************************
* Write command to LCD controller                                              *
*   Parameter:    c:      command to be written                                *
*   Return:                                                                    *
*******************************************************************************/

static __INLINE void wr_cmd (unsigned char c) {

  LCD_CS(0)
  spi_send(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
  spi_send(0);
  spi_send(c);
  LCD_CS(1)
}


/*******************************************************************************
* Write data to LCD controller                                                 *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static __INLINE void wr_dat (unsigned short c) {

  LCD_CS(0)
  spi_send(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
  spi_send((c >>   8));                       /* Write D8..D15                */
  spi_send((c & 0xFF));                       /* Write D0..D7                 */
  LCD_CS(1)
}


/*******************************************************************************
* Start of data writing to LCD controller                                      *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

static __INLINE void wr_dat_start (void) {

  LCD_CS(0)
  spi_send(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
}


/*******************************************************************************
* Stop of data writing to LCD controller                                       *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

static __INLINE void wr_dat_stop (void) {

  LCD_CS(1)
}


/*******************************************************************************
* Data writing to LCD controller                                               *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static __INLINE void wr_dat_only (unsigned short c) {

  spi_send((c >>   8));                       /* Write D8..D15                */
  spi_send((c & 0xFF));                       /* Write D0..D7                 */
}


/*******************************************************************************
* Read data from LCD controller                                                *
*   Parameter:                                                                 *
*   Return:               read data                                            *
*******************************************************************************/

static __INLINE unsigned short rd_dat (void) {
  unsigned short val = 0;

  LCD_CS(0)
  spi_send(SPI_START | SPI_RD | SPI_DATA);    /* Read: RS = 1, RW = 1         */
  spi_send(0);                                /* Dummy read                   */
  val   = spi_send(0);                        /* Read D8..D15                 */
  val <<= 8;
  val  |= spi_send(0);                        /* Read D0..D7                  */
  LCD_CS(1)
  return (val);
}

/*******************************************************************************
* Write to LCD register                                                        *
*   Parameter:    reg:    register to be read                                  *
*                 val:    value to write to register                           *
*******************************************************************************/

static __INLINE void wr_reg (unsigned char reg, unsigned short val) {

  wr_cmd(reg);
  wr_dat(val);
}


/*******************************************************************************
* Read from LCD register                                                       *
*   Parameter:    reg:    register to be read                                  *
*   Return:               value read from register                             *
*******************************************************************************/

static unsigned short rd_reg (unsigned char reg) {

  wr_cmd(reg);
  return (rd_dat());
}


/************************ Exported functions **********************************/

/*******************************************************************************
* Initialize the Graphic LCD controller                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Init (void) {
  static unsigned short driverCode;

  /* Enable clock for SSP1, clock = CCLK / 2                                  */
  LPC_SC->PCONP       |= 0x00000400;
  LPC_SC->PCLKSEL0    |= 0x00200000;

  /* Configure the LCD Control pins                                           */
  LPC_PINCON->PINSEL9 &= 0xF0FFFFFF;
  LPC_GPIO4->FIODIR   |= 0x30000000;
  LPC_GPIO4->FIOSET    = 0x20000000;

  /* SSEL1 is GPIO output set to high                                         */
  LPC_GPIO0->FIODIR   |= 0x00000040;
  LPC_GPIO0->FIOSET    = 0x00000040;
  LPC_PINCON->PINSEL0 &= 0xFFF03FFF;
  LPC_PINCON->PINSEL0 |= 0x000A8000;

  /* Enable SPI in Master Mode, CPOL=1, CPHA=1                                */
  /* Max. 25 MBit used for Data Transfer @ 100MHz                              */
  LPC_SSP1->CR0        = 0xC7;
  LPC_SSP1->CPSR       = 0x02;
  LPC_SSP1->CR1        = 0x02;

  delay(5);                             /* Delay 50 ms                        */
  driverCode = rd_reg(0x00);

  /* Start Initial Sequence --------------------------------------------------*/
    wr_reg(0xEA, 0x00);                 /* Power control internal used (1)    */
    wr_reg(0xEB, 0x20);                 /* Power control internal used (2)    */
    wr_reg(0xEC, 0x0C);                 /* Source control internal used (1)   */
    wr_reg(0xED, 0xC7);                 /* Source control internal used (2)   */
    wr_reg(0xE8, 0x38);                 /* Source output period Normal mode   */
    wr_reg(0xE9, 0x10);                 /* Source output period Idle mode     */
    wr_reg(0xF1, 0x01);                 /* RGB 18-bit interface ;0x0110       */
    wr_reg(0xF2, 0x10);

    /* Adjust the Gamma Curve ------------------------------------------------*/
    wr_reg(0x40, 0x01);
    wr_reg(0x41, 0x00);
    wr_reg(0x42, 0x00);
    wr_reg(0x43, 0x10);
    wr_reg(0x44, 0x0E);
    wr_reg(0x45, 0x24);
    wr_reg(0x46, 0x04);
    wr_reg(0x47, 0x50);
    wr_reg(0x48, 0x02);
    wr_reg(0x49, 0x13);
    wr_reg(0x4A, 0x19);
    wr_reg(0x4B, 0x19);
    wr_reg(0x4C, 0x16);

    wr_reg(0x50, 0x1B);
    wr_reg(0x51, 0x31);
    wr_reg(0x52, 0x2F);
    wr_reg(0x53, 0x3F);
    wr_reg(0x54, 0x3F);
    wr_reg(0x55, 0x3E);
    wr_reg(0x56, 0x2F);
    wr_reg(0x57, 0x7B);
    wr_reg(0x58, 0x09);
    wr_reg(0x59, 0x06);
    wr_reg(0x5A, 0x06);
    wr_reg(0x5B, 0x0C);
    wr_reg(0x5C, 0x1D);
    wr_reg(0x5D, 0xCC);

    /* Power voltage setting -------------------------------------------------*/
    wr_reg(0x1B, 0x1B);
    wr_reg(0x1A, 0x01);
    wr_reg(0x24, 0x2F);
    wr_reg(0x25, 0x57);
    wr_reg(0x23, 0x88);

    /* Power on setting ------------------------------------------------------*/
    wr_reg(0x18, 0x36);                 /* Internal oscillator frequency adj  */
    wr_reg(0x19, 0x01);                 /* Enable internal oscillator         */
    wr_reg(0x01, 0x00);                 /* Normal mode, no scrool             */
    wr_reg(0x1F, 0x88);                 /* Power control 6 - DDVDH Off        */
    delay(20);
    wr_reg(0x1F, 0x82);                 /* Power control 6 - Step-up: 3 x VCI */
    delay(5);
    wr_reg(0x1F, 0x92);                 /* Power control 6 - Step-up: On      */
    delay(5);
    wr_reg(0x1F, 0xD2);                 /* Power control 6 - VCOML active     */
    delay(5);

    /* Color selection -------------------------------------------------------*/
    wr_reg(0x17, 0x55);                 /* RGB, System interface: 16 Bit/Pixel*/
    wr_reg(0x00, 0x00);                 /* Scrolling off, no standby          */

    /* Interface config ------------------------------------------------------*/
    wr_reg(0x2F, 0x11);                 /* LCD Drive: 1-line inversion        */
    wr_reg(0x31, 0x00);
    wr_reg(0x32, 0x00);                 /* DPL=0, HSPL=0, VSPL=0, EPL=0       */

    /* Display on setting ----------------------------------------------------*/
    wr_reg(0x28, 0x38);                 /* PT(0,0) active, VGL/VGL            */
    delay(20);
    wr_reg(0x28, 0x3C);                 /* Display active, VGL/VGL            */

    wr_reg (0x16, 0x08);

    /* Display scrolling settings --------------------------------------------*/

    
    LPC_GPIO4->FIOSET = 0x10000000;
}

void GLCD_Display_On (void)
{
    wr_reg(0x0007, 0x0173);
}    

void GLCD_Display_Off (void)
{
    wr_reg(0x0007, 0x0000);
}   

void GLCD_SetPosition(unsigned int x, unsigned int y)
{
    wr_reg(0x02, x >> 8);               /* Column address start MSB           */
    wr_reg(0x03, x & 0xFF);             /* Column address start LSB           */
    wr_reg(0x04, x >> 8);               /* Column address end MSB             */
    wr_reg(0x05, x & 0xFF);             /* Column address end LSB             */

    wr_reg(0x06, y >> 8);               /* Row address start MSB              */
    wr_reg(0x07, y & 0xFF);             /* Row address start LSB              */
    wr_reg(0x08, y >> 8);               /* Row address end MSB                */
    wr_reg(0x09, y & 0xFF);             /* Row address end LSB                */
}

/*******************************************************************************
* Set draw window region to whole screen                                       *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_WindowMax (void) {

  wr_reg(0x50, 0);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, HEIGHT-1);               /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, 0);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, WIDTH-1);                /* Vertical   GRAM End   Address (-1) */
}


/*******************************************************************************
* Draw a pixel in foreground color                                             *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_PutPixel (unsigned int x, unsigned int y) 
{
  GLCD_SetPosition(x, y);
  wr_cmd(0x22);
  wr_dat(TextColor);
}


/*******************************************************************************
* Set foreground color                                                         *
*   Parameter:      color:    foreground color                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_SetTextColor (unsigned short color) {

  TextColor = color;
}


/*******************************************************************************
* Set background color                                                         *
*   Parameter:      color:    background color                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_SetBackColor (unsigned short color) {

  BackColor = color;
}


/*******************************************************************************
* Clear display                                                                *
*   Parameter:      color:    display clearing color                           *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Clear (unsigned short color) {
  unsigned int TextColorTemp = TextColor;

    TextColor = color;
  for (int i = 0; i < WIDTH; i++)
    { 
         for (int j = 0; j < HEIGHT; j++)
        {
            GLCD_PutPixel(j, i);
        } 
    }
    TextColor = TextColorTemp;
}


/*******************************************************************************
* Draw character on given position                                             *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   c:        pointer to character bitmap                      *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_DrawChar (unsigned int x, unsigned int y, unsigned short *c) {
  int idx = 0, i, j;

  //x = WIDTH-x-CHAR_W;
  //wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  //wr_reg(0x51, y+CHAR_H-1);             /* Horizontal GRAM End   Address (-1) */
  //wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  //wr_reg(0x53, x+CHAR_W-1);             /* Vertical   GRAM End   Address (-1) */

    wr_reg(0x20, x);
    wr_reg(0x21, y);
  //wr_cmd(0x22);
  //wr_dat_start();
  for (j = 0; j < CHAR_H; j++) {
    for (i = CHAR_W-1; i >= 0; i--) {
      if((c[idx] & (1 << i)) == 0x00) {
        //wr_dat_only(BackColor);
      } else {
        GLCD_PutPixel(x + i, y + j);
      }
    }
    c++;
  }
  //wr_dat_stop();
}


/*******************************************************************************
* Disply character on given line                                               *
*   Parameter:      ln:       line number                                      *
*                   col:      column number                                    *
*                   c:        ascii character                                  *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_DisplayChar (unsigned int ln, unsigned int col, unsigned char c) {

  c -= 32;
  GLCD_DrawChar(col * CHAR_W, ln * CHAR_H, (unsigned short *)&Font_24x16[c * CHAR_H]);
}


/*******************************************************************************
* Disply string on given line                                                  *
*   Parameter:      ln:       line number                                      *
*                   col:      column number                                    *
*                   s:        pointer to string                                *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_DisplayString (unsigned int ln, unsigned int col, unsigned char *s) {

  GLCD_WindowMax();
  while (*s) {
    GLCD_DisplayChar(ln, col++, *s++);
  }
}


/*******************************************************************************
* Clear given line                                                             *
*   Parameter:      ln:       line number                                      *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_ClearLn (unsigned int ln) {

  GLCD_WindowMax();
  GLCD_DisplayString(ln, 0, "                    ");
}

/*******************************************************************************
* Draw bargraph                                                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        maximum width of bargraph (in pixels)            *
*                   val:      value of active bargraph (in 1/1024)             *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Bargraph (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int val) {
  int i,j;

  x = WIDTH-x-w;
  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+CHAR_H-1);             /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

  val = (val * w) >> 10;                /* Scale value for 24x12 characters   */
    wr_reg(0x20, x);
    wr_reg(0x21, y);
  wr_cmd(0x22);
  wr_dat_start();
  for (i = 0; i < h; i++) {
    for (j = w-1; j >= 0; j--) {
      if(j >= val) {
        wr_dat_only(BackColor);
      } else {
        wr_dat_only(TextColor);
      }
    }
  }
  wr_dat_stop();
}


/*******************************************************************************
* Display graphical bitmap image at position x horizontally and y vertically   *
* (This function is optimized for 16 bits per pixel format, it has to be       *
*  adapted for any other bits per pixel format)                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        width of bitmap                                  *
*                   h:        height of bitmap                                 *
*                   bitmap:   address at which the bitmap data resides         *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Bitmap (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap) {
  unsigned int    i, j;
  unsigned short *bitmap_ptr = (unsigned short *)bitmap;

  x = WIDTH-x-w;
  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+h-1);                  /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

    wr_reg(0x20, x);
    wr_reg(0x21, y);
  wr_cmd(0x22);
  wr_dat_start();
  for (j = 0; j < h; j++) {
    bitmap_ptr += w-1;
    for (i = 0; i < w; i++) {
      wr_dat_only(*bitmap_ptr--);
    }
    bitmap_ptr += w+1;
  }
  wr_dat_stop();
}


/*******************************************************************************
* Display graphical bmp file image at position x horizontally and y vertically *
* (This function is optimized for 16 bits per pixel format, it has to be       *
*  adapted for any other bits per pixel format)                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        width of bitmap                                  *
*                   h:        height of bitmap                                 *
*                   bmp:      address at which the bmp data resides            *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Bmp (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bmp) {
  unsigned int    i, j;
  unsigned short *bitmap_ptr = (unsigned short *)bmp;

  x = WIDTH-x-w;
  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+h-1);                  /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

    wr_reg(0x20, x);
    wr_reg(0x21, y);
  wr_cmd(0x22);
  wr_dat_start();
  bitmap_ptr += (h*w)-1;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      wr_dat_only(*bitmap_ptr--);
    }
  }
  wr_dat_stop();
}