#include "EPD.h"


UBYTE *BlackImage, *RedImage;

int EPD_dispIndex;


EPD_dispInfo EPD_dispMass[]=
{
	{ EPD_1IN54_Init,     EPD_1IN54_Clear,    1,  EPD_1IN54_Display,         NULL,                   EPD_1IN54_Sleep,    "1.54 inch",     200,200,   270},//0
	{ EPD_1IN54B_Init,    EPD_1IN54B_Clear,   2,  NULL,                      EPD_1IN54B_Display,     EPD_1IN54B_Sleep,   "1.54 inch b",   200,200,   270},//1
	{ EPD_1IN54C_Init,    EPD_1IN54C_Clear,   1,  NULL,                      EPD_1IN54C_Display,     EPD_1IN54C_Sleep,   "1.54 inch c",   200,200,   270},//2

	{ EPD_2IN13_Init ,    EPD_2IN13_Clear,    1,  EPD_2IN13_Display,         NULL,                   EPD_2IN13_Sleep,    "2.13 inch",     122,250,   270},//3

	{ EPD_2IN13BC_Init ,  EPD_2IN13BC_Clear,  2,  NULL,                      EPD_2IN13BC_Display,    EPD_2IN13BC_Sleep,  "2.13 inch b_c", 104,212,   270},//4
	{ EPD_2IN13D_Init ,   EPD_2IN13D_Clear,   1,  EPD_2IN13D_Display,        NULL,                   EPD_2IN13D_Sleep,   "2.13 inch d",   104,212,   270},//5
	{ EPD_2IN7_Init ,     EPD_2IN7_Clear,     1,  EPD_2IN7_Display,          NULL,                   EPD_2IN7_Sleep,     "2.7 inch",      176,264,   270},//6
    { EPD_2IN7B_Init ,    EPD_2IN7B_Clear,    2,  NULL,                      EPD_2IN7B_Display,      EPD_2IN7B_Sleep,    "2.7 inch b",    176,264,   270},//7
	{ EPD_2IN9_Init ,     EPD_2IN9_Clear,     1,  EPD_2IN9_Display,          NULL,                   EPD_2IN9_Sleep,     "2.9 inch",      128,296,   270},//8
	{ EPD_2IN9BC_Init ,   EPD_2IN9BC_Clear,   2,  NULL,                      EPD_2IN9BC_Display,     EPD_2IN9BC_Sleep,   "2.9 inch b_c",  128,296,   270},//9

	{ EPD_2IN9D_Init ,    EPD_2IN9D_Clear,    1,  EPD_2IN9D_Display,         NULL,                   EPD_2IN9D_Sleep,    "2.9 inch d",    128,296,   270},//10
	
	{ EPD_4IN2_Init ,     EPD_4IN2_Clear,     1,  EPD_4IN2_Display,          NULL,                   EPD_4IN2_Sleep,     "4.2 inch",      400,300,   180},//11
	{ EPD_4IN2BC_Init ,   EPD_4IN2BC_Clear,   2,  NULL,                      EPD_4IN2BC_Display,     EPD_4IN2BC_Sleep,   "4.2 inch b_c",  400,300,   180},//12
	{ EPD_5IN83_Init ,    EPD_5IN83_Clear,    1,  EPD_5IN83_Display,         NULL,                   EPD_5IN83_Sleep,    "5.83 inch",     600,448,   180},//13
	{ EPD_5IN83BC_Init ,  EPD_5IN83BC_Clear,  2,  NULL,                      EPD_5IN83BC_Display,    EPD_5IN83BC_Sleep,  "5.83 inch b_c", 600,448,   0},//14
	{ EPD_7IN5_Init ,     EPD_7IN5_Clear,     1,  EPD_7IN5_Display,          NULL,                   EPD_7IN5_Sleep,     "7.5 inch",      640,384,   0},//15
	{ EPD_7IN5BC_Init ,   EPD_7IN5BC_Clear,   2,  NULL,                      EPD_7IN5BC_Display,     EPD_7IN5BC_Sleep,   "7.5 inch b_c",  640,384,   0}//16
};


void EPD_Init()
{
	EPD_dispMass[EPD_dispIndex].Init();
}

void EPD_Clear()
{
	EPD_dispMass[EPD_dispIndex].Clear();
}

void EPD_Display_Single(UBYTE *Imageblack)
{
	EPD_dispMass[EPD_dispIndex].Display_Single(Imageblack);
}

void EPD_Display_Double(UBYTE *Imageblack, UBYTE *Imagered)
{
	EPD_dispMass[EPD_dispIndex].Display_Double(Imageblack,Imagered);
}

void EPD_Sleep()
{
	EPD_dispMass[EPD_dispIndex].Sleep();
}