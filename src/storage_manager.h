#ifndef FS_FILE_H
#define FS_FILE_H
#include "config.h"

// File System Management
void listAllFiles(const char *dirname, uint8_t levels = 0);
void formatFS();

// JSON Helper Functions
bool loadJsonFile(const char *filename, JsonDocument &doc);
bool saveJsonFile(const char *filename, const JsonDocument &doc);

// Application Specific Settings
bool loadAllSettings();
bool saveWifiModeSetting();

#endif