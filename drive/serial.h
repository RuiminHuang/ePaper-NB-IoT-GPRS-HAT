#ifndef __SRIAL_H__
#define __SRIAL_H__

#define _BSD_SOURCE
#define _DEFAULT_SOURCE


int speed;

extern const char *serialPort;

extern int sd,status;

extern struct termios options;

extern struct timeval start_program, end_point;


void beginSerial(int serialSpeed);

int availableSerialByte();

void println(const char *message);

void print(char * message, int len);

char readSerial();

int readSerialBuffer(char* message,int len);

long millis();

#endif