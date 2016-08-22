unsigned long timer = millis();
int xBeeRate = 15000; //interval in millis between xBee GPS transmissions

//function to handle both retrieval of data from GPS module and recording it on the SD card
void updateGPS() {
  while (gpsSerial.available() > 0) {
    GPS.read();
  }
  if (startup) return;    //Don't fill up file with pre-flight data
  if (GPS.newNMEAreceived()) {
    int lastTime = getGPStime();
    GPS.parse(GPS.lastNMEA());
    if (getGPStime() > lastTime) {
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
  if (millis() - timer > xBeeRate) { //every 15s, send current GPS data via xBee
    timer = millis();
    String message = String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",";
    message += String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + "," + String(GPS.altitude * 3.28048) + ",";
    if (GPS.fix) message += "Fix";
    else message += "No Fix";
    sendXBee(message);
  }
}

int getGPStime() {    //returns GPS time as seconds since 0:00:00 UTC. Note that comparisons crossing that time will be inaccurate
  return GPS.hour * 3600 + GPS.minute * 60 + GPS.seconds;
}

//Attempts to detect burst by looking at GPS altitude change over 10s. Sends xBee message if no fix
boolean isBurst() {
  sendXBee("Checking for burst...");
  updateGPS();
  if (!GPS.fix) {
    sendXBee("No fix, burst unknown");
    return false;
  }
  float alt1 = GPS.altitude;
  unsigned long t = millis();
  while (millis() - t < 10000) {
    updateGPS();
  }
  if (alt1 - GPS.altitude > 100 && GPS.fix) {
    hasBurst = true;
    return true;
  }
  else if (!GPS.fix)
    sendXBee("No fix, burst unknown");
  return false;
}

