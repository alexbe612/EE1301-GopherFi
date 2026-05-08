#include <iostream>
#include <cstdlib>
#include "Particle.h"
#include "neopixel.h"
#include <TinyGPS++.h> // A class that easily converts the GPS data into a readable format

// GPS

TinyGPSPlus gps; // Object gps of the TinyGPSPlus class defined, with lat and lon variables
double longitude;
double latitude;
int charsProcessed = 0; // These 3 ints are cloud variables used for testing
int passedChecksum = 0;
int failedChecksum = 0;
int signal_strength = 0; // Percentage value of the WiFi signal strength
void GopherFiGPS();

// iLED

int PIXEL_COUNT = 2; // Defines 2 iLEDs: One for the button press, and the other for signal strength
#define PIXEL_PIN SPI
int PIXEL_TYPE = WS2812;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
void StrengthLED(); 

// Button

int button = D3; 
bool button_state = FALSE;
int prevButton = LOW;
unsigned long int timeToToggleLED;
void Button();

// Publish to cloud

const char *eventName = "sheet-data"; 
unsigned long lastPrintTime = 0;
unsigned long lastPublishTime = 0;
void PublishData();



SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(LOG_LEVEL_INFO);


void setup() {

  Serial.begin(9600);
  Serial1.begin(9600); // Baud of the GPS module
  
  delay(2000); 
  
  // Sets the button time to avoid jitters and the pin

  strip.begin();
  pinMode(button, INPUT_PULLDOWN);
  timeToToggleLED = millis() + 500;

  // Note: All particle varibles were used for testing purposes. 

  Particle.variable("Strength", signal_strength);
  Particle.variable("longitude", longitude);
  Particle.variable("latitude", latitude);
  Particle.variable("chars", charsProcessed);
  Particle.variable("passed", passedChecksum); 
  Particle.variable("failed", failedChecksum); 

}


void loop() {
  
/* Loops both the button and the GPS function. The GPS function passes latitude and longitude
data every millisecond to avoid a location error. Also, the testing variables for chars and checks
allowed us to determine if a proper signal was found. More detail on those varibles below. */

Button();
GopherFiGPS();

}

void Button() {

int blue = strip.Color(0, 0, 255); 
int clear = strip.Color(0,0,0);
int currButton = digitalRead(button);

// If-statement passes once per press and avoids key chatter - Courtisy of IoT lab 4

  if ((currButton == HIGH) && (prevButton == LOW) && (!button_state)) {
    Serial.println("Button Pressed"); // Serial Monitor test
    strip.setPixelColor(0, blue); // One iLED is used to confirm that the button was in fact pressed
    timeToToggleLED = millis() + 500;
    button_state = TRUE;
    
    WiFiSignal strength = WiFi.RSSI(); // RSSI value for signal strength (read in dB, typically between -30 and -120)
    signal_strength = strength.getStrength(); // Converts to a percentage 

    Serial.print("Signal Strength: "); // Serial Monitor test
    Serial.print(signal_strength);
    Serial.println(" %"); 
    StrengthLED(); // For every button press, the iLED function and PublishData function passes
    PublishData();
    strip.show();
  }
  prevButton = currButton;
  unsigned long int currentTime = millis();

  // If-statement passes after 500 milliseconds, defined with timeToToggleLED in the last if-statement
  // This resets the button and allows for another press
  if ((currentTime > timeToToggleLED) && (button_state)) {
    strip.setPixelColor(0, clear); 
    Serial.println("Clear"); // Serial Monitor test
    button_state = FALSE; 
    strip.show();
  }
  
}



void StrengthLED() {

// Takes the signal strength as a percentage and changes the iLED to a red to green scale to indicate strength
// Green is high, red is low, mix in between

// Note: The signal strength values don't match to the exact same colors corresponding to the .html file
//       The iLED wouldn't show the exact same color shown on the .html file with the same rgb values, so
//       we selected values for the iLED that seemed like a proper scale. The .html rgb values were more 
//       towards our highest and lowest values obtained through testing.

int r=0;
int g=0;

if (signal_strength >= 80) {
  g = 255;
}
else if (signal_strength >= 70) {
  g = 255;
  r = 115;
}
else if (signal_strength >= 60) {
  g = 255;
  r = 180;
}
else if (signal_strength >= 50) {
  g = 255;
  r = 255;
}
else if (signal_strength >= 40) {
  g = 210;
  r = 255;
}
else if (signal_strength >= 30) {
  g = 140;
  r = 255;
}
else if (signal_strength >= 20) {
  g = 70;
  r = 255;
}
else {
  r = 255;
}

int color = strip.Color(g,r,0);
strip.setPixelColor(1, color);
  
}




void GopherFiGPS() {

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  /* charsProcesses tells us if the GPS is hooked up properly. The module processes garbage data
  and converts it to valid data if a wifi signal is found. The garbage data is passed as a char
  
  passedChecksum tells us if the GPS is giving us valid data

  failedChecksum tells us invalid data

  These helped us fix issues we had with the GPS hooking up properly.
  */

  charsProcessed = gps.charsProcessed(); 
  passedChecksum = gps.passedChecksum();
  failedChecksum = gps.failedChecksum();

  if (gps.location.isValid()) {

    // Latitude and longitude are read every millisecond

    latitude = (double)gps.location.lat(); 
    longitude = (double)gps.location.lng();
  }


}
void PublishData() {

  char buf[128];

    // Publishes an event to particle.io with 2 floats and 1 int, representing latitude, longitude, and signal strength.

    snprintf(buf, sizeof(buf), "[%f,%f,%d]", latitude, longitude, signal_strength);

    Particle.publish(eventName, buf, PRIVATE);
    Log.info("published: %s", buf);

}

// Once the event is published, the data is read by google sheets. More on that in README and in map_spreadsheet.js