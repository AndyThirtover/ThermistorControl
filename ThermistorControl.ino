// Thermistor Control
// Uses 7789 Display

/*
 ST7789 240x240 IPS (without CS pin) connections (only 6 wires required):

 #01 GND -> GND
 #02 VCC -> VCC (3.3V only!)
 #03 SCL -> D13/SCK
 #04 SDA -> D11/MOSI
 #05 RES -> D8 or any digital
 #06 DC  -> D7 or any digital
 #07 BLK -> NC
*/

#define TARGET_TEMP -30
#define RUN_MIN 300
#define COMPRESSOR_PIN 3

#define TFT_DC    7
#define TFT_RST   8
#define SCR_WD   240
#define SCR_HT   240

#define DGREEN 0x3E0
#define DGREY 0x2044
#define ORANGE 0xF380


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ST7789_Fast.h>
#include <Fonts/FreeSansBold9pt7b.h>
//#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Filter.h>
#include "thermistor.h"

/*
 * Thermistor stuff
 */
THERMISTOR thermistor(A7,              // Analog pin
                      4700,            // Nominal resistance at 25 ºC
                      3724,            // thermistor's beta coefficient
                      100000);         // Value of the series resistor


int current;

int previous = -999;  //a value that forces the first display
long run_time = 0;     // compressor run time

ExponentialFilter<long> ADCFilter(15,0);
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST);


unsigned long res[16];
void setup(void) 
{
  Serial.begin(9600);
  Serial.println(F("Temperature Contoller"));
  //tft.reset();
  tft.init(SCR_WD, SCR_HT);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);  
  tft.setTextSize(2);
  tft.println("Thermistor Control");
  tft.println("Start-up ...");
  tft.fillScreen(DGREY);
  tft.fillRect(174,4,52,232,DGREEN);
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextSize(1);
  tft.setCursor(2,20);
  tft.println("Temperature");
  //tft.setCursor(2,45);
  tft.println("Control");
  tft.setTextSize(2);
  tft.setCursor(2,90);
  tft.setTextColor(BLUE);  
  tft.print("Current");
  tft.setCursor(2,180);
  tft.setTextColor(GREEN);  
  tft.print("Run Time");
  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextSize(1);

  pinMode(2,OUTPUT);
  pinMode(COMPRESSOR_PIN,OUTPUT);  // To Drive Compessor
  digitalWrite(2,HIGH); //3.3V for thermistor, can be switched for self heating issues

}


void loop(void) 
{
  ADCFilter.Filter(thermistor.read());
  current = ADCFilter.Current();
  if (current != previous) {
    tft.fillRect(10,100,162,45,DGREY);
    tft.setCursor(20,140);
    
    if (current > TARGET_TEMP) {
      tft.setTextColor(RED);  
    }
    else {
      tft.setTextColor(WHITE);  
    }
    
    tft.print(current);
    tft.print("C");
    draw_bar(current);
    previous = current;
  }
  if(millis() > 5000) {
    check_compressor();  // Wait for temperatures to settle before running compressor
  }
 delay(300);
}

void draw_bar(int val) {
  int bar_size ;
  // val can be from -40 to 30
  if (val > 30) {
    val = 30;
  }
  if (val < -40){
    val = -40;
  }
  bar_size = (val+40) * 3;
  tft.fillRect(180,10,40,220-bar_size,BLACK);
    if (val > -15) {
      tft.fillRect(180,230-bar_size,40,bar_size,RED);
  }
  else {
      tft.fillRect(180,230-bar_size,40,bar_size,BLUE);
  }
 
}

void check_compressor() {
  if (current > TARGET_TEMP) {
    run_time = RUN_MIN;
    digitalWrite(COMPRESSOR_PIN,HIGH);
  }
  else {
    run_time = run_time - 1 ;
    if (run_time < 1) {
      digitalWrite(COMPRESSOR_PIN,LOW);
      run_time = 0;
    }
  }
  
  tft.fillRect(10,190,162,45,DGREY);
  if (run_time > 0 ) {
    tft.setCursor(20,225);
    tft.setTextColor(ORANGE);  
    tft.println(run_time);
  }
}
