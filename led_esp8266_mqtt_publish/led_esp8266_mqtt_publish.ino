
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "HUAWEI-802-A";
const char* password = "y781881228";

const char* mqttServer = "test.ranye-iot.net";

int delayTime = 1000;  // 1m

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


const int rLedPin = 15;  //引脚R D8
const int gLedPin = 12;  //引脚G D6
const int bLedPin = 4;  //引脚B  D2

void setup() {

  pinMode(rLedPin, OUTPUT);
  pinMode(gLedPin, OUTPUT);
  pinMode(bLedPin, OUTPUT);

  // put your setup code here, to run once:
  Serial.begin(115200);

  // 设置无线终端模式
  WiFi.mode(WIFI_STA);
  // 连接WiFi
  connectWifi();

  // 设置要连接的服务器及端口
  mqttClient.setServer(mqttServer, 1883);
  // 连接mqtt服务器
  connectMQTTServer();
}

void connectMQTTServer() {
  String clientId = "esp8266-" + WiFi.macAddress();

  if (mqttClient.connect(clientId.c_str())) {
    Serial.println("Connect Success. ");
    Serial.print("Server Address: ");
    Serial.println(mqttServer);
    Serial.print("ClientId: ");
    Serial.println(clientId);
  } else {
    Serial.println("Connect Failure.");
    Serial.print("Client State: ");
    Serial.println(mqttClient.state());
    delay(3000);
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.println();
}

void light(int pin, int value) {
  analogWrite(pin, value);
}

void sendCode(bool r, bool g, bool b, int i) {
  // 根据rgb和 i，组成下列指令
  byte high = 0x01;
  if (r) {
    high = high << 2;
  } else if (g) {
    high = high << 1;
  }
  // high = high << 8;
  byte low = (byte)i;

    // 待发布的信息H4L255

  // 0100(0x0400 ~ 0x04FF), 0010(0x0200 ~0x02FF), 0001(0x0100 ~ 0x01FF)
  // 确定发布的主题：
  String topic = "sos";
  char publishTopic[topic.length() + 1];
  strcpy(publishTopic, topic.c_str());

  char publishMessage[] = { 'H', high, 'L', low, 'A', '\0' };
  

  if (mqttClient.connected()) {

    if (mqttClient.publish(publishTopic, publishMessage)) {
      Serial.print("Topic: ");
      Serial.println(publishTopic);
      Serial.print("Message: ");
      
      String value = "H" + String(high) + "L" + String(low);
      Serial.println(value);
      Serial.println(publishMessage);
    } else {
      Serial.println("Publish Failed");
    }

    // 保持客户端心跳
    mqttClient.loop();

  } else {
    connectMQTTServer();
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
}

/**
 * 数值增大，亮度减弱  注意：i = 0x00时，没法正常传输，需要转义
 */
void increase(bool r, bool g, bool b) {
  for (int i = 1; i < 256; i += 255) {
    sendCode(r, g, b, i);
    delay(delayTime);
  }
}

/**
 * 数值减小，亮度增强
 */
void decrease(bool r, bool g, bool b) {
  for (int i = 255; i > 0; i -= 255) {
    sendCode(r, g, b, i);
    delay(delayTime);
  }
}

// client32 发布指令：

void loop() {

  // 业务操作
  decrease(true, false, false);
  increase(true, false, false);
  decrease(false, true, false);
  increase(false, true, false);
  decrease(false, false, true);
  increase(false, false, true);
}