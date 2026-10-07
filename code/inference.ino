#include <Seeed_Arduino_SSCMA.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

SSCMA AI;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID        "cafe0001-0000-0000-0000-123456789abc"
#define CHARACTERISTIC_UUID "cafe0002-0000-0000-0000-123456789abc"

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer)  { deviceConnected = true;  }
  void onDisconnect(BLEServer* pServer){ deviceConnected = false; }
};

void setup() {
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  Serial.println("Booting...\n");

  // Initialize AI camera
  AI.begin();

  // Initialize BLE
  BLEDevice::init("XIAO_ESP32C3_Sender");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ   |
      BLECharacteristic::PROPERTY_WRITE  |
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_INDICATE
  );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE advertising started!");
}

void loop() {
  if (!AI.invoke()) {

    char data[64];           // Enough for several detections
    data[0] = '\0';          // Start as empty string

    auto boxes = AI.boxes();

    if (boxes.size() == 0) {
      // No detections = send "0"
      strcpy(data, "0");
    } 
    else {
      // Append each x-value
      for (int i = 0; i < boxes.size(); i++) {
        float xVal = boxes[i].x;

        // Safe append: snprintf writes only until buffer full
        snprintf(
          data + strlen(data), 
          sizeof(data) - strlen(data),
          "%.2f\n",
          xVal
        );
      }
    }

    Serial.print("Sending: ");
    Serial.println(data);

    if (deviceConnected) {
      pCharacteristic->setValue((uint8_t*)data, strlen(data));
      pCharacteristic->notify();
      Serial.println("Sent x-values over BLE");
    }

    delay(250);
  }
}
