#ifndef _EPD_H_
#define _EPD_H_

#include "EPD_1in54.h"
#include "EPD_1in54b.h"
#include "EPD_1in54c.h"
#include "EPD_2in7.h"
#include "EPD_2in7b.h"
#include "EPD_2in9.h"
#include "EPD_2in9bc.h"
#include "EPD_2in9d.h"
#include "EPD_2in13.h"
#include "EPD_2in13bc.h"
#include "EPD_2in13d.h"
#include "EPD_4in2.h"
#include "EPD_4in2bc.h"
#include "EPD_5in83.h"
#include "EPD_5in83bc.h"
#include "EPD_7in5.h"
#include "EPD_7in5bc.h"

extern UBYTE *BlackImage, *RedImage;

extern int EPD_dispIndex;

typedef struct 
{
	void (*Init)(void); // Init
	void (*Clear)(void); // Clear
	int Surface_Num;//The Num of ePaper surface
	void (*Display_Single)(const UBYTE *);// Display
	void (*Display_Double)(const UBYTE *, const UBYTE *);// Display
	void (*Sleep)(void);// Sleep
	char* Title;// Title of an e-Paper
	int Width;//width
	int Height;//height
	UWORD Rotate;	
}EPD_dispInfo;

extern EPD_dispInfo EPD_dispMass[];

void EPD_Init();

void EPD_Clear();

void EPD_Display_Single(UBYTE *Imageblack);

void EPD_Display_Double(UBYTE *Imageblack, UBYTE *Imagered);

void EPD_Sleep();
#endif
