void openVent() {
  eventlog.println(flightTimeStr() + "  AC  Open Vent");
  digitalWrite(ventClose, LOW);
  digitalWrite(ventOpen, HIGH);
  unsigned long t = millis();
  while (analogRead(ventFeed) < 1023 && millis() - t < 10000) {
    delay(50);
  }
  digitalWrite(ventOpen, LOW);
  sendXBee("Vent Opened");
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
  while (analogRead(ventFeed) > 30 && millis() - t < 10000) {
    delay(50);
  }
  digitalWrite(ventClose, LOW);
  sendXBee("Vent Opened");
  if (ventIsOpen) {
    ventIsOpen = false;
    totalOpen += millis() - openTime;
  }
}


void openForTime(int timeOpen) {
  timeOpen *= 1000;
  unsigned long t = millis();
  openVent();
  while (millis() - t < timeOpen) {
    updateGPS();
    xBeeCommand();
    if (analogRead(ventFeed) < 30) return;
  }
  closeVent();
}


void extendArrow() {
  eventlog.println(flightTimeStr() + "  AC  Extending Arrow");
  digitalWrite(arrowRet, LOW);
  digitalWrite(arrowExt, HIGH);
  unsigned long t = millis();
  while (analogRead(arrowFeed) < 1023 && millis() - t < 10000) {
    delay(50);
  }
  digitalWrite(arrowExt, LOW);
  sendXBee("Arrow Extended");
}


void retractArrow() {
  eventlog.println(flightTimeStr() + "  AC  Retracting Arrow");
  digitalWrite(arrowExt, LOW);
  digitalWrite(arrowRet, HIGH);
  unsigned long t = millis();
  while (analogRead(arrowFeed) > 30 && millis() - t < 10000) {
    delay(50);
  }
  digitalWrite(arrowRet, LOW);
  sendXBee("Arrow Retracted");
}


void Legolas() {    //full arrow cutdown routine which, like The Hobbit movies,
  extendArrow();    //features an unneccessary appearance by everyone's favorite elf.
  unsigned long t = millis();
  while ((millis() - t < 10000) && !isBurst()) {
    updateGPS();
    xBeeCommand();
  }
  retractArrow();
}

