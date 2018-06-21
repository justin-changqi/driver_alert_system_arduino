// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Timer.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define P0            6
#define NUMPIXELS0    16

#define BREATH_RATE 0.5

#define FLASH_COUNT 10  // 10*100 ms

const int mask_index_[][3] = {{1, 2, 3}, {3, 4, 5}, {5, 6, 7}, {15, 14, 13}, {13, 12, 11}, {11, 10, 9}};
//      0 ---- 3
//       |    |
//      1|    |4
//       |    |
//      2 ---- 5
bool notice_flag_[] = {0, 0, 0, 0, 0, 0};
int flash_counter_ = 1000;

Timer t_led, t_flash;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS0, P0, NEO_GRB + NEO_KHZ800);

void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  strip.begin(); // This initializes the NeoPixel library.
  t_led.every(10, update_light);
  t_flash.every(100, flashCount);
  setColor(100, 100, 100);
  strip.show(); 
}

void loop() {
  t_led.update();
  t_flash.update();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  byte x = Wire.read();    // receive byte as an integer
  Serial.println(x, HEX);         // print the integer
  setNoticeFlag(x, notice_flag_);
}

void update_light() {
  breathAll();
  setAlertMask(notice_flag_);
  strip.show(); 
}

void breathAll() { 
  static int i = 0;
  static bool flag=true;
  static double r_ratio, g_ratio, b_ratio;
  r_ratio = BREATH_RATE;
  g_ratio = BREATH_RATE;
  b_ratio = BREATH_RATE;
  if (flag){
    i += 2;
    if(i > 200) {
      flag = false;
    }else{
      for(int j=0; j<strip.numPixels(); j++) {  
        strip.setPixelColor(j, strip.Color(i*r_ratio,i*g_ratio,i*b_ratio));  
      } 
    }
  }else{
    i-=2;
    if(i < 1) {
      flag = true;
    }else{  
      for(int j=0; j<strip.numPixels(); j++) {  
        strip.setPixelColor(j, strip.Color(i*r_ratio,i*g_ratio,i*b_ratio));   
      } 
    }
  }
} 


void setColor(int r, int g, int b) {
  for(int j=0; j<strip.numPixels(); j++) {  
          strip.setPixelColor(j, strip.Color(r,g,b));  
  } 
}

void setNoticeFlag(byte data, bool *flags) {
  flash_counter_ = FLASH_COUNT;
  Serial.println("data: ");
  Serial.println(data, BIN);
  Serial.println("flags: ");
  for(int i=0; i<6; i++) {
    byte flag = data >> i;
    flag &= 0x01;
    Serial.print(flag, BIN);
    Serial.print(" ");
    flags[i] = flag;
  }
  Serial.println("");
}

void setAlertMask(bool *flags) {
  static bool light_on_flag = true;
  for(int i=0; i<6; i++) {
    if(flags[i] == true) {
      if(light_on_flag == true) {
        if (flash_counter_ > 0) {
          if (flash_counter_ % 2 == 0) {
            for (int j=0; j<3; j++) {
              strip.setPixelColor(mask_index_[i][j], strip.Color(100,0,0));  
            }
          } else {
            for (int j=0; j<3; j++) {
              strip.setPixelColor(mask_index_[i][j], strip.Color(0,0,0));  
            }
          }
        }
      }
    }
  }
}

void flashCount() {
  if (flash_counter_ > 0) {
    flash_counter_--;
  }
}

