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

// Led 灯
#define LED_PIN_1       2
#define LED_PIN_2       3

// 两个马达
// 马达1转向针脚
#define MOTOR1_PIN_D    4
// 马达1速度针脚
#define MOTOR1_PIN_S    5
// 马达2转向针脚
#define MOTOR2_PIN_D    7
// 马达2速度针脚
#define MOTOR2_PIN_S    6


#define PIN_SERVO1    A0
#define PIN_SERVO2    A1
#define PIN_SERVO3    A2
#define PIN_SERVO4    A3
#define PIN_SERVO5    A4
#define PIN_SERVO6    A5

// 设置数据速率
#define NRF_DATA_RATE    RF24_2MBPS


// 摇杆大于这个值触发
#define JOY_GT   135
// 摇杆小于这个值触发
#define JOY_LT   119


unsigned int noRecvTime = 0;

// 定义马达速度
unsigned short int Motor1Speed = 200;
unsigned short int Motor2Speed = 200;

// 左边按钮 和右边 4个按钮作为微调使用
short int buttonWtLeft = 0;
short int buttonWtRight = 0;


// 定义马达最大速度
unsigned short int SpeedMax = 255;
unsigned short int SpeedMin = 80;


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

/**
 * 控制马达1
 * @param d [description]
 */
void Motor1(char d) {
    if (d == 'F') {
        setMotor(MOTOR1_PIN_S, MOTOR1_PIN_D, Motor1Speed);
    } else if (d == 'B') {
        setMotor(MOTOR1_PIN_S, MOTOR1_PIN_D, 0 - Motor1Speed);
    } else {
        setMotor(MOTOR1_PIN_S, MOTOR1_PIN_D, 0);
    }
}

/**
 * 控制马达2
 * @param d [description]
 */
void Motor2(char d) {
    if (d == 'F') {
        setMotor(MOTOR2_PIN_S, MOTOR2_PIN_D, Motor2Speed);
    } else if (d == 'B') {
        setMotor(MOTOR2_PIN_S, MOTOR2_PIN_D, 0 - Motor2Speed);
    } else {
        setMotor(MOTOR2_PIN_S, MOTOR2_PIN_D, 0);
    }
}

/**
 * 马达前进
 */
void MotorForward(int speed) {
    #ifdef DEBUG            
        Serial.print(" ->Forward speed:");
        Serial.println(speed);
    #endif
    SpeedSet(speed);
    Motor1('F');
    Motor2('F');
    // delay(d);
}

/**
 * 马达后退
 */
void MotorBack(int speed) {
    #ifdef DEBUG            
        Serial.print(" ->Back speed:");
        Serial.println(speed);
    #endif
    SpeedSet(speed);
    Motor1('B');
    Motor2('B');
    // delay(d);
}

void MotorLeft(int speed) {
    #ifdef DEBUG            
        Serial.print(" ->Left speed:");
        Serial.println(speed);
    #endif
    SpeedSet(speed);
    Motor1('B');
    Motor2('F');
    // delay(d);
}

void MotorRight(int speed) {
    #ifdef DEBUG            
        Serial.print(" ->Right speed:");
        Serial.println(speed);
    #endif
    SpeedSet(speed);
    Motor1('F');
    Motor2('B');
    // delay(d);
}

// 设置两个马达的速度
void SpeedSet(int speed){
    Motor1Speed = speed+buttonWtLeft;
    Motor2Speed = speed+buttonWtRight;

    if( Motor1Speed > SpeedMax ){
        Motor1Speed = SpeedMax;
    }else if( Motor1Speed < SpeedMin-10 ){
        Motor1Speed = SpeedMin;
    }

    if( Motor2Speed > SpeedMax ){
        Motor2Speed = SpeedMax;
    }else if( Motor2Speed < SpeedMin-10 ){
        Motor2Speed = SpeedMin;
    }
}


/**
 * 初始化马达
 */
void initMotor(){
    pinMode(MOTOR1_PIN_D, OUTPUT);
    pinMode(MOTOR1_PIN_S, OUTPUT);
    pinMode(MOTOR2_PIN_D, OUTPUT);
    pinMode(MOTOR2_PIN_S, OUTPUT);

    digitalWrite(MOTOR1_PIN_D, LOW);
    digitalWrite(MOTOR1_PIN_S, LOW);
    digitalWrite(MOTOR2_PIN_D, LOW);
    digitalWrite(MOTOR2_PIN_S, LOW);
}

/**
 * 设置马达
 * @param speedpin 速度引脚
 * @param dirpin   转弯引脚
 * @param speed    速度 -255 到 255,0以上是前进，以下是后台对
 */
void setMotor(int speedpin, int dirpin, int speed) {
    if (speed == 0) {
        digitalWrite(dirpin, LOW);
        analogWrite(speedpin, 0);
    } else if (speed > 0) {
        digitalWrite(dirpin, LOW);
        analogWrite(speedpin, speed);
    } else {
        digitalWrite(dirpin, HIGH);
        analogWrite(speedpin, 255 + speed);
    }
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
    radio.openReadingPipe(1, address);

    // 打开 NRF24L01 模块
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
        if (radio.getDataRate() != NRF_DATA_RATE) {
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
    // 左边开关开启是 全舵机控制，关闭是 马达加舵机控制
    if( data.tSwitch1 ){
        digitalWrite(LED_PIN_2,HIGH);
        ch1.write(map(data.lx, 0, 255, 0, 180));
        ch2.write(map(data.ly, 0, 255, 0, 180));
        ch3.write(map(data.rx, 0, 255, 0, 180));
        ch4.write(map(data.ry, 0, 255, 0, 180));
    }else{
        digitalWrite(LED_PIN_2,LOW);
        // 右边摇杆控制 两个舵机
        ch3.write(map(data.rx, 0, 255, 0, 180));
        ch4.write(map(data.ry, 0, 255, 0, 180));

        // 左面摇杆控制马达
        // 马达控制两种方式 1两个马达单独控制 2同时控制，左右前后
        if( data.tSwitch2 ){

            char left = 'S';
            if( data.lx > JOY_GT ){
                left = 'F';
                Motor1Speed = map(data.lx,JOY_GT,255,SpeedMin,SpeedMax);
            }else if(data.lx < JOY_LT ){
                left = 'B';
                Motor1Speed = map(data.lx,JOY_LT,0,SpeedMin,SpeedMax);
            }

            char right = 'S';
            if( data.ly > JOY_GT ){
                right = 'F';
                Motor2Speed = map(data.ly,JOY_GT,255,SpeedMin,SpeedMax);
            }else if(data.ly < JOY_LT ){
                right = 'B';
                Motor2Speed = map(data.ly,JOY_LT,0,SpeedMin,SpeedMax);
            }

            Motor1(left);
            Motor2(right);
        }else{
            // 同时控制，左右前后
            
            if( !data.button1 && buttonWtLeft < 30 ){
                buttonWtLeft++;
            }else if( !data.button2  && buttonWtLeft > -30 ){
                buttonWtLeft--;
            }

            if( !data.button3 && buttonWtRight < 30 ){
                buttonWtRight++;
            }else if( !data.button4 && buttonWtRight > -30){
                buttonWtRight--;
            }

            if( data.ly > JOY_GT ){
                int speed = map(data.ly,JOY_GT,255,SpeedMin,SpeedMax);
                MotorForward(speed);
            }else if(data.ly < JOY_LT ){
                int speed = map(data.ly,JOY_LT,0,SpeedMin,SpeedMax);
                MotorBack(speed);
            }


            if( data.lx > JOY_GT ){
                int speed = map(data.lx,JOY_GT,255,SpeedMin,SpeedMax);
                MotorLeft(speed);
            }else if(data.lx < JOY_LT ){
                int speed = map(data.lx,JOY_LT,0,SpeedMin,SpeedMax);
                MotorRight(speed);
            }

        }
    }

    ch5.writeMicroseconds(map(data.pot1, 0, 255, 1000, 2000));
    ch6.writeMicroseconds(map(data.pot2, 0, 255, 1000, 2000));

}

void setup() {
    Serial.begin(9600);
    initRadio();

    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    ch1.attach(PIN_SERVO1);
    ch2.attach(PIN_SERVO2);
    ch3.attach(PIN_SERVO3);
    ch4.attach(PIN_SERVO4);
    ch5.attach(PIN_SERVO5);
    ch6.attach(PIN_SERVO6);

    initMotor();
}

void loop() {
    checkRoido();
    if (radio.available()) {
        recvData();
        // unsigned long now = millis();
        // if ( now - lastRecvTime > 1000 ) {
        //     ResetData();
        // }
        // digitalWrite(LED_PIN_1,HIGH);

        run();

        noRecvTime = 0;
        
    }else{
        // digitalWrite(LED_PIN_1,LOW);
        delay(10);
    }
}                                                                                     
