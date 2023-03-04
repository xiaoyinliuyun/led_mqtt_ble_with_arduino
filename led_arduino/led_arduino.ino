/*
 * 接收端：接收蓝牙串口输入：触发两个led灯按要求闪烁
 */
const int rLedPin = 6;  //引脚R
const int gLedPin = 5;  //引脚G
const int bLedPin = 3;  //引脚B


// const int rLedPin2 = 11;  //引脚R
// const int gLedPin2 = 10;  //引脚G
// const int bLedPin2 = 9;   //引脚B

// const int btnPin = 2;  // 引脚2


// int ledR = 255;  //R Led 亮度
// int ledG = 255;  //G Led 亮度
// int ledB = 0;    //B Led 亮度

void setup() {
  pinMode(rLedPin, OUTPUT);
  pinMode(gLedPin, OUTPUT);
  pinMode(bLedPin, OUTPUT);


  // pinMode(rLedPin2, OUTPUT);
  // pinMode(gLedPin2, OUTPUT);
  // pinMode(bLedPin2, OUTPUT);

  // pinMode(btnPin, INPUT_PULLUP);

  Serial.begin(115200);
}


// int delayTime = 1970;

// void increase(bool r, bool g, bool b, bool r2, bool g2, bool b2) {
//   for (int i = 0; i < 256; i++) {
//     analogWrite(rLedPin, r ? i : 255);
//     analogWrite(gLedPin, g ? i : 255);
//     analogWrite(bLedPin, b ? i : 255);

//     analogWrite(rLedPin2, r2 ? i : 255);
//     analogWrite(gLedPin2, g2 ? i : 255);
//     analogWrite(bLedPin2, b2 ? i : 255);

//     delayMicroseconds(delayTime);
//   }
// }

// void decrease(bool r, bool g, bool b, bool r2, bool g2, bool b2) {
//   for (int i = 255; i >= 0; i--) {
//     analogWrite(rLedPin, r ? i : 255);
//     analogWrite(gLedPin, g ? i : 255);
//     analogWrite(bLedPin, b ? i : 255);

//     analogWrite(rLedPin2, r2 ? i : 255);
//     analogWrite(gLedPin2, g2 ? i : 255);
//     analogWrite(bLedPin2, b2 ? i : 255);

//     delayMicroseconds(delayTime);
//   }
// }

void loop() {
  // 有延时问题 :解决 https://blog.csdn.net/shenfuwang/article/details/80262216
  // Serial.println(Serial.parseInt());
  // Serial.readString();

  // 没有延时问题
  // Serial.read();

  // while (Serial.available() > 0) {
  //   // 不延时
  //   Serial.println((char)Serial.read());
  // }
  // 按钮
  // int value = digitalRead(btnPin);
  // decrease(true, false, false, false, true, false);
  // increase(true, false, false, false, true, false);
  // decrease(false, true, false, false, false, true);
  // increase(false, true, false, false, false, true);
  // decrease(false, false, true, true, false, false);
  // increase(false, false, true, true, false, false);

  // 注意：取值需要保证波特率相同

  // 取值方式1：
  // if (Serial.available() > 0 ){                      // 当串口接收到信息后
  //   String serialData = Serial.readString();    // 将接收到的信息使用readString()存储于serialData变量
  //   // Serial.print("length: ");
  //   // Serial.println(serialData.length());
  //   Serial.println(serialData);                 // 以便查看serialData变量的信息
  // }

  // 取值方式2：
  // while (Serial.available()){           // 当串口接收到信息后
  //   byte serialData = Serial.read();    // 将接收到的信息使用read读取
  //   Serial.println(serialData);   // 然后通过串口监视器输出read函数读取的信息
  // }
  // 确定high和low，执行亮灯

  // 取值方式3：
  byte high;
  byte low;

  while (Serial.available() > 0) {
    char serialCmdChar = Serial.read();
    switch (serialCmdChar) {
      case 'H':
        high = Serial.parseInt();
        break;
      case 'L':
        low = Serial.parseInt();

        String value = "H" + String(high) + "L" + String(low);
        Serial.println("BLE received to \"" + value + "\"");
        break;
    }
  }

  if (high == 0x04) {
    // 红色
    light(rLedPin, low);
    light(gLedPin, 255);
    light(bLedPin, 255);


    // light(rLedPin2, low);
    // light(gLedPin2, 255);
    // light(bLedPin2, 255);
  } else if (high == 0x02) {
    // 绿色
    light(rLedPin, 255);
    light(gLedPin, low);
    light(bLedPin, 255);


    // light(rLedPin2, 255);
    // light(gLedPin2, low);
    // light(bLedPin2, 255);
  } else if (high == 0x01) {
    // 蓝色
    light(rLedPin, 255);
    light(gLedPin, 255);
    light(bLedPin, low);


    // light(rLedPin2, 255);
    // light(gLedPin2, 255);
    // light(bLedPin2, low);
  }
  // delay(1);
}

void light(int pin, int value) {
  analogWrite(pin, value);
}
