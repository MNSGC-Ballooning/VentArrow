float checkAlt;
int lastGPS;


//function to handle both retrieval of data from GPS module and recording it on the SD card
void updateGPS() {
  while (gpsSerial.available() > 0) {
    GPS.read();
  }
  if (GPS.newNMEAreceived()) {
    int lastTime = getGPStime();
    GPS.parse(GPS.lastNMEA());
    if (!firstFix && GPS.fix) {
      GPSstartTime = GPS.hour * 3600 + GPS.minute * 60 + GPS.seconds;
      firstFix = true;
    }
    if (getGPStime() > lastTime && !startup) {
      openDatalog();
      String data = "";
      if (GPS.fix) {
        data += (flightTimeStr() + "," + String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + ",");
        data += (String(GPS.altitude * 3.28048) + ",");    //convert meters to feet for datalogging
        data += (String(GPS.month) + "/" + String(GPS.day) + "/" + String(GPS.year) + ",");
        data += (String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",");
        lastGPS = GPS.hour * 3600 + GPS.minute * 60 + GPS.seconds;
      }
      else
        data += (flightTimeStr() + ",No fix");
      datalogA.println(data);
      datalogB.println(data);
      closeDatalog();
    }
  }
}

int getGPStime() {    //returns time in seconds between last successful fix and initial fix. Used to match with altitude data
  if (!newDay && lastGPS < GPSstartTime) {
    days++;
    newDay = true;
  }
  else if (newDay && lastGPS > GPSstartTime)
    newDay = false;
  return days * 86400 + lastGPS;
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
    BurstCheck* burstCheck = new BurstCheck(10);
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

