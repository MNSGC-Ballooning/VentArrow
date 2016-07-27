/*  This code is designed to run on a Teensy 3.2 on board the VentArrow MkII.
    Currently supports use of GPS, SD datalogging, and xBee communications.
    Diff Pressure sensor to be added later.
    Can be used as both an autonomous system or commanded from ground via RFD relay. */

/*  By: Ryan Bowers, with assistance from Danny Toth
    Based off VentArrow MkI code by Christopher Gosch */


/*  Teensy pin connections:
     -------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used             | Notes

     Adafruit Ultimate GPS        | D0-1 (RX/TX1)         | Red LED connects to fix pin on GPS
     Series 2 xBee                | D7-8 (RX/TX3)         |
     Sparkfun SD Breakout         | D10-13 (CS,DO,DI,SCK1)|
     Power LED (Green)            | D23                   | Turns on at startup - off indicates loss of power or critical error
     Data LED (Yellow)            | D22                   | Flashes when writing to SD card
     Vent Actuator                | D16-17, D18 (A4)      | Set D16 high to open, D17 high to close. D18/A4 is feedback
     Arrow Actuator               | D19-20, D21 (A7)      | Set D19 high to exend, D20 high to retract. D21/A7 is feedback
     -------------------------------------------------------------------------------------------------------------------------
*/

//Libraries
#include <SD.h>
#include <Adafruit_GPS.h>

//pin declarations
#define powerLED 22
#define dataLED 23
#define ventOpen 16
#define ventClose 17
#define ventFeed A4
#define arrowExt 19
#define arrowRet 20
#define arrowFeed A7
#define chipSelect 10

HardwareSerial gpsSerial = Serial1, xBee = Serial3;
Adafruit_GPS GPS(&gpsSerial);

File datalog, eventlog;
String filename = "VentAr";
char datafile[13], eventfile[13];

const String xBeeID = "VA";

boolean startup = true;
unsigned long flightStart = 0;

//============================================================================================================================

void setup() {
  //set up pin modes
  pinMode(powerLED, OUTPUT);
  pinMode(dataLED, OUTPUT);
  pinMode(ventOpen, OUTPUT);
  pinMode(ventClose, OUTPUT);
  pinMode(ventFeed, INPUT);
  pinMode(arrowExt, OUTPUT);
  pinMode(arrowRet, OUTPUT);
  pinMode(arrowFeed, INPUT);
  pinMode(chipSelect, OUTPUT);

  digitalWrite(powerLED, HIGH); //turn on power LED at startup

  //begin all serial lines
  GPS.begin(9600);
  gpsSerial.begin(9600);
  xBee.begin(9600);

  //GPS setup and config
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  
  //initialize SD card
  if (!SD.begin(chipSelect)) {
    while (true) {                  //power LED will blink if no card is inserted
      digitalWrite(powerLED, HIGH);
      delay(500);
      digitalWrite(powerLED, LOW);
      delay(500);
    }
  }
  for (int i = 0; i < 100; i++) {                 //check for existing files from previous runs of program...
    (filename + String(i / 10) + String(i % 10) + ".csv").toCharArray(datafile, sizeof(datafile));
    if (!SD.exists(datafile)) {                   //...and make sure a new file is opened each time
      openDatalog();
      (filename + String(i / 10) + String(i % 10) + ".txt").toCharArray(eventfile, sizeof(eventfile));
      openEventlog();
      break;
    }
  }
  if (!datalog) {                   //both power and data LEDs will blink together if card is inserted but file fails to be created
    while (true) {
      digitalWrite(powerLED, HIGH);
      digitalWrite(dataLED, HIGH);
      delay(500);
      digitalWrite(powerLED, LOW);
      digitalWrite(dataLED, LOW);
      delay(500);
    }
  }
  
  String Header = "Flight Time, Lat, Long, Altitude, Date, Hour:Min:Sec";
  datalog.println(Header);  //set up datalog format
  
  closeDatalog();
  closeEventlog();

  sendXBee("Setup Complete");
  sendXBee("Awaiting Startup");
  while (true) {
    xBeeCommand();
    if (!startup) break;
    delay(100);
  }
}

//============================================================================================================================

void loop(){
  updateGPS();
  xBeeCommand();
}

