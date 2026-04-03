#ifndef WEB_HANDLE_H
#define WEB_HANDLE_H
#include "config.h"

// ฟังก์ชันหลักในการรัน Web Route ทั้งหมด
void initWebRoutes();

// Helpers
String getContentType(String filename);
void handleStaticFiles();
void handleJsonAPI();
void handleCommandAPI();
void handleNotFound();

#endif