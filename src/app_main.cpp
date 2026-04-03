#include "app_main.h"

#define PIN_LEFTRIGHT 39
#define PIN_UPDOWN 35
#define PIN_FIRE 25

#define DEADZONE 180
#define SEND_INTERVAL 30 // max rate ~33Hz

ControlPacket data;
ControlPacket lastData;

unsigned long lastSend = 0;

bool fireState = false;      // สถานะ toggle จริง
bool lastButtonState = HIGH; // สถานะปุ่มก่อนหน้า

// แยก filter ของแต่ละ pin
float leftrightFiltered = 0;
float updownFiltered = 0;

bool espnowConnected = false;
int espnowRSSI = -60;
int packetLoss = 0;

unsigned long lastEspNowOK = 0;
const unsigned long espnowTimeout = 1000; // 1 วินาที

int centerLR = 0;
int centerUD = 0;

void calibrateCenter()
{
    for (int i = 0; i < 50; i++)
    {
        centerLR += analogRead(PIN_LEFTRIGHT);
        centerUD += analogRead(PIN_UPDOWN);
        delay(10);
    }
    centerLR /= 50;
    centerUD /= 50;

    Serial.printf("CENTER LR: %d | UD: %d\n", centerLR, centerUD);
}

void onEspNowSend(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        espnowConnected = true;
        lastEspNowOK = millis();
    }
}

int readFiltered(int pin, float &filtered)
{
    int raw = analogRead(pin);
    filtered = filtered * 0.7 + raw * 0.3;
    return (int)filtered;
}

int8_t smooth(int8_t in, float &f)
{
    f = f * 0.85 + in * 0.15;
    return (int8_t)f;
}

int8_t convertAxis(int value, int center)
{
    int minVal = 0;
    int maxVal = 4095;

    // deadzone
    if (abs(value - center) < 80)
        return 0;

    value = constrain(value, minVal, maxVal);

    float norm;

    if (value >= center)
        norm = (float)(value - center) / (maxVal - center);
    else
        norm = (float)(value - center) / (center - minVal);

    norm = constrain(norm, -1.0, 1.0);

    // expo
    float expo = 0.8;
    float out = norm * (1 - expo) + pow(norm, 3) * expo;

    return (int8_t)(out * 100);
}

void setupIO()
{

    pinMode(PIN_FIRE, INPUT_PULLUP);
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_LEFTRIGHT, ADC_11db);
    analogSetPinAttenuation(PIN_UPDOWN, ADC_11db);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void app_setup()
{
    setupIO();
    calibrateCenter();
    Serial.println("⚡ ESP-NOW init");

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("❌ ESP-NOW init failed");
        return;
    }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, MAC_RX, 6);

    peer.channel = WiFi.channel();

    peer.encrypt = false;
    peer.ifidx = (isWifiApMode == 0) ? WIFI_IF_AP : WIFI_IF_STA;

    if (esp_now_add_peer(&peer) != ESP_OK)
    {
        Serial.println("❌ Peer add failed");
        return;
    }

    Serial.println("✅ ESP-NOW ready");
}

int readStable(int pin)
{
    int sum = 0;
    for (int i = 0; i < 10; i++)
    {
        sum += analogRead(pin);
        delay(2); // หน่วงเล็กน้อย
    }
    return sum / 10;
}

void app_loop()
{
    // 1. อ่านค่า Sensor
    // 1. อ่าน + convert
    int8_t lr = convertAxis(
        readFiltered(PIN_LEFTRIGHT, leftrightFiltered),
        centerLR);

    int8_t ud = convertAxis(
        readFiltered(PIN_UPDOWN, updownFiltered),
        centerUD);

    // 2. 🔥 smooth ตรงนี้
    static float lrSmooth = 0;
    static float udSmooth = 0;

    data.leftright = smooth(lr, lrSmooth);
    data.updown = -smooth(ud, udSmooth);

    bool currentButton = digitalRead(PIN_FIRE);

    // ส่วนของการจัดการปุ่ม Fire (Toggle)
    if (lastButtonState == HIGH && currentButton == LOW)
    {
        fireState = !fireState;
        Serial.print("TOGGLE! Status: ");
        Serial.println(fireState ? "ON" : "OFF"); // แสดงผลเป็นข้อความ ON/OFF แทนตัวเลข
    }
    lastButtonState = currentButton;
    data.fire = fireState;

    // 2. ตรวจสอบว่า "ข้อมูลชุดใหม่" ต่างจาก "ข้อมูลชุดเดิม" หรือไม่
    // และเช็คเรื่อง Interval ของการส่งด้วย
    if (memcmp(&data, &lastData, sizeof(data)) != 0 &&
        millis() - lastSend >= SEND_INTERVAL)
    {
        // --- ส่วนที่เพิ่มเข้ามาสำหรับ Serial Print เฉพาะตอนเปลี่ยน ---
        Serial.print("Data Changed -> UD: ");
        Serial.print(data.updown);
        Serial.print(" | LR: ");
        Serial.print(data.leftright);
        Serial.print(" | Fire: ");
        Serial.println(data.fire ? "ON" : "OFF");
        // ---------------------------------------------------

        // ส่งข้อมูลผ่าน ESP-NOW
        esp_now_send(MAC_RX, (uint8_t *)&data, sizeof(data));

        // อัปเดตข้อมูลล่าสุดเพื่อใช้เปรียบเทียบในรอบถัดไป
        lastData = data;
        lastSend = millis();
    }

    // ระบบตรวจสอบ Timeout ของ ESP-NOW
    if (millis() - lastEspNowOK > espnowTimeout)
    {
        espnowConnected = false;
    }
}

void updateSystemStatus()
{
}