#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "sim7000-http.h"

#include "sim7000-basic.h"

#include "../drive/serial.h"

#include "../ini/iniParse.h"

#include "../json/cJSON.h"



int httpGetStatus( int*httpStatus, int*httpLength){
	int x=0, i=0, j=0;
	
	bool statusFinishFlag = false;
	
	bool allFinishFlag = false;
	
	bool firstCommaFlag = false;
	
	bool secondCommaFlag = false;
	
	unsigned long previous;
	
	char httpStatusChar[5]; 
	memset(httpStatusChar,'\0',sizeof(httpStatusChar));
	
	char httpLengthChar[10];
	memset(httpLengthChar,'\0',sizeof(httpLengthChar));
	
	char response[2048];
	memset(response,'\0',sizeof(response));
	
	bcm2835_delay(100);
	
	while(availableSerialByte()>0){
		readSerial();
	}
	
	printf("httpGetStatus program is start now...\r\n");
	
	println("AT+HTTPACTION=0");
	
	previous = millis();
	
	printf("(read serial data):\r\n");
	
	do{
		
		if( availableSerialByte() != 0 ){
			
			response[x] = readSerial();
			
			printf("\033[1;32;40m%c\033[0m",response[x]);
			
			if( firstCommaFlag == false ){
				if(response[x] == ','){
					
					firstCommaFlag = true;
					
					printf("first comma find\r\n");
					
					continue;
				}
			}		
			if( (firstCommaFlag == true)&&(statusFinishFlag == false) ){
				if(response[x] == ','){
					
					statusFinishFlag = true;
					
					secondCommaFlag = true;
					
					printf("second comma find\r\n");
					
					continue;
				}else{
					printf("httpStatusChar:%c\r\n",response[x]);
					httpStatusChar[i++] = response[x];
					
				}
			}
			if( secondCommaFlag == true){
				
				if(response[x]=='\r'){
					
					allFinishFlag = true;
					
					printf("finish flag find\r\n");
					
				}else{
					printf("httpLengthChar:%c\r\n",response[x]);
					httpLengthChar[j++] = response[x];
				}
			}
			x++;
		}
	//wait for 4 minute only	
	}while((allFinishFlag == false)&&((millis()-previous) < 180000));
	
	httpStatusChar[i] = '\0';

	httpLengthChar[j] = '\0';
	
	printf("%s\r\n",httpLengthChar);
	
	//printf("---------the httpStatus is %d---------\r\n",atoi(httpStatusChar));
	//printf("---------the httpLength is %d---------\r\n",atoi(httpLengthChar));
	
	*httpStatus = atoi(httpStatusChar);
	*httpLength = atoi(httpLengthChar);
	
	if(atoi(httpStatusChar) == 200){
		return 0;
	}else{
		return 1;
	}
}




int httpWriteFile(int length){
	
	int x = 0; 
	int i = 0;
	unsigned long previous;
	
	bool colonFlag = false;
	
	bool dataStartFlag = false;
	
	char response[2048];
	memset(response,'\0',sizeof(response));
	
	bcm2835_delay(100);
	
	while(availableSerialByte()>0){
		
		readSerial();
		
	}
	
	printf("httpWriteFile program is start now...\r\n");
	
	println("AT+HTTPREAD");
	
	previous = millis();
	
	printf("(read serial data):\r\n");
	
	do{
		if(availableSerialByte() != 0 ){

			response[x] = readSerial();
			printf("\033[1;32;40m%c\033[0m",response[x]);
			
			if( colonFlag == false ){
			
				if(response[x] == ':'){
					colonFlag = true;
					continue;
				}
			}
			
			if(colonFlag == true){				
				if(response[x] == '\n'){					
					dataStartFlag = true;
				}				
			}						
			x++;
		}
	}while( (dataStartFlag == false) && ((millis()-previous) < 60000) );
	
	response[x] = '\0';
	
	if(dataStartFlag == true){
		
		FILE * file  = fopen("./PIC/wireless-pic.bmp","wb");
		
		if(file == NULL){
			printf("file cann't be accessed");
		}
		
		uint8_t temp;
		printf("Start to write file\r\n");
		while(i<length){
			
			temp = readSerial();
			
			fwrite(&temp,1,1,file);
			
			i++;
		}
		fclose(file);	
	}
	
	return i;
}


int httpConnect(char * address)
{
	int i;
	char commander[1024]={'\0'};
	
	if(softReset()!=0){
		return 1;
	}
	
	if(Send_AT_Command("AT+CNMP=38",3000,"OK","ERROR")!=1){
		return 1;
	}
	
	if(Send_AT_Command("AT+CMNB=2",3000,"OK","ERROR")!=1){
		return 2;
	}
	
    if(Send_AT_Command("AT+NBSC=1",3000,"OK","ERROR")!=1){
		return 3;
	}
		
	Send_AT_Command("AT+CGNAPN",3000,"OK","ERROR");
	char apn[10]={'\0'};
	Get_Supported_APN(apn);
	printf("The valid apn is:%s\n",apn);
	
	while(strcmp(apn,"")==0){
		bcm2835_delay(500);
		Get_Supported_APN(apn);
	}
	
	char cmd[40] = {'\0'};
    sprintf(cmd,"AT+SAPBR=3,1,\"APN\",\"%s\"",apn);
	
	
	if(Send_AT_Command(cmd,3000,"OK","ERROR")!=1){
		return 4;
	}
	
	if(Send_AT_Command("AT+SAPBR=1,1",3000,"OK","ERROR")!=1){
		return 5;
	}
	
	if(Send_AT_Command("AT+SAPBR=2,1",3000,"OK","ERROR")!=1){
		return 6;
	}
		
	Send_AT_Command("AT+CSQ",3000,"OK","ERROR");
	
	int signalValue,times=0;
	
	do{
		signalValue = Get_Signal_Value();
		
		bcm2835_delay(500);
		
		times++;
	}while((signalValue==99)&&(times<240));

	
	
	if(signalValue==99){
		printf("The network works bad,please check your network!!!\r\n");
		return 7;
	}else if(signalValue < 10){
		printf("The signalValue is %d\r\n",signalValue);
		printf("The NB-IoT net signal is bad,please choose another location where the signal is above 10 to test!!!\r\n");
		return 7;
	}
	
	
	if(Send_AT_Command("AT+HTTPINIT",3000,"OK","ERROR")!=1){
		return 8;
	}
	
	if(Send_AT_Command("AT+HTTPPARA=\"CID\",1",3000,"OK","ERROR")!=1){
		return 9;
	}
	
	
	if(Send_AT_Command("AT+HTTPPARA=\"BREAK\",0",3000,"OK","ERROR")!=1){
		return 100;
	}

	if(Send_AT_Command("AT+HTTPPARA=\"BREAKEND\",0",3000,"OK","ERROR")!=1){
		return 101;
	}	
	
	if(Send_AT_Command("AT+HTTPPARA=\"TIMEOUT\",1000",3000,"OK","ERROR")!=1){
		return 102;
	}
	
	sprintf(commander,"AT+HTTPPARA=\"URL\",\"%s\"", address);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 11;
	}
	
	//if(Send_AT_Command("AT+HTTPPARA=\"URL\",\"http://tenacioustornado.oss-cn-shenzhen.aliyuncs.com/PIC13.BMP\"",3000,"OK","ERROR")!=1){
	//	return 11;
	//}


	int httpStatus = 0, httpLength = 0, writeLength;

	//try 5 times to download file without reset
	printf("start to download file from http address\r\n");
	for(i=0; i<5; i++){
		httpGetStatus(&httpStatus,&httpLength);
		printf("httpStatus is %d  ,httpLength is % d\r\n",httpStatus,httpLength);
		if(httpStatus ==200){
			printf("download file from http address success\r\n");
			break;
		}else{
			if(i != 5-1){
				printf("Without reset to down file time %d fail,will try again\r\n",i+1);
			}else{
				printf("Sorry,download fail,will reset and try again\r\n");
				return 12;
			}
		}
		bcm2835_delay(1000);
	}
	
	writeLength = httpWriteFile(httpLength);
	
	if(writeLength != httpLength){
		return 13;	
	}
	
	if(Send_AT_Command("AT+HTTPTERM",3000,"OK","ERROR")!=1){
		return 14;
	}
	return 0;
}