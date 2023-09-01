/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>


#define DEBUG


// rf24接收器 其他针脚 11 12 13
#define PIN_RF24_CE     9
#define PIN_RF24_CSN    10

#define LED_PIN_1       2
#define LED_PIN_2       3

#define PIN_SERVO1    A0
#define PIN_SERVO2    A1
#define PIN_SERVO3    A2
#define PIN_SERVO4    A3
#define PIN_SERVO5    A4
#define PIN_SERVO6    A5

unsigned int noRecvTime = 0;


RF24 radio(PIN_RF24_CE, PIN_RF24_CSN);  // CE, CSN

const byte address[6] = "00001";

Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;
Servo ch6;

struct DataPackage {
    byte lx;        //左边手柄x轴
    byte ly;        //左边手柄y轴
    byte lb;        //左边手柄按钮
    byte rx;        //右边手柄x轴
    byte ry;        //右边手柄y轴
    byte rb;        //右边手柄按钮
    byte pot1;      //电位器1
    byte pot2;      //电位器2
    byte tSwitch1;  //拨动开关1
    byte tSwitch2;  //拨动开关2
    byte button1;   //按钮1
    byte button2;   //按钮2
    byte button3;   //按钮3
    byte button4;   //按钮4
};

DataPackage data;

void ResetData() 
{
    data.lx = 127;
    data.ly = 127;
    data.lb = 1;
    data.rx = 127;
    data.ry = 127;
    data.rb = 1;
    data.pot1 = 127;
    data.pot2 = 127;
    data.tSwitch1 = 0;
    data.tSwitch2 = 0;
    data.button1 = 1;
    data.button2 = 1;
    data.button3 = 1;
    data.button4 = 1;
}


// 初始化无线模块
void initRadio(){
    radio.begin();
    radio.openReadingPipe(0, address);
    // radio.setPALevel(RF24_PA_MAX);
    radio.startListening();
    ResetData();
}

uint32_t configTimer = millis();
// 检测无线模块，如果故障了重新初始化
void checkRoido(){
    if (radio.failureDetected) {
        radio.failureDetected = false;
        delay(250);
        Serial.println("发现无线电模块故障！");
        initRadio();
    }
    // 每隔5秒验证一次无线电配置。这可以使用与无线电默认值不同的任何设置来完成。
    if (millis() - configTimer > 5000) {
        configTimer = millis();
        if (radio.getDataRate() != RF24_1MBPS) {
            radio.failureDetected = true;
            Serial.println("发现无线电模块故障2！");
        }
    }
}

unsigned long lastRecvTime = 0;
void recvData()
{
    while ( radio.available() ) {
        radio.read(&data, sizeof(DataPackage));

#if defined(DEBUG)
        Serial.print("lx:");
        Serial.print(data.lx);
        Serial.print(" ly:");
        Serial.print(data.ly);
        Serial.print(" rx:");
        Serial.print(data.rx);
        Serial.print(" ry:");
        Serial.print(data.ry);
        Serial.print(" lb:");
        Serial.print(data.lb);
        Serial.print(" rb:");
        Serial.print(data.rb);

        Serial.print(" pot1:");
        Serial.print(data.pot1);
        Serial.print(" pot2:");
        Serial.print(data.pot2);
        Serial.print(" tSwitch1:");
        Serial.print(data.tSwitch1);
        Serial.print(" tSwitch2:");
        Serial.print(data.tSwitch2);
        Serial.print(" button1:");
        Serial.print(data.button1);
        Serial.print(" button2:");
        Serial.print(data.button2);
        Serial.print(" button3:");
        Serial.print(data.button3);
        Serial.print(" button4:");
        Serial.println(data.button4);
#endif
        lastRecvTime = millis();   // receive the data | data alınıyor
    }
}

void run(){
    if( data.tSwitch1 ){
        digitalWrite(LED_PIN_2,HIGH);
    }else{
        digitalWrite(LED_PIN_2,LOW);
    }
    ch1.write(map(data.lx, 0, 255, 0, 180));
    ch2.write(map(data.ly, 0, 255, 0, 180));
    ch3.write(map(data.rx, 0, 255, 0, 180));
    ch4.write(map(data.ry, 0, 255, 0, 180));

    ch5.writeMicroseconds(map(data.pot1, 0, 255, 1000, 2000));
    ch6.writeMicroseconds(map(data.pot2, 0, 255, 1000, 2000));
}

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    ch1.attach(PIN_SERVO1);
    ch2.attach(PIN_SERVO2);
    ch3.attach(PIN_SERVO3);
    ch4.attach(PIN_SERVO4);
    ch5.attach(PIN_SERVO5);
    ch6.attach(PIN_SERVO6);

    initRadio();
}

void loop() {
    checkRoido();
    if (radio.available()) {
        recvData();
        unsigned long now = millis();
        if ( now - lastRecvTime > 1000 ) {
            ResetData();
        }
        digitalWrite(LED_PIN_1,HIGH);

        run();

        // ch1.writeMicroseconds(map(data.lx, 0, 255, 1000, 2000));
        // ch2.writeMicroseconds(map(data.ly, 0, 255, 1000, 2000));
        // ch3.writeMicroseconds(map(data.rx, 0, 255, 1000, 2000));
        // ch4.writeMicroseconds(map(data.ry, 0, 255, 1000, 2000));
        noRecvTime = 0;
        
    }else{
        digitalWrite(LED_PIN_1,LOW);
        Serial.print("没有接收到数据");
        noRecvTime++;
        Serial.println(noRecvTime);
        if( noRecvTime > 500 ){
            initRadio();
            Serial.println("重启无线模块");
            noRecvTime = 0;
            run();
        }
    }
    delay(1);
}                                                                                     
