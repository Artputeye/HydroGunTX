#ifndef IOT_HA_H
#define IOT_HA_H
#include "config.h"

// ตัวแปรที่อนุญาตให้ไฟล์อื่นเรียกใช้
extern HASwitch sw;
extern HASensorNumber temp;

// ฟังก์ชันหลัก
void iotHAsetup();
void HA_Diagnostic();
void iotHArun();

#endif