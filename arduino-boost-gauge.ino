//Arduino pro micro, .93" I2C OLED use pin 2 for SDA and 3 for SCL ***Look up i2c pins for your controller

#include <SevSeg.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin to connect pressure sensor output to
#define SENSOR_PIN A2

// Number of data values to average for smoothing
#define WINDOW_SIZE 6

// Setup OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

int OLED_RESET = -1;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Variables for bar graph
float rawval = 0;   // Setup raw sensor value
float barboost = 0; // Setup value for boost bar

// Variables for peak boost
int boostPeakReset = 4000; // time in milis to reset peak value
float boostPeak = 0.00;
float boostMax = 0.00;
float boostDisp = 0.00;
unsigned long boostPeakTimer = 0;

int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;

// log

byte count;
byte sensorArray[128];
byte drawHeight;
boolean filled = 0; //decide either filled, or dot-display. 0==dot display.

// Startup graphic, abg
const unsigned char abg [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe9, 0xff, 0xff, 0x1f, 0xe0, 0x0f, 0xfc, 0x1f, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xfe, 0x0f, 0xe0, 0x07, 0xe0, 0x07, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xfe, 0x0f, 0xe0, 0x03, 0xc0, 0x01, 0xff, 
  0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x0f, 0xe3, 0xc3, 0x83, 0xe1, 0xff, 
  0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x07, 0xe3, 0xe3, 0x87, 0xf0, 0xff, 
  0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x47, 0xe3, 0xe3, 0x0f, 0xff, 0xff, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf8, 0xc3, 0xe0, 0x07, 0x0f, 0xff, 0xff, 
  0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf8, 0xe3, 0xe0, 0x07, 0x1f, 0x00, 0xff, 
  0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf8, 0xe3, 0xe0, 0x03, 0x1f, 0x00, 0xff, 
  0xff, 0xf0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xf1, 0xf1, 0xe3, 0xc3, 0x0f, 0x00, 0xff, 
  0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xf0, 0x01, 0xe3, 0xe1, 0x0f, 0xf8, 0xff, 
  0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xe0, 0x00, 0xe3, 0xe1, 0x87, 0xf1, 0xff, 
  0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xe0, 0x00, 0xe3, 0xc3, 0x83, 0xc1, 0xff, 
  0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xe3, 0xf8, 0xe0, 0x03, 0xc0, 0x03, 0xff, 
  0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xc3, 0xf8, 0x60, 0x07, 0xe0, 0x07, 0xff, 
  0xfe, 0x03, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc7, 0xfc, 0x60, 0x0f, 0xfc, 0x1f, 0xff, 
  0xfe, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xfe, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xfc, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x1f, 0xe0, 0x0f, 0xfc, 0x1f, 0xff, 
  0xfc, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x0f, 0xe0, 0x07, 0xe0, 0x07, 0xff, 
  0xf8, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfe, 0x0f, 0xe0, 0x03, 0xc0, 0x01, 0xff, 
  0xf8, 0x1f, 0xc0, 0x7e, 0x03, 0xf8, 0x1f, 0xc1, 0xff, 0xfc, 0x0f, 0xe3, 0xc3, 0x83, 0xe1, 0xff, 
  0xf8, 0x1f, 0x80, 0x7e, 0x01, 0xf8, 0x1f, 0xc1, 0xff, 0xfc, 0x07, 0xe3, 0xe3, 0x87, 0xf0, 0xff, 
  0xf8, 0x3f, 0x80, 0x3c, 0x01, 0xfc, 0x1f, 0xc1, 0xff, 0xfc, 0x47, 0xe3, 0xe3, 0x0f, 0xff, 0xff, 
  0xf8, 0x3f, 0x81, 0x18, 0xc0, 0xfc, 0x1f, 0xff, 0xff, 0xf8, 0xc3, 0xe0, 0x07, 0x0f, 0xff, 0xff, 
  0xf8, 0x3f, 0x03, 0x81, 0xc0, 0xfc, 0x1f, 0xff, 0xff, 0xf8, 0xe3, 0xe0, 0x07, 0x1f, 0x00, 0xff, 
  0xf8, 0x3f, 0x03, 0xc3, 0xc0, 0xfc, 0x0f, 0xff, 0xff, 0xf8, 0xe3, 0xe0, 0x03, 0x1f, 0x00, 0xff, 
  0xf8, 0x3f, 0x03, 0xc1, 0xc0, 0xfc, 0x0f, 0xff, 0xff, 0xf1, 0xf1, 0xe3, 0xc3, 0x0f, 0x00, 0xff, 
  0xf8, 0x3f, 0x03, 0x81, 0xc0, 0xfc, 0x1f, 0xff, 0xff, 0xf0, 0x01, 0xe3, 0xe1, 0x0f, 0xf8, 0xff, 
  0xf8, 0x3f, 0x81, 0x18, 0xc0, 0xfc, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0xe3, 0xe1, 0x87, 0xf1, 0xff, 
  0xf8, 0x3f, 0x80, 0x3c, 0x01, 0xfc, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0xe3, 0xc3, 0x83, 0xc1, 0xff, 
  0xf8, 0x1f, 0x80, 0x7e, 0x01, 0xf8, 0x1f, 0xff, 0xff, 0xe3, 0xf8, 0xe0, 0x03, 0xc0, 0x03, 0xff, 
  0xf8, 0x1f, 0xc0, 0x3e, 0x03, 0xf8, 0x1f, 0xff, 0xff, 0xc3, 0xf8, 0x60, 0x07, 0xe0, 0x07, 0xff, 
  0xf8, 0x1f, 0xc0, 0x00, 0x03, 0xf8, 0x1f, 0xff, 0xff, 0xc7, 0xfc, 0x60, 0x0f, 0xfc, 0x1f, 0xff, 
  0xfc, 0x0f, 0xe0, 0x00, 0x07, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xfc, 0x0f, 0xf0, 0x00, 0x0f, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0x1f, 0xe0, 0x0f, 0xfc, 0x1f, 0xff, 
  0xfe, 0x07, 0xf8, 0x00, 0x1f, 0xe0, 0x3f, 0xff, 0xff, 0xfe, 0x0f, 0xe0, 0x07, 0xe0, 0x07, 0xff, 
  0xfe, 0x03, 0xfe, 0x00, 0x7f, 0xc0, 0x7f, 0xff, 0xff, 0xfe, 0x0f, 0xe0, 0x03, 0xc0, 0x01, 0xff, 
  0xff, 0x03, 0xff, 0xc3, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xfc, 0x0f, 0xe3, 0xc3, 0x83, 0xe1, 0xff, 
  0xff, 0x01, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xfc, 0x07, 0xe3, 0xe3, 0x87, 0xf0, 0xff, 
  0xff, 0x80, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0xff, 0xfc, 0x47, 0xe3, 0xe3, 0x0f, 0xff, 0xff, 
  0xff, 0xc0, 0x3f, 0xff, 0xfc, 0x01, 0xff, 0xff, 0xff, 0xf8, 0xc3, 0xe0, 0x07, 0x0f, 0xff, 0xff, 
  0xff, 0xc0, 0x1f, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xf8, 0xe3, 0xe0, 0x07, 0x1f, 0x00, 0xff, 
  0xff, 0xf0, 0x07, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xf8, 0xe3, 0xe0, 0x03, 0x1f, 0x00, 0xff, 
  0xff, 0xf0, 0x00, 0xff, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xf1, 0xf1, 0xe3, 0xc3, 0x0f, 0x00, 0xff, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x01, 0xe3, 0xe1, 0x0f, 0xf8, 0xff, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0xe3, 0xe1, 0x87, 0xf1, 0xff, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0xe3, 0xc3, 0x83, 0xc1, 0xff, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe3, 0xf8, 0xe0, 0x03, 0xc0, 0x03, 0xff, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xc3, 0xf8, 0x60, 0x07, 0xe0, 0x07, 0xff, 
  0xff, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xc7, 0xfc, 0x60, 0x0f, 0xfc, 0x1f, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


void setup()
{
  Serial.begin(9600);                  // start monitoring raw voltage for calibration
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // 3.3V power supply
  display.clearDisplay();              // Clear the display and ram

  // Display the splash screen
  display.drawBitmap(0, 0, abg, 128, 64, WHITE);
  display.display();

  delay(2000);

  for (count = 0; count <= 128; count++) //zero all elements
  {
    sensorArray[count] = 0;
  }
}

void loop() // Start loop
{
  int boostmbar = map(analogRead(SENSOR_PIN), 21, 961, 100, 2600);
  rawval = analogRead(SENSOR_PIN); // Read MAP sensor raw value on analog port 0

//  SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
//  VALUE = boostmbar;                 // Read the next sensor value
//  READINGS[INDEX] = VALUE;           // Add the newest reading to the window
//  SUM = SUM + VALUE;                 // Add the newest reading to the sum
//  INDEX = (INDEX + 1) % WINDOW_SIZE; // Increment the index, and wrap to 0 if it exceeds the window size
//
//  AVERAGED = SUM / WINDOW_SIZE; // Divide the sum of the window by the window size for the result

  barboost = ((rawval * 0.14)); // Calculate boost value for the graph

  if (boostPeak < boostmbar && boostmbar > 0.50)
  {
    boostPeak = boostmbar;
    boostPeakTimer = millis();
    if (boostMax < boostPeak)
    {
      boostMax = boostPeak;
    }
  }
  else if (boostPeak > boostmbar && (millis() - boostPeakTimer) > boostPeakReset)
  {
    boostPeak = 0.00;
  }
  

  // log

  drawHeight = map(analogRead(SENSOR_PIN), 0, 1023, 0, 35);

  sensorArray[128] = drawHeight;

  for (count = 0; count <= 128; count++)
  {

  if (sensorArray[count] < 8) {
    sensorArray[count] = 8;
  }
    
    if (filled == false)
    {
      display.drawPixel(count, 71 - sensorArray[count], WHITE);
    }
    else
      display.drawLine(count, 1, count, 71 - sensorArray[count], WHITE);
  }

  for (count = 1; count <= 128; count++) // count down from 160 to 2
  {
    sensorArray[count - 1] = sensorArray[count];
  }

  display.fillRect(0, 0, barboost, 4, WHITE); // Draws the bar depending on the sensor value

  display.setTextSize(1); //Display peak boost
  display.setCursor(97, 10);

  boostDisp = ((boostPeak * 0.001) - 0.865)*14;

  if (boostDisp < 0) {
    boostDisp = 0.00;
  }
  
  display.println(boostDisp); // 0.97 = 970mbar atmospheric pressure correction


  if ((((boostmbar  * 0.001) - 0.865)*14) < 0) {
    display.setTextSize(1);
    display.setCursor(97, 20);
    display.println("INHG");
    display.setTextColor(WHITE);
    display.setTextSize(3);
    display.setCursor(0, 10);
    display.println(((boostmbar * 0.001) - 0.865)*63.2, 1);
  }
  else if ((((boostmbar * 0.001) - 0.865)*14) > 0) {
    display.setTextSize(1);
    display.setCursor(97, 20);
    display.println("BOOST");
    display.setTextColor(WHITE);
    display.setTextSize(3);
    display.setCursor(0, 10);
    display.println(((boostmbar * 0.001) - 0.865)*14);    // calibrated for a 2.5 bar sensor in Denver (+/- 1psi)


}

  delay(1);
  display.display();
  display.clearDisplay();

  delay(10); // delay half second between numbers
}
