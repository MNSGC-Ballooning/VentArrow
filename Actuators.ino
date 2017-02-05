//Variables to store actuator value ranges
int ventMin = 165;
int ventMax = 1023;
int arrowMin = 70;
int ventTime = 8; //time in seconds to allow vent to open or close
int arrowTime = 10; //time to allow arrow to extend or retract
byte timesClosed = 0;

void openVent() {
  openVent(0);
}

void openVent(unsigned long callerID) {
  logAction("Open Vent");
  digitalWrite(ventClose, LOW);
  digitalWrite(ventOpen, HIGH);
  removeVents(callerID);
  StopVentAction* stopVent = new StopVentAction(ventTime); //8 seconds is plenty on ground, but may be insufficient at altitude. Needs more testing
  actions.push_back(stopVent);
  if (!ventIsOpen) {      //Used to track total time vent has been open during flight
    ventIsOpen = true;
    openTime = millis();
  }
}

void closeVent() {
  closeVent(0);
}

void closeVent(unsigned long callerID) {
  logAction("Close Vent");
  digitalWrite(ventOpen, LOW);
  digitalWrite(ventClose, HIGH);
  removeVents(callerID);
  StopVentAction* stopVent = new StopVentAction(ventTime);
  actions.push_back(stopVent);
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
  if (!ventIsOpen) {
    if (IOCcheck()){
      sendXBee("IOC confirm vent closed");
      timesClosed = 0;
    }
    else if (timesClosed < 3) {
      sendXBee("IOC fail, closing again");
      timesClosed++;
      CloseVentAction* closeVent = new CloseVentAction(1);
      actions.push_back(closeVent);
    }
    else {
      sendXBee("no close detected after 3 attempts, possible IOC failure");
      timesClosed = 0;
    }
  }
}

void removeVents(unsigned long callerID) {
  for (vector<Action*>::iterator it = actions.begin(); it < actions.end(); it++) {
    if ((*it)->isRemovedOn("vent") && ((*it)->getID() != callerID)) {
      delete *it;
      it--;
      actions.erase(it + 1);
    }
  }
}

int ventPercent() {
  int pos = analogRead(ventFeed);
  pos -= ventMin;
  return (pos * 100 / (ventMax - ventMin));
}


void openForTime(int timeOpen) {  //opens vent for given number of seconds, then closes automatically
  if (timeOpen == 0) return;
  openVent();
  CloseVentAction* closeVent = new CloseVentAction(ventTime + timeOpen);
  actions.push_back(closeVent);
}

void calibrateVent() {  //fully opens and closes vent to reassign vent min/max values. Use only on ground prior to flight
  unsigned long t = millis();
  while (millis() - t < (ventTime + 2) * 1000) {
    digitalWrite(ventOpen, HIGH);
    updateSensors();
  }
  digitalWrite(ventOpen, LOW);
  ventMax = analogRead(ventFeed);
  if (ventMax == 1023)
    ventMax = 1024;
  t = millis();
  while (millis() - t < (ventTime + 2) * 1000) {
    digitalWrite(ventClose, HIGH);
    updateSensors();
  }
  digitalWrite(ventClose, LOW);
  ventMin = analogRead(ventFeed) - 3;
  sendXBee("Min: " + String(ventMin) + " Max: " + String(ventMax));
}

void extendArrow() {
  extendArrow(0);
}

void extendArrow(unsigned long callerID) {
  logAction("Extending Arrow");
  digitalWrite(arrowRet, LOW);
  digitalWrite(arrowExt, HIGH);
  removeArrows(callerID);
  StopArrowAction* stopArrow = new StopArrowAction(arrowTime);
  actions.push_back(stopArrow);
}

void retractArrow() {
  retractArrow(0);
}

void retractArrow(unsigned long callerID) {
  logAction("Retracting Arrow");
  digitalWrite(arrowExt, LOW);
  digitalWrite(arrowRet, HIGH);
  removeArrows(callerID);
  StopArrowAction* stopArrow = new StopArrowAction(arrowTime);
  actions.push_back(stopArrow);
}

void stopArrow() {
  digitalWrite(arrowExt, LOW);
  digitalWrite(arrowRet, LOW);
  if (analogRead(arrowFeed) > 1015)
    sendXBee("Arrow Extended");
  else if (analogRead(arrowFeed) < arrowMin + 5)
    sendXBee("Arrow Retracted");
  else {
    sendXBee("Arrow Actuation failed");
    sendXBee("Arrow at " + String(analogRead(arrowFeed)));
  }
}

void removeArrows(unsigned long callerID) {
  for (vector<Action*>::iterator it = actions.begin(); it < actions.end(); it++) {
    if ((*it)->isRemovedOn("arrow") && ((*it)->getID() != callerID)) {
      delete *it;
      it--;
      actions.erase(it + 1);
    }
  }
}

void Legolas() {    //full arrow cutdown routine which, like the Hobbit movies,
  extendArrow();    //features an unneccessary appearance by everyone's favorite elf.
  RetractArrowAction* retractArrow = new RetractArrowAction(10 + arrowTime);
  actions.push_back(retractArrow);
}

