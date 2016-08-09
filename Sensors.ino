unsigned long timer = millis();
int logRate = 1000;   //interval in seconds between cycles of datalogging

void updateGPS() {
  while (gpsSerial.available() > 0) {
    GPS.read();
    if (GPS.newNMEAreceived()) {
      GPS.parse(GPS.lastNMEA());
      break;
    }
  }
  if (millis() - timer > logRate) {
    timer = millis();
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
}

int getGPStime() {    //returns GPS time as seconds since 0:00:00 UTC. Note that comparisons crossing that time will be inaccurate
  return GPS.hour * 3600 + GPS.minute * 60 + GPS.seconds;
}

boolean isBurst() {
  updateGPS();
  float alt1 = GPS.altitude;
  unsigned long t = millis();
  byte counter = 0;
  while (millis() - t < 10000) {
    updateGPS();
    if ((millis() - t) / 1000 > counter) {
      counter++;
      sendXBee("Burst check in " + String(10 - counter));
    }
  }
  updateGPS();
  if (alt1 - GPS.altitude > 300) {
    hasBurst = true;
    return true;
  }
  else return false;
}

