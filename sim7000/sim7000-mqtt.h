#ifndef __SIM7000_MQTT_H__
#define __SIM7000_MQTT_H__

#include <stdbool.h>

extern char productkey[];
extern char devicename[];
extern char regionid[];
extern char devicetimestamp[];
extern char devicepassword[];

int getIniInfo();
int MQTTConnectToAliyun(void);

int subPartRFType();
int subPicAddr();
int subPart1();
int subPart2();
int subPart3();

int checkConnectStatue(void);
void DisconnectAliyun(void);

#endif