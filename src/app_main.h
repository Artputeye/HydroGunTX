#ifndef OPERATION_H
#define OPERATION_H
#include "config.h"

typedef struct
{
  int8_t updown;
  int8_t leftright;
  bool fire;
} ControlPacket;

extern ControlPacket data;
extern ControlPacket lastData;

extern bool espnowConnected ;
extern int espnowRSSI;
extern int packetLoss;

void app_setup();
void app_loop();
void updateSystemStatus();

#endif