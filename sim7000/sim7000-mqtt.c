#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "sim7000-mqtt.h"

#include "sim7000-basic.h"

#include "../drive/serial.h"

#include "../ini/iniParse.h"

#include "../json/cJSON.h"


char productkey[128]="\0";

char devicename[128]="\0";

char regionid[128] = "\0";

char devicetimestamp[128] = "\0";

char devicepassword[128] = "\0";

int getIniInfo(){
	char *temp;
	FILE *fp = fopen("./CONFIG/device.ini","r");
	if(fp == NULL){
		printf("please config device.ini file\r\n");
		return 1;
	}
	temp = inifindTag(fp, "productkey");
	strcpy(productkey,temp);
	printf("productkey:%s\n",temp);
	if(temp == NULL){
		printf("please config productkey\r\n");
	}
	temp = inifindTag(fp, "devicename");
	strcpy(devicename,temp);
	printf("devicename:%s\n",temp);
	if(temp == NULL){
		printf("please config devicename\r\n");
	}
	temp = inifindTag(fp, "regionid");
	strcpy(regionid,temp);
	printf("regionid:%s\n",temp);
	if(temp == NULL){
		printf("please config regionid\r\n");
	}
	temp = inifindTag(fp, "devicetimestamp");
	strcpy(devicetimestamp,temp);
	printf("devicetimestamp:%s\n",temp);
	if(temp == NULL){
		printf("please config devicetimestamp\r\n");
	}
	temp = inifindTag(fp, "devicepassword");
	strcpy(devicepassword,temp);
	printf("devicepassword:%s\n",temp);
	if(temp == NULL){
		printf("please config devicepassword\r\n");
	}
	fclose(fp);
	return 0;
}




//all data above is used to define and authenticate a device
//----------------------------------------------------------------------------
int MQTTConnectToAliyun(void){

	int ret,i;
	
	if(softReset()!=0){
		return 1;
	}
	
	getIniInfo();
		
	char commander[1024]={'\0'};
	
	
    if(Send_AT_Command("AT+CFGRI=0",3000,"OK","ERROR")!=1){
		return 1;
	}
	
	if(Send_AT_Command("AT+CSCLK=0",3000,"OK","ERROR")!=1){
		return 2;
	}
	
	if(Send_AT_Command("AT+CPSMS=0",3000,"OK","ERROR")!=1){
		return 3;
	}
	
	if(Send_AT_Command("AT+CPIN?",3000,"OK","ERROR")!=1){
		return 4;
	}
	
	if(Send_AT_Command("AT+CGMR",3000,"OK","ERROR")!=1){
		return 100;
	}
	
	//Following for set China Mobile
	//------------------------------------------------------
	if(Send_AT_Command("AT+CNMP=38",3000,"OK","ERROR")!=1){
		return 101;
	}
	
	if(Send_AT_Command("AT+CNMP?",3000,"OK","ERROR")!=1){
		return 102;
	}
	
	if(Send_AT_Command("AT+CMNB=2",3000,"OK","ERROR")!=1){
		return 103;
	}
	
	if(Send_AT_Command("AT+CMNB?",3000,"OK","ERROR")!=1){
		return 104;
	}
	
	if(Send_AT_Command("AT+NBSC=1",3000,"OK","ERROR")!=1){
		return 105;
	}
	
	if(Send_AT_Command("AT+NBSC?",3000,"OK","ERROR")!=1){
		return 106;
	}
	//------------------------------------------------------
	
	Send_AT_Command("AT+CSQ",3000,"OK","ERROR");
	int signalValue,times=0;
	do{
		signalValue = Get_Signal_Value();
		
		bcm2835_delay(500);
		
		times++;
	}while((signalValue==99)&&(times<240));
	
	if(signalValue==99){
		printf("The network works bad,please check your network!!!\r\n");
		return 5;
	}else if(signalValue < 10){
		printf("The signalValue is %d\r\n",signalValue);
		printf("The net signal is bad,please choose another location where the signal is above 10 to test!!!\r\n");
		return 5;
	}
	
	if(Send_AT_Command("AT+CGREG?",3000,"OK","ERROR")!=1){
		return 6;
	}
	
	Send_AT_Command("AT+COPS?",3000,"OK","ERROR");
	int copsValue;
	times=0;
	do{
		//every time，it delay for 5 second
		copsValue = Get_COPS_Value();
		times++;
	}while((copsValue!=9)&&(times<240));
	if(copsValue!=9){
		return 7;
	}
	
	Send_AT_Command("AT+CGNAPN",3000,"OK","ERROR");
	char apn[10]={'\0'};
	Get_Supported_APN(apn);
	printf("The valid apn is:%s\n",apn);
	
	char cmd[40] = {'\0'};
    sprintf(cmd,"AT+CNACT=1,%s",apn);
	if(Send_AT_Command(cmd,3000,"ACTIVE","ERROR")!=1){
		return 8;
	}
	
	if(Send_AT_Command("AT+CNACT?",3000,"OK","ERROR")!=1){
		return 9;
	}
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMCONF=\"CLIENTID\",\"%s|securemode=3,signmethod=hmacsha1,timestamp=%s|\"",devicename,devicetimestamp);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 10;
	}
	
	if(Send_AT_Command("AT+SMCONF=\"KEEPTIME\",\"60\"",3000,"OK","ERROR")!=1){
		return 11;
	}

	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMCONF=\"URL\",\"%s.iot-as-mqtt.%s.aliyuncs.com\",\"1883\"",productkey,regionid);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 12;
	}
	
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMCONF=\"USERNAME\",\"%s&%s\"",devicename,productkey);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 13;
	}
	
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMCONF=\"PASSWORD\",\"%s\"",devicepassword);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 14;
	}
	
	if(Send_AT_Command("AT+SMCONF=\"QOS\",0",3000,"OK","ERROR")!=1){
		return 15;
	}
	
	if(Send_AT_Command("AT+SMCONF=\"RETAIN\",0",3000,"OK","ERROR")!=1){
		return 16;
	}
	
	if(Send_AT_Command("AT+SMCONF=\"CLEANSS\",1",3000,"OK","ERROR")!=1){
		return 17;
	}
	
	if(Send_AT_Command("AT+SMCONF?",3000,"OK","ERROR")!=1){
		return 18;
	}
	
	bcm2835_delay(500);
	
	printf("start to connect to aliyun IoT platform");

	for(i=0; i<5; i++){
		ret = Send_AT_Command("AT+SMCONN",30000,"OK","ERROR");
		if(ret == 1){
			printf("congratulation!!!, you have connect to Aliyun IoT Platform\r\n");
			break;
		}else if(ret==0 || ret==2){
			if(i != 5-1){
				printf("Without reset to connect to Aliyun IoT platform time %d fail, will try again\r\n",i+1);
			}else{
				printf("Sorry connect failure,will reset and try again\r\n");
				return 19;
			}
		}
		bcm2835_delay(2000);
	}
	
	//sub for web test
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMSUB=\"/%s/%s/user/get\",1",productkey,devicename);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 20;
	}
	
	//not use setting data this moment
	#if(0)
	//sub for set
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMSUB=\"/sys/%s/%s/thing/service/property/set\",1",productkey,devicename);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 21;
	}
	#endif

	//sub for desired
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMSUB=\"/sys/%s/%s/thing/property/desired/get_reply\",1",productkey,devicename);
	if(Send_AT_Command(commander,3000,"OK","ERROR")!=1){
		return 22;
	}
	
	
	return 0;
}



int subPartRFType(){
	//create the json
	cJSON *root,*array;
	char *out;
	root=cJSON_CreateObject();	
	cJSON_AddStringToObject(root, "id", "123");
	cJSON_AddStringToObject(root, "version", "1.0");
	cJSON_AddItemToObject(root, "params", array=cJSON_CreateArray());
	cJSON_AddItemToArray(array, cJSON_CreateString("rfType"));
	cJSON_AddItemToArray(array, cJSON_CreateString("scrType"));
	cJSON_AddStringToObject(root, "method", "thing.property.desired.get");
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	char commander[1024]={'\0'};
	
	//pub topic
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMPUB=\"/sys/%s/%s/thing/property/desired/get\",%d,1,0",productkey,devicename,strlen(out));
	if(Send_AT_Command(commander,3000,">","ERROR")!=1){
		return 23;
	}
	//pub message
	if(Send_AT_Command(out,90000,"OK","ERROR")!=1){
		return 24;
	}
	//free the json buffer
	free(out);
	return 0;
}



int subPicAddr(){
	//create the json
	cJSON *root,*array;
	char *out;
	root=cJSON_CreateObject();	
	cJSON_AddStringToObject(root, "id", "123");
	cJSON_AddStringToObject(root, "version", "1.0");
	cJSON_AddItemToObject(root, "params", array=cJSON_CreateArray());
	cJSON_AddItemToArray(array, cJSON_CreateString("pic1"));
	cJSON_AddItemToArray(array, cJSON_CreateString("pic2"));
	cJSON_AddStringToObject(root, "method", "thing.property.desired.get");
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	char commander[1024]={'\0'};
	
	//pub topic
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMPUB=\"/sys/%s/%s/thing/property/desired/get\",%d,1,0",productkey,devicename,strlen(out));
	if(Send_AT_Command(commander,3000,">","ERROR")!=1){
		return 23;
	}
	//pub message
	if(Send_AT_Command(out,90000,"OK","ERROR")!=1){
		return 24;
	}
	//free the json buffer
	free(out);
	return 0;
}





int subPart1(){
	//create the json
	cJSON *root,*array;
	char *out;
	root=cJSON_CreateObject();	
	cJSON_AddStringToObject(root, "id", "123");
	cJSON_AddStringToObject(root, "version", "1.0");
	cJSON_AddItemToObject(root, "params", array=cJSON_CreateArray());
	cJSON_AddItemToArray(array, cJSON_CreateString("point"));
	cJSON_AddItemToArray(array, cJSON_CreateString("line"));
	cJSON_AddStringToObject(root, "method", "thing.property.desired.get");
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	char commander[1024]={'\0'};
	
	//pub topic
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMPUB=\"/sys/%s/%s/thing/property/desired/get\",%d,1,0",productkey,devicename,strlen(out));
	if(Send_AT_Command(commander,3000,">","ERROR")!=1){
		return 23;
	}
	//pub message
	if(Send_AT_Command(out,90000,"OK","ERROR")!=1){
		return 24;
	}
	//free the json buffer
	free(out);
	return 0;
}

int subPart2(){
	//create the json
	cJSON *root,*array;
	char *out;
	root=cJSON_CreateObject();	
	cJSON_AddStringToObject(root, "id", "123");
	cJSON_AddStringToObject(root, "version", "1.0");
	cJSON_AddItemToObject(root, "params", array=cJSON_CreateArray());
	cJSON_AddItemToArray(array, cJSON_CreateString("circle"));
	cJSON_AddItemToArray(array, cJSON_CreateString("rec"));
	cJSON_AddStringToObject(root, "method", "thing.property.desired.get");
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	char commander[1024]={'\0'};
	
	//pub topic
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMPUB=\"/sys/%s/%s/thing/property/desired/get\",%d,1,0",productkey,devicename,strlen(out));
	if(Send_AT_Command(commander,3000,">","ERROR")!=1){
		return 23;
	}
	//pub message
	if(Send_AT_Command(out,90000,"OK","ERROR")!=1){
		return 24;
	}
	//free the json buffer
	free(out);
	return 0;
}

int subPart3(){
	//create the json
	cJSON *root,*array;
	char *out;
	root=cJSON_CreateObject();	
	cJSON_AddStringToObject(root, "id", "123");
	cJSON_AddStringToObject(root, "version", "1.0");
	cJSON_AddItemToObject(root, "params", array=cJSON_CreateArray());
	cJSON_AddItemToArray(array, cJSON_CreateString("strEn"));
	cJSON_AddItemToArray(array, cJSON_CreateString("strCn"));
	cJSON_AddStringToObject(root, "method", "thing.property.desired.get");
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	char commander[1024]={'\0'};
	
	//pub topic
	memset(commander,'\0',sizeof(commander));
	sprintf(commander,"AT+SMPUB=\"/sys/%s/%s/thing/property/desired/get\",%d,1,0",productkey,devicename,strlen(out));
	if(Send_AT_Command(commander,3000,">","ERROR")!=1){
		return 23;
	}
	//pub message
	if(Send_AT_Command(out,90000,"OK","ERROR")!=1){
		return 24;
	}
	//free the json buffer
	free(out);
	return 0;
}



int checkConnectStatue(void){

    char rslt_value1[30], rslt_value2[2];
    int x = 0, answer = 0;
    int rslt_value = 0;
    unsigned long int timeout = 5000,pre_time = 0;
    memset(rslt_value1,'\0',30);
    memset(rslt_value2,'\0',2);

    pre_time = millis();

	//it has already read to "CSQ: ",so it will directly get the data.
    if(Send_AT_Command("AT+SMSTATE?",1000,"+SMSTATE: ","") == 1){

        while(availableSerialByte()==0);
        do{
            if(availableSerialByte()>0){
                rslt_value1[x++] = readSerial();
            }
			
            if(strstr(rslt_value1,"OK") != NULL){
                rslt_value1[x]='\0';
				
				int pos = 0;
				for(pos=0; rslt_value1[pos]!=':'; pos++){
					break;
				}
				
				rslt_value2[0]=rslt_value1[pos];
				
				//change the char to int
                rslt_value = atoi(rslt_value2);
                printf("\033[4;37;40mconnect status:%d\033[0m\r\n",rslt_value);
                answer = 1;
            }
        }while((answer == 0)&&((millis()-pre_time)<timeout));
    }
    
    if(answer == 0){
        printf("Get the signal quality value failed.\n");
        return 0;
    }
    return rslt_value;
}


void DisconnectAliyun(void){
	
	checkConnectStatue();
	
	Send_AT_Command("AT+SMDISC",3000,"OK","ERROR");
	
	Send_AT_Command("AT+CNACT=0",3000,"OK","ERROR");

	checkConnectStatue();

}