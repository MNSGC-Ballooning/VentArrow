unsigned long timer1 = millis();
unsigned long timer2 = timer1;
int logRate = 1000;   //interval in millis between cycles of datalogging
int xBeeRate = 15000; //interval in millis between xBee GPS transmissions

void updateGPS() {
  while (gpsSerial.available() > 0) {
    GPS.read();
    if (GPS.newNMEAreceived()) {
      GPS.parse(GPS.lastNMEA());
      break;
    }
  }
  if (millis() - timer1 > logRate) {
    timer1 = millis();
    openDatalog();
    if (GPS.fix) {
      datalog.print(flightTimeStr() + "," + String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + ",");
      datalog.print(String(GPS.altitude * 3.28048) + ",");    //convert meters to feet for datalogging
      datalog.print(String(GPS.month) + "/" + String(GPS.day) + "/" + String(GPS.year) + ",");
      datalog.println(String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",");
    }
    else
      datalog.println(flightTimeStr() + ",No fix");
    closeDatalog();
  }
  if (millis() - timer2 > xBeeRate) {
    timer2 = millis();
    openDatalog();
    String message = String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",";
    message += String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + "," + String(GPS.altitude * 3.28048) + ",";
    if (GPS.fix) message += "Fix";
    else message += "No Fix";
    sendXBee(message);
    closeDatalog();
  }
}

int getGPStime() {    //returns GPS time as seconds since 0:00:00 UTC. Note that comparisons crossing that time will be inaccurate
  return GPS.hour * 3600 + GPS.minute * 60 + GPS.seconds;
}

boolean isBurst() {
  updateGPS();
  float alt1 = GPS.altitude;
  unsigned long t = millis();
  sendXBee("Checking for burst...");
  while (millis() - t < 10000) {
    updateGPS();
  }
  if (alt1 - GPS.altitude > 100) {
    hasBurst = true;
    return true;
  }
  else return false;
}

