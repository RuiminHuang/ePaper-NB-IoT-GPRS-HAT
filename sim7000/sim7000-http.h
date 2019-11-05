#ifndef __SIM7000_HTTP_H__
#define __SIM7000_HTTP_H__


int httpGetStatus( int*httpStatus, int*httpLength);
int httpWriteFile(int length);
int httpConnect(char * address);

#endif