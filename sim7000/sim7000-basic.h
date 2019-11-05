#ifndef __SIM7000_BASIC_H__
#define __SIM7000_BASIC_H__

#include <stdbool.h>

bool powerOn(int power_key);

int softReset(void);
int hardReset(void);

int Send_AT_Command(const char* at_command, unsigned int timeout, const char* expected_answer1, const char* expected_answer2);
int  Get_Signal_Value(void);
int Get_Supported_APN(char *_apn);
int Get_COPS_Value(void);

#endif