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

#define PIN_ANALOG_X A0
#define PIN_ANALOG_Y A1

#define BUTTON_UP 2
#define BUTTON_RIGHT 3
#define BUTTON_DOWN 5
#define BUTTON_LEFT 4
#define BUTTON_E 6
#define BUTTON_F 7

RF24 radio(9, 10);  // CE, CSN

const byte address[6] = "00002";
const uint64_t pipeOut = 0xE9E8F0F0E1LL;

struct Signal {
    byte lx;
    byte ly;
    byte up; //BUTTON_UP
    byte down; //BUTTON_DOWN
    byte right; //BUTTON_RIGHT
    byte left; //BUTTON_LEFT
    byte be; //BUTTON_E
    byte bf; //BUTTON_F
};

Signal data;

void ResetData() 
{
    data.lx = 127; // Motor Stop (254/2=127)| Motor Kapalı (Signal lost position | sinyal kesildiğindeki pozisyon)
    data.ly = 127; // Center | Merkez (Signal lost position | sinyal kesildiğindeki pozisyon)
    data.ly = 127;
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

void setup() {
  Serial.begin(9600);
  pinMode(PIN_ANALOG_X, INPUT_PULLUP);
  pinMode(PIN_ANALOG_Y, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_E, INPUT_PULLUP);
  pinMode(BUTTON_F, INPUT_PULLUP);


  radio.begin();
  radio.openWritingPipe(address);
  // radio.openWritingPipe(pipeOut);
  // radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  ResetData();
}

void loop() {
  // const char text[] = "Hello World123231";
  // radio.write(&text, sizeof(text));
  // Serial.println(text);
  
  // char json_response[64];
  // char* p = json_response;
  // *p++ = '{';
  // p += sprintf(p, "\"x\":%d,", analogRead(A0));
  // p += sprintf(p, "\"y\":%d,", analogRead(A1));
  // p += sprintf(p, "\"rx\":%d,", analogRead(A2));
  // p += sprintf(p, "\"ry\":%d", analogRead(A3));
  // *p++ = '}';
  // *p++ = 0;
  // Serial.println(json_response);
  // radio.write(json_response, sizeof(json_response));
  // delay(1000);


    data.lx = mapJoystickValues( analogRead(PIN_ANALOG_X), 12, 512, 1022, true );
    data.ly = mapJoystickValues( analogRead(PIN_ANALOG_Y), 12, 512, 1022, true );
    data.rx = mapJoystickValues( analogRead(A2), 12, 506, 1022, true );
    data.ry = mapJoystickValues( analogRead(A3), 12, 506, 1022, true );


    // if( data.lx != 127 ){
        Serial.print("lx:");
        Serial.print(data.lx);
    // }
    // if( data.ly != 127 ){
        Serial.print(" ly:");
        Serial.print(data.ly);

    // }
    // if( data.rx != 127 ){
        Serial.print(" rx:");
        Serial.print(data.rx);
    // }
    // if( data.ry != 127 ){
        Serial.print(" ry:");
        Serial.println(data.ry);
    // }

    radio.write(&data, sizeof(Signal));
    delay(5);
}