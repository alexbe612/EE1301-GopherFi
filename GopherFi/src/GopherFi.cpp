#include <iostream>
#include <cstdlib>
#include "Particle.h"
#include "neopixel.h"
#include <TinyGPS++.h>
#include <fstream>
#include <fcntl.h>

TinyGPSPlus gps;

double longitude;
double latitude;

int charsProcessed = 0;
int passedChecksum = 0;
int failedChecksum = 0;

unsigned long lastPrintTime = 0;
unsigned long lastPublishTime = 0;

int PIXEL_COUNT = 2;
#define PIXEL_PIN SPI
int PIXEL_TYPE = WS2812;
int button = D3;
bool button_state = FALSE;
void StrengthLED();

int prevButton = LOW;
unsigned long int timeToToggleLED;
void Button();

void GopherFiGPS();
int signal_strength = 0;

void PublishData();
const char *eventName = "sheet-data";



Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

SYSTEM_MODE(AUTOMATIC);

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

// This function trickles the offline data to the cloud 1 per second
int uploadToGoogleSheets(String command) {
  Serial.println("--- INITIATING GOOGLE SHEETS UPLOAD ---");
  
  // Open the file using standard C fopen (great for reading line-by-line)
  FILE *fp = fopen("/usr/gopherfi.csv", "r");
  
  if (fp != NULL) {
    char line[128]; 
    
    // Read the file one line at a time until the end
    while (fgets(line, sizeof(line), fp) != NULL) {
      
      // Remove the newline character from the end of the string
      String payload = String(line).trim();
      
      // Publish the single line to the Particle Cloud Webhook
      Particle.publish("Google_Sheet_Upload", payload, PRIVATE);
      Serial.println("Uploaded offline point: " + payload);
      
      // CRITICAL: We must wait 1.1 seconds between publishes so Particle doesn't ban us
      delay(1100); 
    }
    
    fclose(fp);
    Serial.println("--- ALL OFFLINE DATA UPLOADED ---");
    
    // Clear the file
    remove("/usr/gopherfi.csv");
    Serial.println("Internal file cleared.");
    
    return 1;
  } else {
    Serial.println("No offline data to upload.");
    return -1;
  }
}

void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);
 // Force the Adafruit GPS to only use standard GPS satellites
 Serial1.println("$PMTK353,1,0,0,0,0*2A");
  delay(2000); 
  
  strip.begin();
  pinMode(button, INPUT_PULLDOWN);
  
  timeToToggleLED = millis() + 500;
  Particle.variable("Strength", signal_strength);

  
  Serial.println("GPS-Only Serial Test Started!");
  Serial.println("Waiting for satellite fix... (Look for the slow blink!)");
  Serial.println("--------------------------------------------------");

  Particle.variable("longitude", longitude);
  Particle.variable("latitude", latitude);
  Particle.variable("chars", charsProcessed);
  Particle.variable("passed", passedChecksum); 
  Particle.variable("failed", failedChecksum); 

}


void loop() {
  
Button();
GopherFiGPS();

}

void Button() {
  
int blue = strip.Color(0, 0, 255);
int clear = strip.Color(0,0,0);


  int currButton = digitalRead(button);
  if ((currButton == HIGH) && (prevButton == LOW) && !button_state) {
    Serial.println("Button Pressed"); // Serial Monitor test
    strip.setPixelColor(0, blue);
    timeToToggleLED = millis() + 500;
    button_state = TRUE;
    
    WiFiSignal strength = WiFi.RSSI(); 
    signal_strength = strength.getStrength();

    Serial.print("Signal Strength: "); 
    Serial.print(signal_strength);
    Serial.println(" %"); // Serial Monitor test
    StrengthLED();
    PublishData();
    strip.show();
  }
  prevButton = currButton;
  unsigned long int currentTime = millis();

  if ((currentTime > timeToToggleLED) && (button_state)) {
    strip.setPixelColor(0, clear);
    Serial.println("Clear"); // Serial Monitor test
    button_state = FALSE;
    strip.show();
  }
  
}




void StrengthLED() {
/*
int green = strip.Color(255, 0, 0);
int green_yellow = strip.Color(250, 157, 0);
int yellow = strip.Color(207, 250, 0);
int yellow_orange = strip.Color(130, 250, 0);
int orange = strip.Color(86, 250, 0);
int red = strip.Color(0, 255, 0);
*/

int r=0;
int g=0;
int b=0;

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

int color = strip.Color(g,r,b);
strip.setPixelColor(1, color);
  
}




void GopherFiGPS() {

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  charsProcessed = gps.charsProcessed();
  passedChecksum = gps.passedChecksum();
  failedChecksum = gps.failedChecksum();

  if (gps.location.isValid()) {
    latitude = (double)gps.location.lat();
    longitude = (double)gps.location.lng();
  }

// Every 10 seconds, push live data OR save it locally if offline
  if (millis() - lastPublishTime > 10000) {
    lastPublishTime = millis();
    
    // We only want to record if we actually have a valid GPS lock
    if (gps.location.isValid()) {
        
        // Format exactly like the online data so Google Sheets accepts it
        String logData = String::format("[%f,%f,%f]", latitude, longitude, (double)signal_strength);
        // CHECK WI-FI CONNECTION
        if (Particle.connected()) {
            // WE ARE ONLINE - Send to the Particle Console
            Particle.publish("GopherFi_Live", logData, PRIVATE);
            Serial.println("Published to Cloud: " + logData);
        } 
        else {
            // WE ARE OFFLINE - Save to internal flash memory
            int fd = open("/usr/gopherfi.csv", O_WRONLY | O_CREAT | O_APPEND);
            
            if (fd != -1) {
                String fileLine = logData + "\n"; // Add a line break for the file
                write(fd, fileLine.c_str(), fileLine.length());
                close(fd);
                Serial.println("Saved locally (Offline): " + logData);
            } else {
                Serial.println("File System Error! Could not save.");
            }
        }
    } else {
        Serial.println("Waiting for GPS lock to record data...");
    }
  }

}
void PublishData() {

  char buf[128];

    snprintf(buf, sizeof(buf), "[%f,%f,%d]", latitude, longitude, signal_strength);

    Particle.publish(eventName, buf, PRIVATE);
    Log.info("published: %s", buf);

}