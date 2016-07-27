unsigned long timer = millis();
byte logRate = 1;   //interval in seconds between cycles of datalogging

void updateGPS() {
  while (gpsSerial.available() > 0) {
    GPS.read();
    if (GPS.newNMEAreceived()) {
      GPS.parse(GPS.lastNMEA());
      break;
    }
  }
  if (millis() - timer > logRate * 1000) {
    timer = millis();
    if (GPS.fix) {
      openDatalog();
      datalog.print(flightTimeStr() + "," + String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + ",");
      datalog.print(String(GPS.altitude * 3.28048) + ",");    //convert meters to feet for datalogging
      datalog.print(String(GPS.month) + "/" + String(GPS.day) + "/" + String(GPS.year) + ",");
      datalog.println(String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",");
      closeDatalog();
    }
    else
      datalog.println(flightTimeStr() + ",No fix");
  }

}

