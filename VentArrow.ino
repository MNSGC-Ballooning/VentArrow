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
     Series 1 xBee                | D7-8 (RX/TX3)         |
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
#include <vector>
namespace std { //for whatever reason, std libraries aren't well supported on Teensy - this seems to clear up most errors
  void __throw_bad_alloc()
  {
    Serial.println("Unable to allocate memory");
  }

  void __throw_length_error( char const*e )
  {
    Serial.print("Length Error :");
    Serial.println(e);
  }
}
using namespace std;

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

class AutoVent { //Class for automatic venting events. Implementation is in Autopilot.ino
  private:
    int alts[4], times[4];
    byte reached;
  public:
    int targetAlt;
    int ventingTime;
    int rate1();
    int rate2();
    boolean autoCheck();
    void newRate();
    AutoVent(int alt, int vent);
};

//define Action class and subclasses used throughout code
class Action {
  protected:
    unsigned long startTime;
  public:
    boolean checkTimer() {
      if (millis() > startTime) return true;
      else return false;
    }
    virtual void doAction() {}
    virtual boolean isRemovedOn(String type) {return false;}
    Action(int t) {startTime = millis() + t * 1000;}
};

class OpenVentAction:public Action {
  public:
    void doAction();
    boolean isRemovedOn(String type);
    OpenVentAction(int t) : Action(t){}
};

class CloseVentAction:public Action {
  public:
    void doAction();
    boolean isRemovedOn(String type);
    CloseVentAction(int t) : Action(t){}
};

class StopVentAction:public Action {
  public:
    void doAction();
    boolean isRemovedOn(String type);
    StopVentAction(int t) : Action(t){}
};

class ExtendArrowAction:public Action {
  public:
    void doAction();
    boolean isRemovedOn(String type);
    ExtendArrowAction(int t) : Action(t){}
};

class RetractArrowAction:public Action {
  public:
    void doAction();
    boolean isRemovedOn(String type);
    RetractArrowAction(int t) : Action(t){}
};

class StopArrowAction:public Action {
  public:
    void doAction();
    boolean isRemovedOn(String type);
    StopArrowAction(int t) : Action(t){}
};

class CheckRateAction:public Action {
  public:
    void doAction();
    CheckRateAction(int t) : Action(t){}
};

class StartBurstCheck:public Action {
  public:
    void doAction();
    StartBurstCheck(int t) : Action(t){}
};

class BurstCheck:public Action {
  public:
    void doAction();
    BurstCheck(int t) : Action(t){}
};

class Beacon:public Action {
  private:
    float GPSalt;
    int GPStime;
  public:
    void doAction();
    Beacon(int t);
};

const String xBeeID = "VA";

HardwareSerial gpsSerial = Serial1;
Adafruit_GPS GPS(&gpsSerial); //Constructor for GPS object

File datalogA, datalogB, eventlogA, eventlogB;
char datafileA[13], datafileB[13], eventfileA[13], eventfileB[13];

String filename = "Vent";
int cutTime = 120;        //Time in minutes after flight start to auto-cutdown
int cutAlt = 900000;      //Altitude in ft to auto cutdown
AutoVent autos[] = {AutoVent(50, 120), AutoVent(70, 120), AutoVent(999, 0)}; //put any planned AutoVents here
vector<Action*> actions;

boolean startup = true;
boolean ventIsOpen = false;
unsigned long flightStart = 0;
unsigned long totalOpen = 0;
unsigned long openTime;
boolean hasBurst = false;
boolean checkBurst = false;
boolean checkRate = false;

int GPSstartTime;
int days = 0;
boolean newDay = false;
boolean firstFix = false;

//============================================================================================================================

void setup() {
  actions.reserve(10);
  
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
    (filename + String(i / 10) + String(i % 10) + "A" + ".csv").toCharArray(datafileA, sizeof(datafileA));
    if (!SD.exists(datafileA)) {                   //...and make sure a new file is opened each time
      (filename + String(i / 10) + String(i % 10) + "B" + ".csv").toCharArray(datafileB, sizeof(datafileB));
      openDatalog();
      (filename + String(i / 10) + String(i % 10) + "A" + ".txt").toCharArray(eventfileA, sizeof(eventfileA));
      (filename + String(i / 10) + String(i % 10) + "B" + ".txt").toCharArray(eventfileB, sizeof(eventfileB));
      openEventlog();
      break;
    }
  }
  if (!datalogA) {                   //both power and data LEDs will blink together if card is inserted but file fails to be created
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
  datalogA.println(Header);
  datalogB.println(Header);    //set up datalog format
  closeDatalog();

  sendXBee("Setup Complete");
  sendXBee("Awaiting Startup");
  
  while (true) { //Don't begin autopilot or full sensor logging until flight start command received
    updateGPS();
    xBeeCommand();
    checkActions();
    if (!startup) break;
  }
}

//============================================================================================================================

void loop(){
  updateGPS();
  xBeeCommand();
  autopilot();
  checkActions();
}

