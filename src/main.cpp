#include <Arduino.h>

#pragma region "Bluetooth"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>



// BLE UUIDs
#define GENERIC_DISPLAY 0x1811
#define SERVICE_180A_UUID   "0000180a-0000-1000-8000-00805f9b34fb"
#define SERVICE_CCCC_UUID   "0000CCCC-CCCC-CCCC-1337-00805f9b34fb"
static const BLEUUID ALERT_DISPLAY_SERVICE_UUID = BLEUUID("3db02924-b2a6-4d47-be1f-0f90ad62a048");
static const BLEUUID DISPLAY_MESSAGE_CHARACTERISTIC_UUID = BLEUUID("8d8218b6-97bc-4527-a8db-13094ac06b1d");

// #define BUFF_LEN 140
// char* payload = new char[BUFF_LEN];

class MyServerCallback: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    //todo display `connected` on the screen
    Serial.println("LE onConnect");
    // memset(payload, 0x00, BUFF_LEN);
    /*connected = true;
    hasLongText = false;
    hasText = false;
    hasTimeText = false;
    yScrollPos = 0;*/
  }

  void onDisconnect(BLEServer* pServer) {
    // todo display `disconnected` on the screen
    Serial.println("LE onDisconnect");
    //connected = false;
  }
};
void emulatePOCSAG(const char* msg);
class DisplayCharacteristicCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    // write text to OLED
    std::string str = pCharacteristic->getValue();
    BLEUUID uuid = pCharacteristic->getUUID();
    std::string uuid_s = uuid.toString();
    Serial.print("onWrite() {uuid=");
    Serial.print(String(uuid_s.c_str()));
    Serial.print(",len=");
    Serial.print(str.length());
    Serial.println("}");
    Serial.println(str.c_str());

    emulatePOCSAG(str.c_str());
  }
};

void setupBT() {
  BLEDevice::init("CC-Bluetooth-Pager");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallback());
  
  BLEService *pServiceDeviceInfo = pServer->createService(SERVICE_180A_UUID);
  BLECharacteristic *pCharacteristicDeviceInfo_ManufacturerName = new BLECharacteristic((uint16_t)0x2A29,BLECharacteristic::PROPERTY_READ);
  pCharacteristicDeviceInfo_ManufacturerName->setValue("CuddlyCheetah");
  pServiceDeviceInfo->addCharacteristic(pCharacteristicDeviceInfo_ManufacturerName);
  pServiceDeviceInfo->start();

  BLEService *pService = pServer->createService(SERVICE_CCCC_UUID);
  BLECharacteristic *pCharacteristicText = new BLECharacteristic(DISPLAY_MESSAGE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE_NR); //Request MTU=500 from client
  pCharacteristicText->setCallbacks(new DisplayCharacteristicCallback());
  pService->addCharacteristic(pCharacteristicText);;
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(GENERIC_DISPLAY); //Generic Display
  pAdvertising->setScanResponse(true);
  /*pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);*/
  pAdvertising->start();
  Serial.println("Waiting a client connection to notify...");
  BLEDevice::startAdvertising();
}
#pragma endregion

#pragma region "POCSAG"
#include <cuddlycheetah.pocsag.h>
#define BAUDPAUSE 1000000 / 1200
#define POCSAG_PIN 19
#define FUNCTION_BITS 3
#define CAPCODE 924395
#define IS_NUMERIC true
#define INVERTED false

void setupPOCSAG() {
  pinMode(POCSAG_PIN, OUTPUT);
  digitalWrite(POCSAG_PIN, 1);
}
// to simulate some activity
void emulatePOCSAG(const char* msg) {
  char* message = (char*)msg;

  Serial.print("emulatePOCSAG "); Serial.println(message);
  Serial.print("length="); Serial.println(strlen(message));
  size_t messageLength = IS_NUMERIC
    ? numericMessageLength(CAPCODE, strlen(message))
    : textMessageLength(CAPCODE, strlen(message));

  uint32_t* transmission = (uint32_t*) malloc(sizeof(uint32_t) * messageLength+2);
	int Sym=0;
  // https://raw.githubusercontent.com/nkolban/ESP32_BLE_Arduino/master/examples/BLE_server_multiconnect/BLE_server_multiconnect.ino
  encodeTransmission(IS_NUMERIC, CAPCODE, FUNCTION_BITS, message, strlen(message), transmission);
	char *pocsagData=(char *)malloc(messageLength*32 + 1);

  //Serial.println("generating");
  for (int i = 0;i < messageLength; i++) {
    if (!INVERTED) transmission[i] = ~transmission[i];
    for (int j = 31; j >= 0; j--) {
      pocsagData[Sym] = (transmission[i] >> j) & 0x1 == 1 ? '1' : '0';
      Sym++;
    }
  }
  pocsagData[Sym] = 0x00;
  for (const char* p = pocsagData; *p; p++) {
    bool bit = (*p == '1');
    Serial.print(bit ? '1' :'0');
  }
  for (const char* p = pocsagData; *p; p++) {
    bool bit = (*p == '1');
    digitalWrite(POCSAG_PIN, bit);
    delayMicroseconds(1000000 / 1200);
  }
  /*Serial.println("");
  Serial.println("transmitting");
  for (int i = 0; i < messageLength * 32; i++) {
    digitalWrite(POCSAG_PIN, TabSymbol[i]);
    Serial.print(TabSymbol[i] ? '1' : '0');
    delayMicroseconds(BAUDPAUSE);
  }*/
  delayMicroseconds(BAUDPAUSE);
  digitalWrite(POCSAG_PIN, 1);
}
void fakePOCSAG() {
  for (int i = 0; i < PREAMBLE_LENGTH / 32; i++) {
    digitalWrite(POCSAG_PIN, i % 2 == 0);
    delayMicroseconds(BAUDPAUSE);
  }
  delayMicroseconds(BAUDPAUSE);
  digitalWrite(POCSAG_PIN, 1);
}
#pragma endregion


void setup() {
  Serial.begin(57600);
  Serial.println('Bluetooth Empfänger V1 - by <@cuddlycheetah>');
  setupBT();
  setupPOCSAG();
  emulatePOCSAG("42");
}

void loop() {
}