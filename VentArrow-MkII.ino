/*  This code is designed to run on a Teensy 3.2 on board the VentArrow MkII.
    Currently supports use of GPS, SD datalogging, and xBee communications.
    Pressure sensors to be added later.
    Can be used as both an autonomous system or commanded from ground via RFD relay. */

/*  By: Ryan Bowers, with assistance from Danny Toth
    Based off VentArrow MkI code by Christopher Gosch */


/*  Teensy pin connections:
     -------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used             | Notes

     Adafruit Ultimate GPS        | D0-1 (RX/TX1)         | Red LED connects to fix pin on GPS
     Series 2 xBee                | D7-8 (RX/TX3)         |
     Sparkfun SD Breakout         | D10-13 (CS,DO,DI,SCK1)|
     Power LED (Green)            | D6                    | Turns on at startup - off indicates loss of power or critical error
     Data LED (Yellow)            | D9                    | Flashes when writing to SD card
     Vent Actuator                | D4-5, D14 (A0)        | Set D5 high to open, D4 high to close. D14/A0 is feedback
     Arrow Actuator               | D2-3, D15 (A1)        | Set D3 high to exend, D2 high to retract. D15/A1 is feedback
     -------------------------------------------------------------------------------------------------------------------------
*/

//Libraries
#include <SD.h>
#include <Adafruit_GPS.h>

//pin declarations
#define powerLED 6
#define dataLED 9
#define ventOpen 5
#define ventClose 4
#define ventFeed A0
#define arrowExt 3
#define arrowRet 2
#define arrowFeed A1
#define chipSelect 10

class AutoVent {
  private:
    int alts[4], times[4];
    byte reached;
  public:
    int targetAlt;
    int ventTime;
    int rate1();
    int rate2();
    void autoCheck();
    AutoVent(int alt, int vent);
};

const String xBeeID = "VA";

HardwareSerial gpsSerial = Serial1;
Adafruit_GPS GPS(&gpsSerial);

File datalog, eventlog;
char datafile[13], eventfile[13];

String filename = "VentAr";
int cutTime = 120;        //Time in minutes after flight start to auto-cutdown
int cutAlt = 900000;      //Altitude in ft to auto cutdown
AutoVent autos[] = {AutoVent(50, 120), AutoVent(70, 120), AutoVent(999, 0)};

boolean startup = true;
boolean ventIsOpen = false;
unsigned long flightStart = 0;
unsigned long totalOpen = 0;
unsigned long openTime;
boolean hasBurst = false;

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
  Serial3.begin(9600);

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

  sendXBee("Setup Complete");
  sendXBee("Awaiting Startup");
  
  while (true) {
    updateGPS();
    xBeeCommand();
    if (!startup) break;
    delay(100);
  }
}

//============================================================================================================================

void loop(){
  updateGPS();
  xBeeCommand();
  autopilot();
}

