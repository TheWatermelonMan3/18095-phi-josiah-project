#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

#define SERVICE_UUID        "cafe0001-0000-0000-0000-123456789abc"
#define CHARACTERISTIC_UUID "cafe0002-0000-0000-0000-123456789abc"

#define LED_PIN 13     // Status LED output on ESP32
#define BOOT_LED 2     // Onboard LED
#define PIN12 12       // Signal output 1
#define PIN14 14      // Signal output 2

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient)  { Serial.println("Connected to server"); }
  void onDisconnect(BLEClient* pClient){ Serial.println("Disconnected from server"); }
};

BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pCharacteristic = nullptr;
BLEAdvertisedDevice* target = nullptr;

void connectToServer() {
  if (target == nullptr) return;

  if (pClient) { delete pClient; pClient = nullptr; }

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  Serial.println("Connecting to server...");
  if (!pClient->connect(target)) {
    Serial.println("❌ Failed to connect. Retrying...");
    delay(2000);
    return;
  }

  BLERemoteService* pService = pClient->getService(SERVICE_UUID);
  if (!pService) {
    Serial.println("❌ Service not found!");
    pClient->disconnect();
    return;
  }

  pCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);
  if (!pCharacteristic) {
    Serial.println("❌ Characteristic not found!");
    pClient->disconnect();
    return;
  }

  Serial.println("✅ Connected and ready!");
  delay(100);
}

void setup() {
  // Boot pulse
  pinMode(BOOT_LED, OUTPUT);
  digitalWrite(BOOT_LED, HIGH);
  delay(300);
  digitalWrite(BOOT_LED, LOW);

  Serial.begin(115200);
  Serial.println("Starting BLE Client...");

  // Setup output pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIN12, OUTPUT);
  pinMode(PIN14, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(PIN12, LOW);
  digitalWrite(PIN14, LOW);

  BLEDevice::init("ESP32_WROOM_Client");
  delay(500);

  BLEScan* pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);

  while (target == nullptr) {
    Serial.println("🔍 Scanning for sender...");
    BLEScanResults results = *pScan->start(5, false);

    for (int i = 0; i < results.getCount(); i++) {
      BLEAdvertisedDevice d = results.getDevice(i);
      if (d.getName() == "XIAO_ESP32C3_Sender") {
        target = new BLEAdvertisedDevice(d);
        Serial.println("✅ Found sender!");
        break;
      }
    }

    if (!target) {
      Serial.println("Sender not found. Retrying...");
      delay(2000);
    }
  }

  connectToServer();
}

void loop() {
  if (!pClient || !pClient->isConnected()) {
    Serial.println("Reconnecting...");
    connectToServer();
    delay(2000);
    return;
  }

  if (!pCharacteristic) {
    Serial.println("⚠️ No characteristic");
    delay(500);
    return;
  }

  // Read BLE data
  String value = pCharacteristic->readValue();
  Serial.print("📩 Received: ");
  Serial.println(value);

  int x = value.toInt();
  x += 20;

  // Status LED
  digitalWrite(LED_PIN, (x > 1) ? HIGH : LOW);

  // ---- Direction Control Logic ----

  if (x < 100) {
    // Turn LEFT → activate ONLY pin 12
    digitalWrite(PIN12, HIGH);
    digitalWrite(PIN14, LOW);
  }
  else if (x >= 100 && x <= 140) {
    // Go STRAIGHT → activate BOTH pins
    digitalWrite(PIN12, HIGH);
    digitalWrite(PIN14, HIGH);
  }
  else if (x > 140) {
    // Turn RIGHT → activate ONLY pin 13
    digitalWrite(PIN12, LOW);
    digitalWrite(PIN14, HIGH);
  }

  delay(250);
}
