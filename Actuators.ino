//Variables to store actuator value ranges
int ventMin = 165;
int ventMax = 1023;
int arrowMin = 70;

void openVent() {
  logEvent("Open Vent");
  digitalWrite(ventClose, LOW);
  digitalWrite(ventOpen, HIGH);
  Event stopVent ("stopVent", 8); //8 seconds is plenty on ground, but may be insufficient at altitude. Needs more testing
  events.push_back(stopVent);
  if (!ventIsOpen) {      //Used to track total time vent has been open during flight
    ventIsOpen = true;
    openTime = millis();
  }
}

void closeVent() {
  logEvent("Close Vent");
  digitalWrite(ventOpen, LOW);
  digitalWrite(ventClose, HIGH);
  unsigned long t = millis();
  while (millis() - t < 8000) {
    updateGPS();
    delay(50);
  }
  sendXBee("Vent at " + String(analogRead(ventFeed)));
  sendXBee("Vent " + String(ventPercent()) + "% open");
  if (ventIsOpen) {
    ventIsOpen = false;
    totalOpen += millis() - openTime;
  }
}

void stopVent() {
  digitalWrite(ventOpen, LOW);
  digitalWrite(ventClose, LOW);
  sendXBee("Vent at " + String(analogRead(ventFeed)));
  //Attempts to calculate relative amount open based on above min/max values
  sendXBee("Vent " + String(ventPercent()) + "% open");
  if (!ventIsOpen) {      //Used to track total time vent has been open during flight
    ventIsOpen = true;
    openTime = millis();
  }
}

int ventPercent() {
  int pos = analogRead(ventFeed);
  pos -= ventMin;
  return (pos * 100 / (ventMax - ventMin));
}


void openForTime(int timeOpen) {  //opens vent for given number of seconds, then closes automatically
  timeOpen *= 1000;
  unsigned long t = millis();
  openVent();
  while (millis() - t < timeOpen) {
    updateGPS();
    xBeeCommand();
  }
  closeVent();
}

void calibrateVent() {  //fully opens and closes vent to reassign vent min/max values. Use only on ground prior to flight
  unsigned long t = millis();
  while (millis() - t < 10000) {
    digitalWrite(ventOpen, HIGH);
    updateGPS();
  }
  digitalWrite(ventOpen, LOW);
  ventMax = analogRead(ventFeed);
  if (ventMax == 1023)
    ventMax = 1024;
  t = millis();
  while (millis() - t < 10000) {
    digitalWrite(ventClose, HIGH);
    updateGPS();
  }
  digitalWrite(ventClose, LOW);
  ventMin = analogRead(ventFeed) - 3;
  sendXBee("Min: " + String(ventMin) + " Max: " + String(ventMax));
}


void extendArrow() {
  logEvent("Extending Arrow");
  digitalWrite(arrowRet, LOW);
  digitalWrite(arrowExt, HIGH);
  unsigned long t = millis();
  while (millis() - t < 10000) {
    updateGPS();
    delay(50);
  }
  digitalWrite(arrowExt, LOW);
  if (analogRead(arrowFeed) > 1015)
    sendXBee("Arrow Extended");
  else
    sendXBee("Arrow Extend failed");
}


void retractArrow() {
  logEvent("Retracting Arrow");
  digitalWrite(arrowExt, LOW);
  digitalWrite(arrowRet, HIGH);
  unsigned long t = millis();
  while (millis() - t < 10000) {
    updateGPS();
    delay(50);
  }
  digitalWrite(arrowRet, LOW);
  if (analogRead(arrowFeed) < arrowMin + 5)
    sendXBee("Arrow Retracted");
  else
    sendXBee("Arrow Retract failed");
}


void Legolas() {    //full arrow cutdown routine which, like the Hobbit movies,
  extendArrow();    //features an unneccessary appearance by everyone's favorite elf.
  unsigned long t = millis();
  while (millis() - t < 10000) {
    updateGPS();
    xBeeCommand();
  }
  retractArrow();
}

