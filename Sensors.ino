unsigned long timer = millis();
byte logRate = 1;   //interval in seconds between cycles of datalogging

void updateGPS() {
  while (gpsSerial.available() > 0){
    GPS.read();
    if (GPS.newNMEAreceived()) break;
  }
  GPS.parse(GPS.lastNMEA());
  if (millis() - timer < logRate * 1000) {
    timer = millis();
    openDatalog();
    datalog.print(flightTimeStr() + "," + String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + ",");
    datalog.print(String(GPS.altitude * 3.28048) + ",");    //convert meters to feet for datalogging
    datalog.print(String(GPS.month) + "/" + String(GPS.day) + "/" + String(GPS.year) + ",");
    datalog.print(String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",");
    closeDatalog();
  }
  
}

