boolean datalogOpen = false;
boolean eventlogOpen = false;

//The following four functions handle both opening files and controlling the data indicator LED
void openDatalog() {
  if (!datalogOpen) {
    datalogA = SD.open(datafileA, FILE_WRITE);
    datalogB = SD.open(datafileB, FILE_WRITE);
    datalogOpen = true;
    digitalWrite(dataLED, HIGH);
  }
}

void closeDatalog() {
  if (datalogOpen) {
    datalogA.close();
    datalogB.close();
    datalogOpen = false;
    if (!eventlogOpen)
      digitalWrite(dataLED, LOW);
  }
}

void openEventlog() {
  if (!eventlogOpen) {
    eventlogA = SD.open(eventfileA, FILE_WRITE);
    eventlogB = SD.open(eventfileB, FILE_WRITE);
    eventlogOpen = true;
    digitalWrite(dataLED, HIGH);
  }
}

void closeEventlog() {
  if (eventlogOpen) {
    eventlogA.close();
    eventlogB.close();
    eventlogOpen = false;
    if (!datalogOpen)
      digitalWrite(dataLED, LOW);
  }
}

//Takes a string describing any event that takes place and records it in the eventlog with a timestamp. 
void logEvent(String event) {
  openEventlog();
  eventlogA.println(flightTimeStr() + "  AC  " + event);
  eventlogB.println(flightTimeStr() + "  AC  " + event);
  closeEventlog();
}

