/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define DEBUG

// 最大重试次数
#define MAX_RETRIES     3


// 定义手柄引脚
#define PIN_ANALOG_LX   A0
#define PIN_ANALOG_LY   A1
#define PIN_ANALOG_RX   A3
#define PIN_ANALOG_RY   A2

// 定义旋转定位器
#define PIN_POT1        A5
#define PIN_POT2        A4

// 手柄按钮
#define PIN_ANALOG_B1       0
#define PIN_ANALOG_B2       1

// 拨动开关
#define PIN_TSWITCH1        6
#define PIN_TSWITCH2        7

// 4个按钮
#define PIN_BUTTON1         3
#define PIN_BUTTON2         2
#define PIN_BUTTON3         5
#define PIN_BUTTON4         4

// Led
#define PIN_LED             8


// rf24接收器 其他针脚 11 12 13
#define PIN_RF24_CE     9
#define PIN_RF24_CSN    10

// 设置数据速率
#define NRF_DATA_RATE    RF24_2MBPS

RF24 radio(PIN_RF24_CE, PIN_RF24_CSN);  // CE, CSN

const byte address[6] = "00001";

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

int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
    val = constrain(val, lower, upper);
    if ( val < middle )
        val = map(val, lower, middle, 0, 128);
    else
        val = map(val, middle, upper, 128, 255);
    return ( reverse ? 255 - val : val );
}

// 初始化无线模块
void initRadio(){

    // 初始化 NRF24L01
    radio.begin();
    
    // 设置通道
    radio.setChannel(1);

    // 数据速率
    // RF24_250KBPS : 250 kbps 的数据速率。
    // RF24_1MBPS : 1 Mbps 的数据速率（默认值）。
    // RF24_2MBPS : 2 Mbps 的数据速率。
    radio.setDataRate(NRF_DATA_RATE);

    // 设置射频功率
    radio.setPALevel(RF24_PA_LOW);

    // 打开接收器
    radio.openWritingPipe(address);

    // 打开 NRF24L01 模块
    radio.stopListening();
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
        if (radio.getDataRate() != NRF_DATA_RATE) {
            radio.failureDetected = true;
            Serial.println("发现无线电模块故障2！");
        }
    }
}

void setup() {
    Serial.begin(9600);
    initRadio();

    pinMode(PIN_ANALOG_LX, INPUT_PULLUP);
    pinMode(PIN_ANALOG_LY, INPUT_PULLUP);
    pinMode(PIN_ANALOG_RX, INPUT_PULLUP);
    pinMode(PIN_ANALOG_RY, INPUT_PULLUP);

    pinMode(PIN_POT1, INPUT_PULLUP);
    pinMode(PIN_POT2, INPUT_PULLUP);

    pinMode(PIN_ANALOG_B1, INPUT_PULLUP);
    pinMode(PIN_ANALOG_B2, INPUT_PULLUP);


    pinMode(PIN_TSWITCH1, INPUT_PULLUP);
    pinMode(PIN_TSWITCH2, INPUT_PULLUP);
    pinMode(PIN_BUTTON1, INPUT_PULLUP);
    pinMode(PIN_BUTTON2, INPUT_PULLUP);
    pinMode(PIN_BUTTON3, INPUT_PULLUP);
    pinMode(PIN_BUTTON4, INPUT_PULLUP);

    pinMode(PIN_LED, OUTPUT);
}

void loop() {
    checkRoido();

    short lx = analogRead(PIN_ANALOG_LX);
    short ly = analogRead(PIN_ANALOG_LY);
    short rx = analogRead(PIN_ANALOG_RX);
    short ry = analogRead(PIN_ANALOG_RY);
    short pot1 = analogRead(PIN_POT1);
    short pot2 = analogRead(PIN_POT2);

    data.lx = mapJoystickValues(lx,15,546,1022,true);
    data.ly = mapJoystickValues(ly,15,572,1022,true);
    data.rx = mapJoystickValues(rx,14,547,1022,true);
    data.ry = mapJoystickValues(ry,14,552,1022,true);

    // data.lx = map(lx, 15, 1023, 0, 255);;
    // data.ly = map(ly, 15, 1023, 0, 255);
    // data.rx = map(rx, 14, 1023, 0, 255);
    // data.ry = map(ry, 14, 1023, 0, 255);

    data.pot1 = map(pot1, 13, 1023, 0, 255);
    data.pot2 = map(pot2, 13, 1023, 0, 255);

    data.lb = digitalRead(PIN_ANALOG_B1);
    data.rb = digitalRead(PIN_ANALOG_B2);

    data.tSwitch1 = digitalRead(PIN_TSWITCH1);
    data.tSwitch2 = digitalRead(PIN_TSWITCH2);
    data.button1 = digitalRead(PIN_BUTTON1);
    data.button2 = digitalRead(PIN_BUTTON2);
    data.button3 = digitalRead(PIN_BUTTON3);
    data.button4 = digitalRead(PIN_BUTTON4);

#if defined(DEBUG)
    Serial.print("lx:");
    Serial.print(lx);
    Serial.print(" ly:");
    Serial.print(ly);
    Serial.print(" rx:");
    Serial.print(rx);
    Serial.print(" ry:");
    Serial.print(ry);
    Serial.print(" pot1:");
    Serial.print(pot1);
    Serial.print(" pot2:");
    Serial.print(pot2);

    Serial.print(" | dlx:");
    Serial.print(data.lx);
    Serial.print(" dly:");
    Serial.print(data.ly);
    Serial.print(" drx:");
    Serial.print(data.rx);
    Serial.print(" dry:");
    Serial.print(data.ry);
    Serial.print(" dlb:");
    Serial.print(data.lb);
    Serial.print(" drb:");
    Serial.print(data.rb);

    Serial.print(" dpot1:");
    Serial.print(data.pot1);
    Serial.print(" dpot2:");
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

    bool success = false;
    for (int i = 0; i < MAX_RETRIES; i++) {
        if (radio.write(&data, sizeof(DataPackage))) {
            // 发送成功
            success = true;
            break;
        } else {
            // 发送失败，延迟一段时间后重试
            delay(1);
        }
    }

    if (success) {
        // 发送成功
        digitalWrite(PIN_LED,HIGH);
#if defined(DEBUG)
        Serial.println("数据发送成功");
#endif

    } else {
        // 发送失败
        digitalWrite(PIN_LED,LOW);
#if defined(DEBUG)
        Serial.println("数据发送失败");
#endif

    }
    // delay(5);
}
