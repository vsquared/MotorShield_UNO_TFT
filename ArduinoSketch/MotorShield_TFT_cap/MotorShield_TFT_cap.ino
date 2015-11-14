
/*
THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * For use with the Adafruit Motor Shield v2 
 * And Adafruit 2.8" TFT LCD - capacitive
 * Any of the four screen rotations may be use.
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

// Default values for Adafruit shield v2.
#define TFT_DC 9
#define TFT_CS 10

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

 typedef struct{
  int Min;
  int Max;
  int Init;
  int Step;
 }arrowRec;
 
 // Rotations 0,2 = portrait  : 0->USB=right,upper : 2->USB=left,lower
 // Rotations 1,3 = landscape : 1->USB=left,upper  : 3->USB=right,lower
 const int rotation = 2; //(0->3)
 char arrwValueStr[5];

// { Min, Max, Init, Step }
 arrowRec arrw = {0,255,50,1};
 int arrwValue = arrw.Init;
 
 // Coordinates of core square for arrows
 const int left = 120;
 const int top = 80;
 const int w = 50;
 const int h = 50;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Select which 'port' M1, M2, M3 or M4. In this case, M4
Adafruit_DCMotor *myMotor = AFMS.getMotor(4);
// You can also drive another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);

void drawArrows()
 {
  // up arrow rect
  tft.fillTriangle(left + w/2, top - h, left, top, left + w, top, WHITE);
  // down arrow rect
  tft.fillTriangle(left + w/2, top + 2*h, left, top + h, left + w, top + h, WHITE);
  // right arrow rect
  tft.fillTriangle(left + 2*w, top + h/2, left + w, top, left + w, top + h, WHITE);
  // left arrow rect
  tft.fillTriangle(left - w, top + h/2, left, top, left, top + h, WHITE);
  tft.setCursor(left+w/2-6,top + h/2-10 );
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.print("S");
 }

 void showArrwValue(int arrowValue)
 {
  sprintf(arrwValueStr,"%03d",arrowValue);
  tft.setCursor(14,20);
  tft.setTextColor(WHITE, BLUE);
  tft.setTextSize(3);
  tft.print(arrwValueStr);
 }

void barGraphInit()
 { 
    for(int j = 0; j < 51; j++){
     tft.drawLine(20 , 310 - j, 60, 310 - j, GREEN);     
    }
    tft.drawRect( 19, 320-255-12, 43, 255+4, WHITE);
 }
 
 void leftArrowProc()
 {
  tft.setCursor(90,210);
  tft.print("Reverse");
  myMotor->run(BACKWARD);
 }
 
 void rightArrowProc()
 {
  tft.setCursor(90,210);
  tft.print("Forward");
  myMotor->run(FORWARD);
 }
 
 void upArrowProc()
 {
  if(arrwValue < arrw.Max)arrwValue = arrwValue + arrw.Step;
  if(arrwValue >= arrw.Max) arrwValue = arrw.Max;
  showArrwValue(arrwValue);
  tft.drawLine(20 ,310 - arrwValue, 60, 310 - arrwValue, GREEN);
  myMotor->setSpeed(arrwValue);
 }
 
 void downArrowProc()
 {
  tft.drawLine(20 ,310 - arrwValue, 60, 310 - arrwValue, BLUE);
  arrwValue = arrwValue - arrw.Step;
  if(arrwValue < arrw.Min) arrwValue = arrw.Min;
  showArrwValue(arrwValue);
  myMotor->setSpeed(arrwValue);
 }

  void stopProc()
 {
  tft.setCursor(90,210);
  tft.print("Stop");
  myMotor->run(RELEASE);
 }
 
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps

   tft.begin();
  if (! ctp.begin()) {// may pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }
  tft.setRotation(rotation);
  tft.setRotation(rotation);
  tft.fillScreen(BLUE);
  drawArrows();
  barGraphInit();
  showArrwValue(arrwValue);
  
  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  // Set the speed to start, from 0 (off) to 255 (max speed)
  myMotor->setSpeed(arrw.Init);
}

void loop() {
  uint16_t x, y; 
  TS_Point p;

  if (! ctp.touched()) {  
    return;
  }
  
  p = ctp.getPoint();
 
 if (rotation == 0){
  x = map(p.x, 240, 0, 0, 240);
  y = map(p.y, 320, 0, 0, 320);
 }
 
 if (rotation == 1){
 // p.x, p.y reversed //  
  x = map(p.y, 320, 0, 0, 320);
  y = map(p.x, 0, 240, 0, 240);
 }
 
 if (rotation == 2){
  x = map(p.x, 0, 240, 0, 240);
  y = map(p.y, 0, 320, 0, 320);  
 }
 
  if (rotation == 3){
 //  p.x, p.y reversed //
  x = map(p.y, 0, 320, 0, 320);
  y = map(p.x, 240, 0, 0, 240);
 }
 
  // Erase old data 
  tft.fillRect( 90, 210, 135, 30, BLUE );
  
  while (ctp.touched()) { 
  // up arrow rect
    if(x > left && x < left + w){
      if(y > top - h && y < top){
       upArrowProc();
      }
    }
 // down arrow rect
    if(x > left && x < left + w){
      if(y > top + h && y < top + 2*h){
       downArrowProc();
      }
    }
 // right arrow rect  
   if(x > left + w && x < left + 2*w){
     if(y > top && y < top + h){
       rightArrowProc();
     }
   }   
 // left arrow rect   
   if(x > left - w && x < left){
     if(y > top && y < top + h){
       leftArrowProc();
     }
   }
 // center rect = stop
    if(x > left && x < left + w){
     if(y > top && y < top + h){
       stopProc();
     }
   }     
  }
}


