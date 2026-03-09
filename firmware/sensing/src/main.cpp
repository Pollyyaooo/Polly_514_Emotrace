#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ---------------- BLE ----------------
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;

#define SERVICE_UUID        "72e8ce72-67f1-4114-8412-1b0e1b35e0b1"
#define CHARACTERISTIC_UUID "59ce0b83-f1c9-4229-9227-c6bd61ca7797"

// ---------------- GSR ----------------
#define GSR_PIN A1

int rawGsr = 0;
int baseline = 0;
int emotionValue = 0;
float smoothEmotion = 0;

// ---------------- Timing ----------------
unsigned long previousMillis = 0;
unsigned long sampleInterval = 1000;

// ---------------- States ----------------
enum DeviceMode {
  MODE_MONITORING,
  MODE_ACTIVE
};

DeviceMode currentMode = MODE_MONITORING;

// 接触检测阈值
const int CONTACT_THRESHOLD = 2500;

// ---------------- BLE Callbacks ----------------
class MyServerCallbacks: public BLEServerCallbacks {

  void onConnect(BLEServer* pServer) {

    deviceConnected = true;

    Serial.println("Client connected");
  }

  void onDisconnect(BLEServer* pServer) {

    deviceConnected = false;

    Serial.println("Client disconnected");

    BLEDevice::startAdvertising(); // reconnect 必须
  }
};

// ---------------- Setup ----------------
void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println("Sensing device booting...");

  pinMode(GSR_PIN, INPUT);

  baseline = analogRead(GSR_PIN);

  // -------- BLE 初始化（只一次） --------
  BLEDevice::init("EmoSense");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->addDescriptor(new BLE2902());

  uint8_t initValue = 0;
  pCharacteristic->setValue(&initValue, 1);

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started.");

  currentMode = MODE_MONITORING;

  Serial.println("Entering MONITORING mode");
}

// ---------------- Loop ----------------
void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= sampleInterval) {

    previousMillis = currentMillis;

    rawGsr = analogRead(GSR_PIN);

    Serial.print("Raw: ");
    Serial.print(rawGsr);

    // ---------------- MONITORING ----------------
    if (currentMode == MODE_MONITORING) {

      Serial.println(" | Mode: MONITORING");

      if (rawGsr < CONTACT_THRESHOLD) {

        Serial.println("Contact detected → switching to ACTIVE");

        currentMode = MODE_ACTIVE;

        baseline = rawGsr;

        sampleInterval = 50;
      }

      return;
    }

    // ---------------- ACTIVE ----------------
    if (currentMode == MODE_ACTIVE) {

      // baseline slowly adapts
      baseline = baseline * 0.999 + rawGsr * 0.001;

      int delta = abs(rawGsr - baseline);
      if (delta < 5) delta = 0;

      int rawEmotion = map(delta, 0, 250, 0, 100);
      rawEmotion = constrain(rawEmotion, 0, 100);

      smoothEmotion = smoothEmotion * 0.8 + rawEmotion * 0.2;

      emotionValue = (int)smoothEmotion;

      Serial.print(" | Baseline: ");
      Serial.print(baseline);

      Serial.print(" | Emotion: ");
      Serial.print(emotionValue);

      Serial.println(" | Mode: ACTIVE");

      // 发送数据
      if (deviceConnected) {

        uint8_t data = emotionValue;

        pCharacteristic->setValue(&data, 1);
        pCharacteristic->notify();

        Serial.print("Notify: ");
        Serial.println(emotionValue);
      }

      // 如果检测不到接触 → 回到 monitoring
      if (rawGsr > CONTACT_THRESHOLD) {

        Serial.println("Contact lost → back to MONITORING");

        currentMode = MODE_MONITORING;

        sampleInterval = 1000;
      }
    }
  }
}