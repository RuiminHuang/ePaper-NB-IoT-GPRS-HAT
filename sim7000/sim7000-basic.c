#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "sim7000-basic.h"

#include "../drive/serial.h"

#include "../ini/iniParse.h"

#include "../json/cJSON.h"

bool powerOn(int power_key){
		
    char answer  = 0, count = 0;
	
	// checks if the module is started
	answer = Send_AT_Command("AT", 1000, "OK","");
	
	if (answer == 0){

		printf("Module is not ready,...\n");

		bcm2835_gpio_fsel(power_key, BCM2835_GPIO_FSEL_OUTP);
		
		// power on pulse
		bcm2835_gpio_write(power_key, HIGH);
		bcm2835_delay(200);
		bcm2835_gpio_write(power_key, LOW);

		// waits for an answer from the module
		while ((answer == 0)&&(count++<100)){

            bcm2835_delay(200);
			// Send AT every two seconds and wait for the answer
			answer = Send_AT_Command("AT", 1000, "OK","");

		}
	}

    if(answer == 1){
        printf("Module is ready!!!\r\n");
        return true;
    }

    printf("Module communiction failed.\r\n");
    printf("Please check if the module is not powered or not set to the default baud rate:115200.\r\n");
    return false;
}




int softReset(void){	
	if(powerOn(4)==false){
		return 1;
	}
	
	if(Send_AT_Command("AT+CFUN=1,1",3000,"OK","ERROR")!=1){
		return 2;
	}
	
	bcm2835_delay(5000);
	
	if(powerOn(4)==false){
		return 3;
	}

	return 0;
}



//if software reset fail, then do a hardware reset
int hardReset(void){

	return 0;
}



int Send_AT_Command(const char* at_command, unsigned int timeout, const char* expected_answer1, const char* expected_answer2){
	int x=0,  answer=0;
    char response[2048];
    unsigned long previous;
    memset(response, '\0', sizeof(response));    // Initialize the string
	
    bcm2835_delay(100);

	//Clean the input buffer
    while( availableSerialByte() > 0)
	{
		readSerial();
	}

	//Send the AT command
    println(at_command);

    previous = millis();
	
	printf("(read serial data):\r\n");
    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(availableSerialByte() != 0){
			
            response[x] = readSerial();
			
			printf("\033[1;32;40m%c\033[0m",response[x]);

			if(strcmp(expected_answer1,"") != 0)
            {
                // check if the desired answer 1  is in the response of the module
                if (strstr(response, expected_answer1) != NULL)   
                {
					printf("\r\n");
                    answer = 1;
                }
                // check if the desired answer 2 is in the response of the module
                else if(strcmp(expected_answer2,"") != 0) 
                {
                    if(strstr(response, expected_answer2) != NULL)
                    {
						printf("\r\n");
                        answer = 2;
                    }
                }
            }
            else
            {
                printf("\n");
            }
            x++;
        }
    }while((answer == 0) && ((millis() - previous) < timeout));
	
    return answer;
}


int Get_Signal_Value(void){
    char signal_value1[30], signal_value2[3];
    uint8_t x = 0, answer = 0;
    int signal_value = 0;
    unsigned long int timeout = 5000,pre_time = 0;
    memset(signal_value1,'\0',30);
    memset(signal_value2,'\0',3);

    pre_time = millis();

	//it has already read to "CSQ: ",so it will directly get the data.
    if(Send_AT_Command("AT+CSQ",1000,"+CSQ: ","") == 1){

        while(availableSerialByte()==0);
        do{
            if(availableSerialByte()>0){
                signal_value1[x++] = readSerial();
            }
			
            if(strstr(signal_value1,"OK") != NULL){
                signal_value1[x]='\0';
                for(uint8_t j=0; (j<=2)&&(signal_value1[j] != ','); j++ ){
                    signal_value2[j] = signal_value1[j];
				}
				//change the char to int
                signal_value = atoi(signal_value2);
                printf("The signal value is:%d\n",signal_value);
                answer = 1;
            }
        }while((answer == 0)&&((millis()-pre_time)<timeout));
    }
    
    if(answer == 0){
        printf("Get the signal quality value failed.\n");
        return 0;
    }
    return signal_value;
}


int Get_Supported_APN(char *_apn){
	
    char network_apn[30];
    int x = 0, answer = 0;
    char *y;
    unsigned long int timeout = 5000,pre_time = 0;
    memset(network_apn,'\0',sizeof(network_apn));
    memset(_apn,'\0',strlen(_apn));

    pre_time = millis();
    if(Send_AT_Command("AT+CGNAPN",2000,"+CGNAPN: ","") == 1)
    {
        while(availableSerialByte()==0);

        do
        {
            if(availableSerialByte()>0)
            {
                network_apn[x++] = readSerial();
            }

            if(strstr(network_apn,"OK") != NULL)
            {
                network_apn[x]='\0';
                y = strstr(network_apn,"\"");

                for(int j=0; (*(y+j+1) != '"'); j++)
				{
					_apn[j] = *(y+j+1);
				}
				
                answer = 1;
            }
        }while((answer == 0)&&((millis()-pre_time)<timeout));
    }
    if(answer == 0){
        printf("Get the network apn failed.\n");
    }
	return answer;
}



int Get_COPS_Value(void){
	char cops;
	char copsTemp[40];
    int length = 0, answer = 0;
	
    unsigned long int timeout = 5000,pre_time = 0;
    memset(copsTemp,'\0',sizeof(copsTemp));

    pre_time = millis();
	
    if(Send_AT_Command("AT+COPS?" , 2000 , "+COPS:" , "") == 1)
    {
        while(availableSerialByte()==0);

        do
        {
            if(availableSerialByte()>0)
            {
                copsTemp[length++] = readSerial();
            }

            if(strstr(copsTemp,"OK") != NULL)
            {
                copsTemp[length]='\0';
				
				for(int i=length; i>0; i-- ){
					
					if(copsTemp[i]==','){
						cops = copsTemp[i+1];
						printf("cops is %c\r\n",cops);
						answer = 1;
						break;
					}
				}
            }
        }while((answer == 0)&&((millis()-pre_time)<timeout));
    }
	
    if(answer == 0){
        printf("The coap is 0.\n");
		return 0;
    }
    return (int)cops-(int)'0';
}
