unsigned long timer = millis();
int xBeeRate = 15000; //interval in millis between xBee GPS transmissions
float checkAlt;


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
      String data = "";
      if (GPS.fix) {
        data += (flightTimeStr() + "," + String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + ",");
        data += (String(GPS.altitude * 3.28048) + ",");    //convert meters to feet for datalogging
        data += (String(GPS.month) + "/" + String(GPS.day) + "/" + String(GPS.year) + ",");
        data += (String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",");
      }
      else
        data += (flightTimeStr() + ",No fix");
      datalogA.println(data);
      datalogB.println(data);
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
void startBurstCheck() {
  sendXBee("Checking for burst...");
  if (!GPS.fix) {
    sendXBee("No fix, burst unknown");
    checkBurst = false;
    return;
  }
  else {
    checkAlt = GPS.altitude;
    Action burstCheck ("burstCheck", 10);
    actions.push_back(burstCheck);
    checkBurst = true;
  }
}

void burstCheck() {
  if (!GPS.fix)
    sendXBee("No fix, burst unknown");
  else if (checkAlt - GPS.altitude > 100) {
    hasBurst = true;
    sendXBee("Burst detected");
  }
  else
    sendXBee("No burst detected");
  checkBurst = false;
}

