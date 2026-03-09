#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// ---------------- BLE ----------------
static BLEUUID serviceUUID("72e8ce72-67f1-4114-8412-1b0e1b35e0b1");
static BLEUUID charUUID("59ce0b83-f1c9-4229-9227-c6bd61ca7797");

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// ---------------- LED ----------------
#define LED_PIN D0

unsigned long ledTimer = 0;
bool ledState = false;

unsigned long lastNotifyTime = 0;

// ---------------- Device States ----------------
enum DeviceMode {
  MODE_SCANNING,
  MODE_DISPLAYING
};

DeviceMode currentMode = MODE_SCANNING;

// ---------------- Stepper Motor ----------------
#define A1 9
#define A2 10
#define B1 8
#define B2 5

int stepIndex = 0;
int currentStep = 0;
int targetStep = 0;

float smoothEmotion = 0;

const uint8_t seq[4][4] = {
  {1,0,1,0},
  {0,1,1,0},
  {0,1,0,1},
  {1,0,0,1}
};

// ---------------- LED Update ----------------
void updateLED() {

  // 最近0.5秒有收到数据 → 常亮
  if (millis() - lastNotifyTime < 500) {
    digitalWrite(LED_PIN, HIGH);
    return;
  }

  // 否则慢闪
  if (millis() - ledTimer > 100) {

    ledTimer = millis();
    ledState = !ledState;

    digitalWrite(LED_PIN, ledState);
  }
}

// ---------------- Motor ----------------
void stepMotor(int dir)
{
  stepIndex = (stepIndex + dir + 4) % 4;

  digitalWrite(A1, seq[stepIndex][0]);
  digitalWrite(A2, seq[stepIndex][1]);
  digitalWrite(B1, seq[stepIndex][2]);
  digitalWrite(B2, seq[stepIndex][3]);

  delay(6);
}

// ---------------- BLE Notify ----------------
static void notifyCallback(
  BLERemoteCharacteristic*,
  uint8_t* pData,
  size_t length,
  bool)
{
  if (length == 1) {

    lastNotifyTime = millis();

    uint8_t value = pData[0];

    smoothEmotion = smoothEmotion * 0.85 + value * 0.15;

    int angle = map((int)smoothEmotion, 0, 100, -60, 60);

    Serial.print("Emotion: ");
    Serial.print(value);
    Serial.print(" Smoothed: ");
    Serial.print(smoothEmotion);
    Serial.print(" → Angle: ");
    Serial.println(angle);

    targetStep = map(angle, -60, 60, 80, 420);
  }
}

// ---------------- BLE Client Callbacks ----------------
class MyClientCallback : public BLEClientCallbacks {

  void onConnect(BLEClient* pclient) {}

  void onDisconnect(BLEClient* pclient) {

    Serial.println("Disconnected → back to SCANNING");

    connected = false;
    currentMode = MODE_SCANNING;
  }
};

// ---------------- BLE Scan Callback ----------------
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  void onResult(BLEAdvertisedDevice advertisedDevice) {

    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(serviceUUID)) {

      Serial.println("Sensing device found");

      BLEDevice::getScan()->stop();

      myDevice = new BLEAdvertisedDevice(advertisedDevice);

      doConnect = true;
    }
  }
};

// ---------------- Connect ----------------
bool connectToServer() {

  Serial.println("Connecting to sensing device...");

  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  if (!pClient->connect(myDevice)) {

    Serial.println("Connection failed");

    return false;
  }

  Serial.println("Connected");

  BLERemoteService* pRemoteService =
      pClient->getService(serviceUUID);

  if (pRemoteService == nullptr) {

    Serial.println("Service not found");

    pClient->disconnect();

    return false;
  }

  pRemoteCharacteristic =
      pRemoteService->getCharacteristic(charUUID);

  if (pRemoteCharacteristic->canNotify()) {

    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }

  connected = true;
  currentMode = MODE_DISPLAYING;

  Serial.println("Entering DISPLAYING mode");

  return true;
}

// ---------------- Setup ----------------
void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("Display device booting...");


  BLEDevice::init("");

  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(B1, OUTPUT);
  pinMode(B2, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  currentStep = 250;
  targetStep = 250;

  BLEScan* pBLEScan = BLEDevice::getScan();

  pBLEScan->setAdvertisedDeviceCallbacks(
      new MyAdvertisedDeviceCallbacks());

  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
}

// ---------------- Loop ----------------
void loop() {

  updateLED();

  // ---------- SCANNING ----------
  if (currentMode == MODE_SCANNING) {

    BLEDevice::getScan()->start(3, false);

    if (doConnect) {

      if (connectToServer()) {

        Serial.println("Connected to BLE Server");
      }

      doConnect = false;
    }

    delay(100);
  }

  // ---------- DISPLAYING ----------
  if (currentMode == MODE_DISPLAYING) {

    int diff = targetStep - currentStep;

    if (abs(diff) > 10) {

      int dir = (diff > 0) ? 1 : -1;

      for(int i=0;i<3;i++) {
      stepMotor(dir);
      currentStep += dir;
     }

      currentStep = constrain(currentStep, 80, 420);
    }
  }
}