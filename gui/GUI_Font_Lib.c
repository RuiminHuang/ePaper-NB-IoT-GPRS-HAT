#include "GUI_Font_Lib.h"
#include "GUI_Paint.h"
#include <iconv.h>
#include <stdio.h>
#include <string.h>


int code_convert(char* from_charset, char* to_charset, char* inbuf, size_t inlen,char* outbuf, size_t outlen){

	iconv_t cd;  
	char **pin = &inbuf;  
	char **pout = &outbuf;  
	
	cd = iconv_open(to_charset, from_charset);
	
	if (cd == 0){
		printf("to gbk fail in format \r\n");
		return -1;
	}
	
	memset(outbuf, '\0', outlen);
	
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1){  
		printf("to gbk fail in convert \r\n");
		return -1;
	}
	
	iconv_close(cd);  
	*pout = '\0';  

	return 0;  
}


void Get_GBK_DZK(char* libPath, uint8_t *code){
	
	FILE *fd;
	
	if(NULL == (fd=fopen(libPath,"rb")))
	{
		printf("Open dir error\n");
	}
	
	uint8_t GBKH,GBKL;                 
    uint32_t offset;
	uint8_t  character[32]={0};
	uint8_t* characterPtr;
	
    GBKH=*code;
    GBKL=*(code+1);
	
    if(GBKH>0XFE||GBKH<0X81){
		return;
	}
	
    GBKH-=0x81;
    GBKL-=0x40;
	//for offset
    offset=((uint32_t)192*GBKH+GBKL)*32;
	
	
    if((-1 ==fseek(fd,offset,SEEK_SET))){
    	printf("Fseek error\n"); 
	}
	
	fread(character,1,32,fd);
	characterPtr = character;
	
	int i,j;
	for(j=0; j<16; j++){
		for(i=0; i<16; i++){
			if(*characterPtr &(0x80 >> (i % 8))){
				printf("*");
			}else{
				printf(" ");
			}
			if(i%8 == 7){
				characterPtr++;
			}
		}
		printf("\r\n");
	}
	fclose(fd);
}


int Paint_DrawString_Lib(char* libPath, char* srtUTF8,int x, int y, uint16_t frontColor, uint16_t backColor){
	
	char gbk[512];
	int k=0;
	FILE *fd;
	uint8_t GBKH,GBKL;
	uint32_t offset;
	uint8_t  character[32]={0};
	uint8_t* characterPtr;
	int i,j;
	
	int Xpoint = x;
	int Ypoint = y;
	
	if(NULL == (fd=fopen(libPath,"rb"))){
		printf("Open dir error\n");
		return -1;
	}

	//Print the original UTF-8 String to console
	#if(0)
		
	printf("print in string\r\n");
	int l=0;
	printf("%s",srtUTF8);
	printf("\r\n");

	printf("print in char\r\n");
	while(srtUTF8[l] != '\0'){
		printf("%c%c",srtUTF8[l],srtUTF8[l+1]);
		l+=2;						
	}
	printf("\r\n");
	#endif

	//Change it to GBK encode
	code_convert("UTF-8", "GBK", srtUTF8, strlen(srtUTF8), gbk, sizeof(gbk));
 
	while(gbk[k] != '\0'){
		
        //if X direction filled , reposition to(Xstart,Ypoint),Ypoint is Y direction plus the Height of the character
        if ((Xpoint + 16) > Paint.Width ) {
            Xpoint = x;
            Ypoint += 16;
        }
        // If the Y direction is full, reposition to(Xstart, Ystart)
        if ((Ypoint + 16) > Paint.Height ) {
            Xpoint = x;
            Ypoint = y;
        }
		
		GBKH=gbk[k];
		GBKL=gbk[k+1];
		if(GBKH>0XFE||GBKH<0X81){
			break;
		}
		
		//Use GBK to calculate the offset
		GBKH-=0x81;
		GBKL-=0x40;
		offset=((uint32_t)192*GBKH+GBKL)*32;
		
		if((-1 ==fseek(fd,offset,SEEK_SET))){
			printf("Fseek error\n");
			break;
		}
		fread(character,1,32,fd);
		characterPtr = character;
		
		//Show it in the ePaper
		for(j=0; j<16; j++){
			for(i=0; i<16; i++){
				if(*characterPtr &(0x80 >> (i % 8))){
					//print character to show it in screen
					//printf("*");
					Paint_SetPixel(Xpoint + i, Ypoint + j, frontColor);
				}else{
					//print character to show it in screen
					//printf(" ");
					Paint_SetPixel(Xpoint + i, Ypoint + j, backColor);
				}
				if(i%8 == 7){
					characterPtr++;
				}
			}
			//print character to show it in screen 
			//printf("\r\n");
		}
		Xpoint += 16;
		k+=2;
		//print character to show it in screen
		//printf("\r\n");
	}
	return 0;
}