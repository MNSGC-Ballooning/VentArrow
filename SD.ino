boolean datalogOpen = false;
boolean eventlogOpen = false;

//The following four functions handle both opening files and controlling the data indicator LED
void openDatalog() {
  if (!datalogOpen) {
    datalog = SD.open(datafile, FILE_WRITE);
    datalogOpen = true;
    digitalWrite(dataLED, HIGH);
  }
}

void closeDatalog() {
  if (datalogOpen) {
    datalog.close();
    datalogOpen = false;
    if (!eventlogOpen)
      digitalWrite(dataLED, LOW);
  }
}

void openEventlog() {
  if (!eventlogOpen) {
    eventlog = SD.open(eventfile, FILE_WRITE);
    eventlogOpen = true;
    digitalWrite(dataLED, HIGH);
  }
}

void closeEventlog() {
  if (eventlogOpen) {
    eventlog.close();
    eventlogOpen = false;
    if (!datalogOpen)
      digitalWrite(dataLED, LOW);
  }
}

//Takes a string describing any event that takes place and records it in the eventlog with a timestamp. 
void logEvent(String event) {
  openEventlog();
  eventlog.println(flightTimeStr() + "  AC  " + event);
  closeEventlog();
}

