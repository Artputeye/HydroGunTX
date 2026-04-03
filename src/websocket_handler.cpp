#include "websocket_handler.h"

AsyncWebSocket ws("/ws");

// --- Configuration ---
#define SERIAL_BUFFER_SIZE 128
#define WS_UPDATE_INTERVAL 50   // 20Hz
#define WS_FORCE_SEND_MS 200

// --- System State ---
unsigned long ws_last_update_time = 0;
unsigned long ws_last_force_send = 0;
char serialBuffer[SERIAL_BUFFER_SIZE];
uint8_t serialIndex = 0;

struct HydroState {
    bool fire;
    int8_t updown;     // เปลี่ยนจาก int เป็น int8_t
    int8_t leftright;  // เปลี่ยนจาก int เป็น int8_t
    int rssi;
    int loss;
    bool espnow;
};
HydroState last_broadcasted_state = {0};

// --- Helper Functions ---
// ตรวจสอบการเปลี่ยนแปลงของสถานะ (State Tracking)
template <typename T>
bool hasStateChanged(T &oldVal, T newVal) {
    if (oldVal != newVal) {
        oldVal = newVal;
        return true;
    }
    return false;
}

// --- Core Logic ---

void ws_broadcast_telemetry() {
    if (ws.count() == 0 || ws.availableForWriteAll() == 0) return;

    bool forceUpdate = (millis() - ws_last_force_send > WS_FORCE_SEND_MS);
    String json = "{";
    bool firstEntry = true;

    // Lambda สำหรับช่วยต่อ JSON String
    auto appendJson = [&](const String &key, const String &value) {
        if (!firstEntry) json += ",";
        json += "\"" + key + "\":" + value;
        firstEntry = false;
    };

    // ตรวจสอบการเปลี่ยนแปลงของปุ่มกดยิงและจอยสติ๊ก
    if (hasStateChanged(last_broadcasted_state.fire, data.fire) || forceUpdate)
        appendJson("fire", data.fire ? "true" : "false");

    if (hasStateChanged(last_broadcasted_state.updown, data.updown) || forceUpdate)
        appendJson("updown", String(data.updown));

    if (hasStateChanged(last_broadcasted_state.leftright, data.leftright) || forceUpdate)
        appendJson("leftright", String(data.leftright));

    // ข้อมูลระบบ ส่งตามรอบ Force Send เท่านั้นเพื่อลด Traffic
    if (forceUpdate) {
        appendJson("rssi", String(espnowRSSI));
        appendJson("loss", String(packetLoss));
        appendJson("espnow", espnowConnected ? "true" : "false");
        ws_last_force_send = millis();
    }

    json += "}";
    
    if (!firstEntry) { // ส่งเฉพาะเมื่อมีข้อมูลเปลี่ยน
        ws.textAll(json);
    }
}

void ws_broadcast_serial(const char *msg) {
    if (msg && msg[0] != '\0' && ws.count() > 0) {
        String json = "{\"Serial\":\"";
        json += msg;
        json += "\"}";
        ws.textAll(json);
    }
}

void ws_handle_serial_bridge() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\r') continue;

        if (c == '\n') {
            serialBuffer[serialIndex] = '\0';
            // สะท้อนค่าออก Serial Monitor และส่งไปที่ WebSocket Dashboard
            Serial.println(serialBuffer); 
            ws_broadcast_serial(serialBuffer);
            serialIndex = 0;
        } 
        else if (serialIndex < SERIAL_BUFFER_SIZE - 1) {
            serialBuffer[serialIndex++] = c;
        } 
        else {
            serialIndex = 0; // Buffer Overflow Reset
        }
    }
}

// --- WebSocket Events ---

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("[WS] Client %u connected\n", client->id());
    } 
    else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("[WS] Client %u disconnected\n", client->id());
    }
}

// --- Public API ---

void ws_init() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    Serial.println(F("[WS] WebSocket Server Initialized"));
}

void ws_process() {
    // 1. จัดการข้อมูลจาก Serial
    ws_handle_serial_bridge();

    // 2. จัดการส่งข้อมูล Telemetry ตามรอบเวลา
    if (millis() - ws_last_update_time > WS_UPDATE_INTERVAL) {
        ws_broadcast_telemetry();
        ws_last_update_time = millis();
    }

    // 3. คืนค่า Memory ของ Client ที่หลุดไป
    ws.cleanupClients();
}