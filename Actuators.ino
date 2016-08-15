void openVent() {
  eventlog.println(flightTimeStr() + "  AC  Open Vent");
  digitalWrite(ventClose, LOW);
  digitalWrite(ventOpen, HIGH);
  unsigned long t = millis();
  while (analogRead(ventFeed) < ventMax && millis() - t < 10000) {
    updateGPS();
    delay(50);
  }
  digitalWrite(ventOpen, LOW);
  if (analogRead(ventFeed) > ventMax - 5)
    sendXBee("Vent Opened");
  else {
    sendXBee("Open Vent failed");
    sendXBee("Vent " + String(ventPercent()) + "% open");
  }
  if (!ventIsOpen) {
    ventIsOpen = true;
    openTime = millis();
  }
}


void closeVent() {
  eventlog.println(flightTimeStr() + "  AC  Close Vent");
  digitalWrite(ventOpen, LOW);
  digitalWrite(ventClose, HIGH);
  unsigned long t = millis();
  while (analogRead(ventFeed) > ventMin && millis() - t < 10000) {
    updateGPS();
    delay(50);
  }
  digitalWrite(ventClose, LOW);
  if (analogRead(ventFeed) < ventMin + 5)
    sendXBee("Vent Closed");
  else {
    sendXBee("Vent Close failed");
    sendXBee("Vent " + String(ventPercent()) + "% open");
  }
  if (ventIsOpen) {
    ventIsOpen = false;
    totalOpen += millis() - openTime;
  }
}

int ventPercent() {
  int pos = analogRead(ventFeed);
  pos -= ventMin;
  return (pos * 100 / (ventMax - ventMin));
}


void openForTime(int timeOpen) {
  timeOpen *= 1000;
  unsigned long t = millis();
  openVent();
  while (millis() - t < timeOpen) {
    updateGPS();
    xBeeCommand();
    if (analogRead(ventFeed) < ventMin + 5) return;
  }
  closeVent();
}

void calibrateVent() {
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
  ventMin = analogRead(ventFeed);
  eventlog.println(flightTimeStr() + "  AC  New calibration: ventMin " + String(ventMin) + " ventMax " + String(ventMax));
}


void extendArrow() {
  eventlog.println(flightTimeStr() + "  AC  Extending Arrow");
  digitalWrite(arrowRet, LOW);
  digitalWrite(arrowExt, HIGH);
  unsigned long t = millis();
  while (analogRead(arrowFeed) < 1020 && millis() - t < 10000) {
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
  eventlog.println(flightTimeStr() + "  AC  Retracting Arrow");
  digitalWrite(arrowExt, LOW);
  digitalWrite(arrowRet, HIGH);
  unsigned long t = millis();
  while (analogRead(arrowFeed) > arrowMin && millis() - t < 10000) {
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

