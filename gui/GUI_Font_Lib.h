#ifndef __GUI_FONT_LIB_H__
#define __GUI_FONT_LIB_H__

#include <stdlib.h>
#include <stdint.h>

int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,char *outbuf, size_t outlen);
void Get_GBK_DZK(char* libPath, uint8_t *code);
int Paint_DrawString_Lib(char* libPath, char* srtUTF8,int x, int y, uint16_t frontColor, uint16_t backColor);

#endif