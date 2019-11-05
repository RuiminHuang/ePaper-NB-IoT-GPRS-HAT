#include <signal.h>//signal()
#include <stdlib.h>//exit()

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <bcm2835.h>

#include <time.h>

#include "../ePapers/EPD.h"

#include "../gui/GUI_Paint.h"
#include "../gui/GUI_BMPfile.h"
#include "../gui/ImageData.h"
#include "../gui/GUI_Paint.h"
#include "../gui/GUI_Font_Lib.h"

#include "../drive/serial.h"

#include "../sim7000/sim7000-basic.h"
#include "../sim7000/sim7000-http.h"
#include "../sim7000/sim7000-mqtt.h"

#include "../json/cJSON.h"

#include "../drive/serial.h"

#include "../log/log.h"


int tryTime, successTime,mqttConnectFailTime, httpDownloadFailTime,duplicatePackageTime,noneMqttPackageTime;
FILE* pFile;

UBYTE *BlackImage, *RedImage;

int ePaperInit(int dispIndex){

	EPD_dispIndex = dispIndex;
	int EPD_WIDTH = EPD_dispMass[EPD_dispIndex].Width;
	int EPD_HEIGHT = EPD_dispMass[EPD_dispIndex].Height;
	int ROTATE = EPD_dispMass[EPD_dispIndex].Rotate;

    EPD_Init();

    //Create a new image cache named IMAGE_BW and fill it with white
    UWORD Imagesize = ((EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1)) * EPD_HEIGHT;
	
	printf("NewImage:BlackImage\r\n");
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    Paint_NewImage(BlackImage, EPD_WIDTH, EPD_HEIGHT, ROTATE, WHITE);
	Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
	
	printf("NewImage:RedImage\r\n");
	if(EPD_dispMass[EPD_dispIndex].Surface_Num == 2){
		if((RedImage = (UBYTE *)malloc(Imagesize)) == NULL) {
			printf("Failed to apply for red memory...\r\n");
			return -2;
		}
		Paint_NewImage(RedImage, EPD_WIDTH, EPD_HEIGHT, ROTATE, WHITE);
		Paint_SelectImage(RedImage);
		Paint_Clear(WHITE);
	}
	return 0;
}


void ePaperDeinit(void){
	
	free(BlackImage);
    BlackImage = NULL;
	
	if(EPD_dispMass[EPD_dispIndex].Surface_Num == 2){
		free(RedImage);
		RedImage = NULL;
	}
	EPD_Sleep();
}




int sim7000Init(void){
	
	beginSerial(115200);
	
	if(powerOn(4)==false){
		return 1;
	}
	
	return 0;
}


//Ctrl+c or exit
void Handler(int signo){

	//let ePaper goes to sleep first
	printf("\r\n");
    printf("\033[1;36;40mePaper goes to Sleep mode\033[0m\r\n");
    EPD_Sleep();
    DEV_ModuleExit();
	
	//file for log
	fclose(pFile);
	
	//disconnect connect from Aliyun IoT Platform
	//DisconnectAliyun();
	
	//free the buffer
	free(BlackImage);
    BlackImage = NULL;
	if(EPD_dispMass[EPD_dispIndex].Surface_Num == 2){
		free(RedImage);
		RedImage = NULL;
	}
	
    exit(0);
}



char ack[2048];
char response;
int replyStart=0;
int setStart=0;
int length=0;
uint16_t i=0;

//Aliyun IoT Platform can transform max 256K byte for every packet
char buffer[256*1024];


typedef struct{
	int onOff;  
	int x;
	int y;
	int pixel;
}Point;

typedef struct{
	int onOff;
	int x1;
	int y1;
	int x2;
	int y2;
	int lStl;
	int pixel;
}Line;
typedef struct{
	int onOff;
	int x;
	int y;
	int w;
	int h;
	int fill;
	int pixel;
}Rec;
typedef struct
{
	int onOff;
	int x;
	int y;
	int r;
	int fill;
	int pixel;
}Circle;
typedef struct
{
	int onOff;
	char picPath[512];
}Pic;
typedef struct
{
	int onOff;
	int x;
	int y;
	int font;
	int backColor;
	int frontColor;
	char str[512];
}Str;

int rfType,scrType;
Point point;
Line line;
Circle circle;
Rec rec;
Pic pic1,pic2;
Str strCn,strEn;


int refreshScreen(void)
{
	int i,rslt;
	
	pFile = fopen("./log/log.txt", "a");
	
	rslt = ePaperInit(scrType);
	if(rslt != 0){
		printf("Init %s fail\r\n",EPD_dispMass[EPD_dispIndex].Title);
		return 1;
	}else{
		printf("Init %s success\r\n",EPD_dispMass[EPD_dispIndex].Title);
	}
	
	//picture model
	if(rfType==2){
		
		//file from local
		if(pic1.onOff==1){
			
			Paint_SelectImage(BlackImage);
			printf("Refreshing black pic now...\r\n");	
			//try 5 times to download file with reset
			for(i=0; i<5; i++){
				rslt = httpConnect(pic1.picPath);
				if(rslt == 0){
					printf("\033[1;40;35m---download picture success!!!---\033[0m\r\n");
					GUI_ReadBmp("./PIC/wireless-pic.bmp", 0, 0);
					break;
				}
				else if(rslt == 12){
					if(i != 5-1){
						httpDownloadFailTime += 1;
						printf("Reset to download file time %d fail, will reset and try again\r\n",i+1);
					}else{
						printf("Download file fail, the NB-IoT network is busy,Please try it next time\r\n");
						//Try 5 times, still fail,The problem in network
						return 3;
					}
				}else{
					printf("Fail!!! the error code is %d\r\n",rslt);
					printf("Connect to Aliyun fail，Please check your SIM7000C Module or SIM Card works well\r\n");
					//The problem in Module
					return 3;
				}
			}
			
			write_log(pFile, "black-white picture from http\r\n");
		}
		
		if(EPD_dispMass[EPD_dispIndex].Surface_Num == 2){
			if(pic2.onOff==1){
				Paint_SelectImage(RedImage);
				printf("Refreshing other pic now...\r\n");
				//try 5 times to download file with reset
				for(i=0; i<5; i++){
					rslt = httpConnect(pic2.picPath);
					if(rslt == 0){
						printf("\033[1;40;35m---download picture success!!!---\033[0m\r\n");
						GUI_ReadBmp("./PIC/wireless-pic.bmp", 0, 0);
						break;
					}
					else if(rslt == 12){
						if(i != 5-1){
							httpDownloadFailTime += 1;
							printf("Reset to download file time %d fail, will reset and try again\r\n",i+1);
						}else{
							printf("Download file fail, the NB-IoT network is busy,Please try it next time\r\n");
							//Try 5 times, still fail,The problem in network
							return 3;
						}
					}else{
						printf("Fail!!! the error code is %d\r\n",rslt);
						printf("Connect to Aliyun fail，Please check your SIM7000C Module or SIM Card works well\r\n");
						//The problem in Module
						return 3;
					}
				}
				write_log(pFile, "other-white picture from http\r\n");
			}
		}

		Paint_SelectImage(BlackImage);
		
		printf("Refreshing data now...\r\n");
		
		if(point.onOff==1){
			Paint_DrawPoint(point.x,point.y,BLACK, point.pixel, DOT_STYLE_DFT);
		}

		if(line.onOff==1){
			
			Paint_DrawLine(line.x1,line.y1,line.x2,line.y2,BLACK, line.lStl-1, line.pixel);
		}

		if(rec.onOff==1){
			Paint_DrawRectangle(rec.x, rec.y, rec.x+rec.w, rec.y+rec.h, BLACK, rec.fill-1, rec.pixel);
		}
		if(circle.onOff==1){
			Paint_DrawCircle(circle.x,circle.y,circle.r,BLACK, circle.fill-1, circle.pixel);
		}
		if(strCn.onOff==1){
			
			char backColor,frontColor;
			if(strCn.backColor==1){
				backColor = WHITE;
			}else{
				backColor= BLACK;
			}
			if(strCn.frontColor==1){
				frontColor = BLACK;
			}else{
				frontColor = WHITE;
			}
			
			Paint_DrawString_Lib("./fonts/GBK.bin", strCn.str ,strCn.x, strCn.y, frontColor, backColor);
		}
		if(strEn.onOff==1){
			char backColor,frontColor;
			sFONT* Font = &Font8;
			if(strEn.backColor==1){
				backColor = WHITE;
			}else{
				backColor= BLACK;
			}
			if(strEn.frontColor==1){
				frontColor = BLACK;
			}else{
				frontColor = WHITE;
			}
			switch(strEn.font){
				case 1: Font = &Font8; break;
				case 2: Font = &Font12; break;
				case 3: Font = &Font16; break;
				case 4: Font = &Font20; break;
				case 5: Font = &Font24; break;
				default: break;
			}
			Paint_DrawString_EN(strEn.x, strEn.y, strEn.str, Font, backColor, frontColor);
		}
		
		//clear epaper for refresh
		printf("clear...\r\n");
		EPD_Clear();
		DEV_Delay_ms(500);
		
		if(EPD_dispMass[EPD_dispIndex].Surface_Num == 2){
				EPD_Display_Double(BlackImage, RedImage);
				write_log(pFile, "BlackImage & RedImage display success\r\n");
				successTime +=1;
				printf("Refresh epaper finish\r\n");
		}
		
		if(EPD_dispMass[EPD_dispIndex].Surface_Num == 1){
				EPD_Display_Single(BlackImage);
				write_log(pFile, "RedImage display success\r\n");
				successTime +=1;
				printf("Refresh epaper finish\r\n");
		}

	}
	
	//refresh model
	if(rfType==3){
		//clear the screen
		EPD_Clear();
		write_log(pFile, "clear screen\r\n");
		
		printf("Refresh epaper finish\r\n");
		successTime +=1;
	}
	
	printf("tryTime: %d\r\n",tryTime);
	printf("successTime: %d\r\n",successTime);
	printf("mqttConnectFailTime: %d\r\n",mqttConnectFailTime);
	printf("httpDownloadFailTime: %d\r\n",httpDownloadFailTime);
	printf("duplicatePackageTime: %d\r\n",duplicatePackageTime);
	printf("noneMqttPackageTime: %d\r\n",noneMqttPackageTime);
	write_log(pFile, "tryTime:%d,  successTime:%d,  mqttConnectFailTime:%d,  httpDownloadFailTime:%d,  duplicatePackageTime:%d,  noneMqttPackageTime:%d\r\n", tryTime,successTime,mqttConnectFailTime,httpDownloadFailTime,duplicatePackageTime,noneMqttPackageTime);

	ePaperDeinit();
	fclose(pFile);
	return 0;
}


unsigned long previousTime;

int sim7000ReturnInitState(){
	tryTime += 1;
	int i,ret;
	printf("SIM7000 return to init state\r\n");
	//try 5 time to connect to Aliyun IoT Platform
	for(i=0; i<5; i++){
		ret = MQTTConnectToAliyun();
		if(ret == 0){
			printf("\033[1;40;35m---connect to Aliyun success!!!---\033[0m\r\n");
			break;
		}
		else if(ret == 19){
			if(i != 5-1){
				mqttConnectFailTime += 1;
				printf("Reset to connect to Aliyun IoT platform time %d fail, will reset and try again\r\n",i+1);
			}else{
				printf("Connect to Aliyun fail, the NB-IoT network is busy,Please try it next time\r\n");
				//Try 5 times, still fail,The problem in network
				return 3;
			}
		}else{
			printf("Fail!!! the error code is %d\r\n",ret);
			printf("Connect to Aliyun fail，Please check your SIM7000C Module or SIM Card is work well\r\n");
			//The problem in Module
			return 3;
		}
	}	
	replyStart = 0;
	subPartRFType();
	previousTime = millis();
	
	return 0;
}

//When serial in idle state, serialIdleFlag change to exact value
//When serial don't in idle state, serialIdleFlag change to -1
int serialIdleFlag = -1;

void loop(void)
{
	if(availableSerialByte() != 0){
		
		serialIdleFlag = -1;
        response = readSerial();
		
		//To view the serial response data
		//printf("\033[1;36;40m%c\033[0m",response);

		if(replyStart==0)
		{
			ack[i++] = response;
			if(strstr(ack,"/desired/get_reply\",\"{") !=NULL){
				printf("\033[1;40;35m/desired/get_reply\",\" checked\033[0m\r\n");
				replyStart = 1;
				memset(ack,'\0',sizeof(ack));
				i=0;
			}else{
				serialIdleFlag = 0;
			}
		}
		
		
		if(replyStart==1)
		{
			buffer[length++]=response;
			//printf("%c",response);
			if(response=='\n'){
							
				/* /r  /n   /"  totally 3 items */
				buffer[length-3]='\0';
				length=0;
				printf("%s",buffer);
				printf("\r\n");
				
				//start to parse the json data
				cJSON *pJsonBuffer = cJSON_Parse(buffer);
				if(NULL == pJsonBuffer){
					printf("Parse fail\r\n"); 
				}
				
				cJSON * pSubData = cJSON_GetObjectItem(pJsonBuffer, "data");
				if(NULL == pSubData){
					printf("There is no data node in json\r\n");
				}
				
				
				/*parse rfType*/
				cJSON *pSubDataRfType = cJSON_GetObjectItem(pSubData, "rfType");
				cJSON *pSubDataRfTypeValue = cJSON_GetObjectItem(pSubDataRfType, "value");
				printf("rfType:%d\r\n", pSubDataRfTypeValue->valueint);
				rfType = pSubDataRfTypeValue->valueint;

				/*parse scrType*/
				cJSON *pSubDataScrType = cJSON_GetObjectItem(pSubData, "scrType");
				cJSON *pSubDataScrTypeValue = cJSON_GetObjectItem(pSubDataScrType, "value");
				printf("scrType:%d\r\n", pSubDataScrTypeValue->valueint);
				scrType = pSubDataScrTypeValue->valueint;
				
				
				cJSON_Delete(pJsonBuffer);
				memset(buffer,'\0',sizeof(buffer));	
				
				//just clean the screen
				if(rfType==3)
				{
					//wait for the next time to refresh the screen,this is finally step
					replyStart = 10;
					refreshScreen();
				}
				//just refresh the screen with picture
				else if(rfType==2)
				{
					//get the picture only
					replyStart = 2;
					
					//delay for 3 second and empty the serial data,which for avoiding duplicate package rush into next step
					bcm2835_delay(5000);
					while( availableSerialByte() > 0){
						duplicatePackageTime += 1;
						readSerial();
					}
					
					//To get the picture address
					subPicAddr();
					previousTime = millis();
				}
			}
		}



		if(replyStart==2)
		{
			ack[i++] = response;
			if(strstr(ack,"/desired/get_reply\",\"{") !=NULL){
				printf("\033[1;40;35m/desired/get_reply\",\" checked\033[0m\r\n");
				replyStart = 3;
				memset(ack,'\0',sizeof(ack));
				i=0;
			}else{
				serialIdleFlag = 2;
			}
		}


		if(replyStart==3)
		{
			buffer[length++]=response;
			//printf("%c",response);
			if(response=='\n'){
				replyStart=4;
				/* /r  /n   /"  totally 3 items */
				buffer[length-3]='\0';
				length=0;
				printf("%s",buffer);
				printf("\r\n");
				
				//start to parse the json data
				cJSON *pJsonBuffer = cJSON_Parse(buffer);
				if(NULL == pJsonBuffer){
					printf("Parse fail\r\n"); 
				}
				
				cJSON * pSubData = cJSON_GetObjectItem(pJsonBuffer, "data");
				if(NULL == pSubData){
					printf("There is no data node in json\r\n");
				}
								
				/*parse pic1*/
				cJSON *pSubDataPic1 = cJSON_GetObjectItem(pSubData, "pic1");
				cJSON *pSubDataPicValue1 = cJSON_GetObjectItem(pSubDataPic1, "value");
				
				cJSON *pSubDataPic1OnOff = cJSON_GetObjectItem(pSubDataPicValue1, "onOff");
				printf("pic1_onOff:%d\r\n", pSubDataPic1OnOff->valueint);
				pic1.onOff = pSubDataPic1OnOff->valueint;
				
				cJSON *pSubDataPic1PicPath = cJSON_GetObjectItem(pSubDataPicValue1, "picPath");
				printf("pic1_picPath:%s\r\n", pSubDataPic1PicPath->valuestring);
				strcpy(pic1.picPath,pSubDataPic1PicPath->valuestring);
				
				/*parse pic2*/
				cJSON *pSubDataPic2 = cJSON_GetObjectItem(pSubData, "pic2");
				cJSON *pSubDataPicValue2 = cJSON_GetObjectItem(pSubDataPic2, "value");
				
				cJSON *pSubDataPic2OnOff = cJSON_GetObjectItem(pSubDataPicValue2, "onOff");
				printf("pic2_onOff:%d\r\n", pSubDataPic2OnOff->valueint);
				pic2.onOff = pSubDataPic2OnOff->valueint;
				
				cJSON *pSubDataPic2PicPath = cJSON_GetObjectItem(pSubDataPicValue2, "picPath");
				printf("pic2_picPath:%s\r\n", pSubDataPic2PicPath->valuestring);
				strcpy(pic2.picPath,pSubDataPic2PicPath->valuestring);
				
				
				
				cJSON_Delete(pJsonBuffer);
				memset(buffer,'\0',sizeof(buffer));

				//delay for 3 second and empty the serial data,which for avoiding duplicate package rush into next step
				bcm2835_delay(5000);
				while( availableSerialByte() > 0){
					duplicatePackageTime += 1;
					readSerial();
				}
				
				subPart1();
				previousTime = millis();
			}
		}

		//prepare data of part2
		if(replyStart==4)
		{
			ack[i++] = response;
			if(strstr(ack,"/desired/get_reply\",\"{") !=NULL){
				printf("\033[1;40;35m/desired/get_reply\",\" checked\033[0m\r\n");
				replyStart = 5;
				memset(ack,'\0',sizeof(ack));
				i=0;
			}else{
				serialIdleFlag = 4;
			}
		}
		

		if(replyStart==5)
		{
			buffer[length++]=response;
			//printf("%c",response);
			if(response=='\n'){
				replyStart=6;
				/* /r  /n   /"  totally 3 items */
				buffer[length-3]='\0';
				length=0;
				printf("%s",buffer);
				printf("\r\n");
				
				//start to parse the json data
				cJSON *pJsonBuffer = cJSON_Parse(buffer);
				if(NULL == pJsonBuffer){
					printf("Parse fail\r\n"); 
				}
				
				cJSON * pSubData = cJSON_GetObjectItem(pJsonBuffer, "data");
				if(NULL == pSubData){
					printf("There is no data node in json\r\n");
				}
				
				/*parse Line*/
				cJSON *pSubDataLine1 = cJSON_GetObjectItem(pSubData, "line");
				cJSON *pSubDataLine1Value = cJSON_GetObjectItem(pSubDataLine1, "value");
				cJSON *pSubDataLine1ValueX1 = cJSON_GetObjectItem(pSubDataLine1Value, "x1");
				printf("x1:%d\r\n", pSubDataLine1ValueX1->valueint);
				line.x1 = pSubDataLine1ValueX1->valueint;
				cJSON *pSubDataLine1ValueY1 = cJSON_GetObjectItem(pSubDataLine1Value, "y1");
				printf("y1:%d\r\n", pSubDataLine1ValueY1->valueint);
				line.y1 = pSubDataLine1ValueY1->valueint;
				cJSON *pSubDataLine1ValueX2 = cJSON_GetObjectItem(pSubDataLine1Value, "x2");
				printf("x2:%d\r\n", pSubDataLine1ValueX2->valueint);
				line.x2 = pSubDataLine1ValueX2->valueint;
				cJSON *pSubDataLine1ValueY2 = cJSON_GetObjectItem(pSubDataLine1Value, "y2");
				printf("y2:%d\r\n", pSubDataLine1ValueY2->valueint);
				line.y2 = pSubDataLine1ValueY2->valueint;
				
				cJSON *pSubDataLine1ValueLStl = cJSON_GetObjectItem(pSubDataLine1Value, "lStl");
				printf("lStl:%d\r\n", pSubDataLine1ValueLStl->valueint);
				line.lStl = pSubDataLine1ValueLStl->valueint;
				cJSON *pSubDataLine1ValuePixel = cJSON_GetObjectItem(pSubDataLine1Value, "pixel");
				printf("pixel:%d\r\n", pSubDataLine1ValuePixel->valueint);
				line.pixel = pSubDataLine1ValuePixel->valueint;
				cJSON *pSubDataLine1ValueOnOff = cJSON_GetObjectItem(pSubDataLine1Value, "onOff");
				printf("onOff:%d\r\n", pSubDataLine1ValueOnOff->valueint);
				line.onOff = pSubDataLine1ValueOnOff->valueint;
				
				
				/*parse Point*/
				cJSON *pSubDataPoint1 = cJSON_GetObjectItem(pSubData, "point");
				cJSON *pSubDataPoint1Value = cJSON_GetObjectItem(pSubDataPoint1, "value");
				cJSON *pSubDataPoint1ValueX = cJSON_GetObjectItem(pSubDataPoint1Value, "x");
				printf("x:%d\r\n", pSubDataPoint1ValueX->valueint);
				point.x = pSubDataPoint1ValueX->valueint;
				cJSON *pSubDataPoint1ValueY = cJSON_GetObjectItem(pSubDataPoint1Value, "y");
				printf("y:%d\r\n", pSubDataPoint1ValueY->valueint);
				point.y = pSubDataPoint1ValueY->valueint;
				
				cJSON *pSubDataPoint1ValuePixel = cJSON_GetObjectItem(pSubDataPoint1Value, "pixel");
				printf("lStl:%d\r\n", pSubDataPoint1ValuePixel->valueint);
				point.pixel = pSubDataPoint1ValuePixel->valueint;
				cJSON *pSubDataPoint1ValueOnOff = cJSON_GetObjectItem(pSubDataPoint1Value, "onOff");
				printf("pixel:%d\r\n", pSubDataPoint1ValueOnOff->valueint);
				point.onOff = pSubDataPoint1ValueOnOff->valueint;
				
				
				cJSON_Delete(pJsonBuffer);
				memset(buffer,'\0',sizeof(buffer));	
				
				//delay for 3 second and empty the serial data,which for avoiding duplicate package rush into next step
				bcm2835_delay(5000);
				while( availableSerialByte() > 0){
					duplicatePackageTime += 1;
					readSerial();
				}
				
				subPart2();
				previousTime = millis();
			}
		}

		//prepare data of part2
		if(replyStart==6)
		{
			ack[i++] = response;
			if(strstr(ack,"/desired/get_reply\",\"{") !=NULL){
				printf("\033[1;40;35m/desired/get_reply\",\" checked\033[0m\r\n");
				replyStart = 7;
				memset(ack,'\0',sizeof(ack));
				i=0;
			}else{
				serialIdleFlag = 6;
			}
		}

		//Synchronous Data of part 2
		if(replyStart==7)
		{
			buffer[length++]=response;
			//printf("%c",response);
			if(response=='\n'){
				replyStart=8;
				/* /r  /n   /"  totally 3 items */
				buffer[length-3]='\0';
				length=0;
				printf("%s",buffer);
				printf("\r\n");
				
				//start to parse the json data
				cJSON *pJsonBuffer = cJSON_Parse(buffer);
				if(NULL == pJsonBuffer){
					printf("Parse fail\r\n"); 
				}
				
				cJSON * pSubData = cJSON_GetObjectItem(pJsonBuffer, "data");
				if(NULL == pSubData){
					printf("There is no data node in json\r\n");
				}
				
				/*parse rectangle*/
				cJSON *pSubDataRec = cJSON_GetObjectItem(pSubData, "rec");
				cJSON *pSubDataRecValue = cJSON_GetObjectItem(pSubDataRec, "value");
				cJSON *pSubDataRecValueX = cJSON_GetObjectItem(pSubDataRecValue, "x");
				printf("x:%d\r\n", pSubDataRecValueX->valueint);
				rec.x = pSubDataRecValueX->valueint;
				cJSON *pSubDataRecValueY = cJSON_GetObjectItem(pSubDataRecValue, "y");
				printf("y:%d\r\n", pSubDataRecValueY->valueint);
				rec.y = pSubDataRecValueY->valueint;
				cJSON *pSubDataRecValueW = cJSON_GetObjectItem(pSubDataRecValue, "w");
				printf("w:%d\r\n", pSubDataRecValueW->valueint);
				rec.w = pSubDataRecValueW->valueint;
				cJSON *pSubDataRecValueH = cJSON_GetObjectItem(pSubDataRecValue, "h");
				printf("h:%d\r\n", pSubDataRecValueH->valueint);
				rec.h = pSubDataRecValueH->valueint;
				
				cJSON *pSubDataRecValueLFill = cJSON_GetObjectItem(pSubDataRecValue, "fill");
				printf("fill:%d\r\n", pSubDataRecValueLFill->valueint);
				rec.fill = pSubDataRecValueLFill->valueint;
				cJSON *pSubDataRecValuePixel = cJSON_GetObjectItem(pSubDataRecValue, "pixel");
				printf("pixel:%d\r\n", pSubDataRecValuePixel->valueint);
				rec.pixel = pSubDataRecValuePixel->valueint;
				cJSON *pSubDataRecValueOnOff = cJSON_GetObjectItem(pSubDataRecValue, "onOff");
				printf("onOff:%d\r\n", pSubDataRecValueOnOff->valueint);
				rec.onOff = pSubDataRecValueOnOff->valueint;
				
				/*parse ciecle*/
				cJSON *pSubDataCircle = cJSON_GetObjectItem(pSubData, "circle");
				cJSON *pSubDataCircleValue = cJSON_GetObjectItem(pSubDataCircle, "value");
				cJSON *pSubDataCircleValueX = cJSON_GetObjectItem(pSubDataCircleValue, "x");
				printf("x:%d\r\n", pSubDataCircleValueX->valueint);
				circle.x = pSubDataCircleValueX->valueint;
				cJSON *pSubDataCircleValueY = cJSON_GetObjectItem(pSubDataCircleValue, "y");
				printf("y:%d\r\n", pSubDataCircleValueY->valueint);
				circle.y = pSubDataCircleValueY->valueint;
				cJSON *pSubDataCircleValueR = cJSON_GetObjectItem(pSubDataCircleValue, "r");
				printf("r:%d\r\n", pSubDataCircleValueR->valueint);
				circle.r = pSubDataCircleValueR->valueint;
				
				cJSON *pSubDataCircleValueFill = cJSON_GetObjectItem(pSubDataCircleValue, "fill");
				printf("fill:%d\r\n", pSubDataCircleValueFill->valueint);
				circle.fill = pSubDataCircleValueFill->valueint;
				cJSON *pSubDataCircleValuePixel = cJSON_GetObjectItem(pSubDataCircleValue, "pixel");
				printf("pixel:%d\r\n", pSubDataCircleValuePixel->valueint);
				circle.pixel = pSubDataCircleValuePixel->valueint;
				cJSON *pSubDataCircleValueOnOff = cJSON_GetObjectItem(pSubDataCircleValue, "onOff");
				printf("onOff:%d\r\n", pSubDataCircleValueOnOff->valueint);
				circle.onOff = pSubDataCircleValueOnOff->valueint;
				
				
				cJSON_Delete(pJsonBuffer);
				memset(buffer,'\0',sizeof(buffer));
				
				//delay for 3 second and empty the serial data,which for avoiding duplicate package rush into next step
				bcm2835_delay(5000);
				while( availableSerialByte() > 0){
					duplicatePackageTime += 1;
					readSerial();
				}
				
				subPart3();
				previousTime = millis();
			}
		}
		
		//prepare data of part3
		if(replyStart==8)
		{
			ack[i++] = response;
			if(strstr(ack,"/desired/get_reply\",\"{") !=NULL){
				printf("\033[1;40;35m/desired/get_reply\",\" checked\033[0m\r\n");
				replyStart = 9;
				memset(ack,'\0',sizeof(ack));
				i=0;
			}else{
				serialIdleFlag = 8;
			}
		}
		//Synchronous Data of part 3
		if(replyStart==9)
		{
			buffer[length++]=response;
			//printf("%c",response);
			if(response=='\n'){
				replyStart=10;
				/* /r  /n   /"  totally 3 items */
				buffer[length-3]='\0';
				length=0;
				printf("%s",buffer);
				printf("\r\n");
				
				//start to parse the json data
				cJSON *pJsonBuffer = cJSON_Parse(buffer);
				if(NULL == pJsonBuffer){
					printf("Parse fail\r\n"); 
				}
				
				cJSON * pSubData = cJSON_GetObjectItem(pJsonBuffer, "data");
				if(NULL == pSubData){
					printf("There is no data node in json\r\n");
				}
				
				/*parse strEn*/
				cJSON *pSubDatastrEn = cJSON_GetObjectItem(pSubData, "strEn");
				cJSON *pSubDatastrEnValue = cJSON_GetObjectItem(pSubDatastrEn, "value");
				cJSON *pSubDatastrEnValueX = cJSON_GetObjectItem(pSubDatastrEnValue, "x");
				printf("x:%d\r\n", pSubDatastrEnValueX->valueint);
				strEn.x = pSubDatastrEnValueX->valueint;
				cJSON *pSubDatastrEnValueY = cJSON_GetObjectItem(pSubDatastrEnValue, "y");
				printf("y:%d\r\n", pSubDatastrEnValueY->valueint);
				strEn.y = pSubDatastrEnValueY->valueint;
				
				cJSON *pSubDatastrEnValueOnOff = cJSON_GetObjectItem(pSubDatastrEnValue, "onOff");
				printf("onOff:%d\r\n", pSubDatastrEnValueOnOff->valueint);
				strEn.onOff = pSubDatastrEnValueOnOff->valueint;
				cJSON *pSubDatastrEnValueFrontColor = cJSON_GetObjectItem(pSubDatastrEnValue, "frontColor");
				printf("frontColor:%d\r\n", pSubDatastrEnValueFrontColor->valueint);
				strEn.frontColor = pSubDatastrEnValueFrontColor->valueint;
				cJSON *pSubDatastrEnValueBackColor = cJSON_GetObjectItem(pSubDatastrEnValue, "backColor");
				printf("backColor:%d\r\n", pSubDatastrEnValueBackColor->valueint);
				strEn.backColor = pSubDatastrEnValueBackColor->valueint;
				cJSON *pSubDatastrEnValueFont = cJSON_GetObjectItem(pSubDatastrEnValue, "font");
				printf("font:%d\r\n", pSubDatastrEnValueFont->valueint);
				strEn.font = pSubDatastrEnValueFont->valueint;
				cJSON *pSubDatastrEnValueStr = cJSON_GetObjectItem(pSubDatastrEnValue, "str");
				printf("str:%s\r\n", pSubDatastrEnValueStr->valuestring);
				memset(strEn.str,'\0',sizeof(strEn.str));
				strcpy(strEn.str,pSubDatastrEnValueStr->valuestring);
				
				/*parse strCn*/
				cJSON *pSubDatastrCn = cJSON_GetObjectItem(pSubData, "strCn");
				cJSON *pSubDatastrCnValue = cJSON_GetObjectItem(pSubDatastrCn, "value");
				cJSON *pSubDatastrCnValueX = cJSON_GetObjectItem(pSubDatastrCnValue, "x");
				printf("x:%d\r\n", pSubDatastrCnValueX->valueint);
				strCn.x = pSubDatastrCnValueX->valueint;
				cJSON *pSubDatastrCnValueY = cJSON_GetObjectItem(pSubDatastrCnValue, "y");
				printf("y:%d\r\n", pSubDatastrCnValueY->valueint);
				strCn.y = pSubDatastrCnValueY->valueint;
				
				cJSON *pSubDatastrCnValueOnOff = cJSON_GetObjectItem(pSubDatastrCnValue, "onOff");
				printf("onOff:%d\r\n", pSubDatastrCnValueOnOff->valueint);
				strCn.onOff = pSubDatastrCnValueOnOff->valueint;
				cJSON *pSubDatastrCnValueFrontColor = cJSON_GetObjectItem(pSubDatastrCnValue, "frontColor");
				printf("frontColor:%d\r\n", pSubDatastrCnValueFrontColor->valueint);
				strCn.frontColor = pSubDatastrCnValueFrontColor->valueint;
				cJSON *pSubDatastrCnValueBackColor = cJSON_GetObjectItem(pSubDatastrCnValue, "backColor");
				printf("backColor:%d\r\n", pSubDatastrCnValueBackColor->valueint);
				strCn.backColor = pSubDatastrCnValueBackColor->valueint;
				
				
				cJSON *pSubDatastrCnValueStr = cJSON_GetObjectItem(pSubDatastrCnValue, "str");
				printf("str:%s\r\n", pSubDatastrCnValueStr->valuestring);
				memset(strCn.str,'\0',sizeof(strCn.str));
				strcpy(strCn.str,pSubDatastrCnValueStr->valuestring);
				
				cJSON_Delete(pJsonBuffer);
				memset(buffer,'\0',sizeof(buffer));
				
				//start to refresh the screen
				refreshScreen();
			}
		}

		if(replyStart==10)
		{
			unsigned long previous;
			int count=5;
			previous = millis();
			while((millis() - previous) < 5000){
				printf("Program will Synchronize data again in %d second\r\n",count--);
				bcm2835_delay(1000);
			}
			//Will reset and connect to Aliyun again and get data
			sim7000ReturnInitState();
		}
    }else{
		if(serialIdleFlag==0||serialIdleFlag==2||serialIdleFlag==4||serialIdleFlag==6||serialIdleFlag==8){
			if((millis() - previousTime) > 15000){
				noneMqttPackageTime += 1;
				printf("wait serial data timeout,reset and try again\r\n");
				printf("the serialIdleFlag is %d\r\n",serialIdleFlag);
				sim7000ReturnInitState();
			}
		}
	}
}


int main(void){
	
	int ret;
	
	DEV_ModuleInit();
	
	signal(SIGINT, Handler);
	
	ret = sim7000Init();
	if(ret!=0){
		printf("sim7000 init failed  !!!\r\n");
		return 2;
	}else{
		printf("sim7000 init success !!!\r\n");
	}
	
	sim7000ReturnInitState();
	
	while(1){
		loop();
	}
}
