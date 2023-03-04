
#include <BLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>

static BLEUUID serviceUUID("0000ffe0-0000-1000-8000-00805f9b34fb");  // 0000fff0-0000-1000-8000-00805f9b34fb / 5ed11da3-f50e-e90c-fbde-019c623e7cc3
static BLEUUID charUUID("0000ffe1-0000-1000-8000-00805f9b34fb");

// 蓝牙广播设备
static BLEAdvertisedDevice* targetAdvertisedDevice;
static BLERemoteCharacteristic* bleRemoteCharacteristic;

// 是否找到目标蓝牙设备
static boolean findBLEDevice = false;
static boolean connect2BLEDevice = false;

const char* ssid = "HUAWEI-802-A";
const char* password = "y781881228";

const char* mqttServer = "test.ranye-iot.net";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const int rLedPin = 5;  // D5 5
const int gLedPin = 4;  // D4 4
const int bLedPin = 2;  // D2 2

int delayTime = 1000;

void setup() {
  ledcSetup(rLedPin, 5000, 8);
  ledcSetup(gLedPin, 5000, 8);
  ledcSetup(bLedPin, 5000, 8);
  ledcAttachPin(rLedPin, rLedPin);
  ledcAttachPin(gLedPin, gLedPin);
  ledcAttachPin(bLedPin, bLedPin);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  connectWifi();

  mqttClient.setServer(mqttServer, 1883);

  mqttClient.setCallback(receiveCallback);


  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // 扫描目标蓝牙设备
  findBLEDeviceByScan();

  connectMQTTServer();
}

void connectMQTTServer() {
  String clientId = "esp32-" + WiFi.macAddress();

  if (mqttClient.connect(clientId.c_str())) {
    Serial.println(F("MQTT Connect Success. "));
    Serial.print(F("MQTT Server Address: "));
    Serial.println(mqttServer);
    Serial.print(F("MQTT ClientId: "));
    Serial.println(clientId);
    // 订阅主题
    subscribeTopic();
  } else {
    Serial.println(F("MQTT Connect Failure. "));
  }
}

void subscribeTopic() {
  String topic = "sos";
  char subTopic[topic.length() + 1];
  strcpy(subTopic, topic.c_str());

  if (mqttClient.subscribe(subTopic)) {
    Serial.print(F("MQTT Subscribe Topic: "));
    Serial.println(subTopic);
  } else {
    Serial.println(F("MQTT Subscribe Failure"));
  }
}

void receiveCallback(char* topic, byte* payload, unsigned int length) {
  byte high = (byte)payload[1];
  byte low = (byte)payload[3];

  if (connect2BLEDevice) {
    String value = "H" + String(high) + "L" + String(low);
    bleRemoteCharacteristic->writeValue(value.c_str(), value.length());
    Serial.println("BLE send to \"" + value + "\"");
  }

  if (high == 0x04) {
    // 红色
    light(rLedPin, low);
    light(gLedPin, 255);
    light(bLedPin, 255);
  } else if (high == 0x02) {
    // 绿色
    light(rLedPin, 255);
    light(gLedPin, low);
    light(bLedPin, 255);
  } else if (high == 0x01) {
    // 蓝色
    light(rLedPin, 255);
    light(gLedPin, 255);
    light(bLedPin, low);
  }

  // 通过Serial.println 把需要传输的内容发送出去
}

void connectWifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(F("WiFi connected!"));
  Serial.println();
}

void light(int pin, int value) {
  ledcWrite(pin, value);
}



class BLEAdvertisedDeviceCallback : public BLEAdvertisedDeviceCallbacks {


  void onResult(BLEAdvertisedDevice advertisedDevice) {

    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // 找到目标ServiceUUID
    if (advertisedDevice.haveName() && advertisedDevice.getName() == "HC-08" && advertisedDevice.haveServiceUUID()) {  // && advertisedDevice.isAdvertisingService(serviceUUID)

      // 停止扫描
      BLEDevice::getScan()->stop();

      if (advertisedDevice.isAdvertisingService(serviceUUID)) {
        Serial.println("设备广播了UUID为 serviceUUID 的服务");
      }
      Serial.printf("Address: %s\r\n", advertisedDevice.getAddress().toString().c_str());
      if (advertisedDevice.haveAppearance()) {
        Serial.printf("Appearance: %d\r\n", advertisedDevice.getAppearance());
      }
      if (advertisedDevice.haveManufacturerData()) {
        Serial.printf("ManufacturerData: %s\r\n", advertisedDevice.getManufacturerData().c_str());
      }
      if (advertisedDevice.haveRSSI()) {
        Serial.printf("RSSI: %d\r\n", advertisedDevice.getRSSI());
      }
      if (advertisedDevice.haveServiceData()) {
        Serial.printf("ServiceData: %s\r\n", advertisedDevice.getServiceData().c_str());
        Serial.printf("ServiceDataUUID: %s\r\n", advertisedDevice.getServiceDataUUID().toString().c_str());
        Serial.printf("ServiceDataCount: %d\r\n", advertisedDevice.getServiceDataCount());
        Serial.printf("ServiceDataUUIDCount: %d\r\n", advertisedDevice.getServiceDataUUIDCount());
      }
      if (advertisedDevice.haveServiceUUID()) {
        Serial.printf("ServiceUUID: %s\r\n", advertisedDevice.getServiceUUID().toString().c_str());
        Serial.printf("ServiceUUIDCount: %d\r\n", advertisedDevice.getServiceUUIDCount());
      }
      if (advertisedDevice.haveTXPower()) {
        Serial.printf("TXPower: %d\r\n", advertisedDevice.getTXPower());
      }

      targetAdvertisedDevice = new BLEAdvertisedDevice(advertisedDevice);
      findBLEDevice = true;
    }
  }
};

// 扫描目标蓝牙设备，找到后停止扫描
void findBLEDeviceByScan() {
  BLEScan* bleScan = BLEDevice::getScan();
  // 设置广播设备回调，找到目标设备
  bleScan->setAdvertisedDeviceCallbacks(new BLEAdvertisedDeviceCallback());
  // 设置扫描的间隔时间
  bleScan->setInterval(1349);
  // 设置窗口
  bleScan->setWindow(449);
  // 设置激活扫描
  bleScan->setActiveScan(true);
  // 开始扫描
  bleScan->start(5, false);
}


class BLEClientConnectCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* bleClient) {
    Serial.println(F("ble connected"));
    connect2BLEDevice = true;
  }

  void onDisconnect(BLEClient* bleClient) {
    Serial.println(F("ble disconnect"));
    connect2BLEDevice = false;
  }
};

/**
 * 扫码并连接HC-08(ble)
 */
bool connectToBLEServer() {
  // 1. 创建蓝牙客户端
  BLEClient* bleClient = BLEDevice::createClient();
  // 2. 设置连接状态回调
  bleClient->setClientCallbacks(new BLEClientConnectCallback());
  // 3. 连接扫描到的蓝牙广播设备(remove server)
  bleClient->connect(targetAdvertisedDevice);

  // 4. 在远程BLE服务器中获取我们所追求的服务的引用。
  BLERemoteService* bleRemoteService = bleClient->getService(serviceUUID);
  if (bleRemoteService == nullptr) {
    Serial.println(F("Failed to find serviceUUID"));
    bleClient->disconnect();
    return false;
  }
  Serial.println(F("Successed to find serviceUUID"));

  bleRemoteCharacteristic = bleRemoteService->getCharacteristic(charUUID);
  if (bleRemoteCharacteristic == nullptr) {
    Serial.println(F("Failed to find charUUID"));
    bleClient->disconnect();
    return false;
  }
  Serial.println(F("Successed to find charUUID"));

  if (bleRemoteCharacteristic->canRead()) {
    std::string value = bleRemoteCharacteristic->readValue();
    Serial.print(F("read value: "));
    Serial.println(value.c_str());
  }

  if (bleRemoteCharacteristic->canNotify()) {
    // 注册通知接收回调
    bleRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  Serial.println(F("BLE Connect Success!!!"));
  return true;
}

static void notifyCallback(BLERemoteCharacteristic* bleRemoteCharacteristic, uint8_t* data, size_t length, bool isNotify) {
  Serial.println(F("Notify callback: ---> "));
  Serial.print(F("characteristic: "));
  Serial.println(bleRemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(F("length: "));
  Serial.println(length);
  Serial.print(F("data: "));
  Serial.println((char*)data);
  Serial.println(F(" <--- end"));
}

void loop() {

  if (mqttClient.connected()) {
    mqttClient.loop();
  } else {
    connectMQTTServer();
  }

  if (findBLEDevice && !connect2BLEDevice) {
    // 通过回调扫描到目标蓝牙设备后，再进行连接到目标蓝牙服务
    if (connectToBLEServer()) {
      connect2BLEDevice = true;
    }
  }
}
