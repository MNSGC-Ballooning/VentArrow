void OpenVentAction::doAction() {openVent();}
boolean OpenVentAction::isRemovedOn(String type) {
  if (type.equals("vent")) return true;
  else return false;
}


void CloseVentAction::doAction() {closeVent();}
boolean CloseVentAction::isRemovedOn(String type) {
  if (type.equals("vent")) return true;
  else return false;
}

void StopVentAction::doAction() {stopVent();}
boolean StopVentAction::isRemovedOn(String type) {
  if (type.equals("vent")) return true;
  else return false;
}

void ExtendArrowAction::doAction() {extendArrow();}
boolean ExtendArrowAction::isRemovedOn(String type) {
  if (type.equals("arrow")) return true;
  else return false;
}

void RetractArrowAction::doAction() {retractArrow();}
boolean RetractArrowAction::isRemovedOn(String type) {
  if (type.equals("arrow")) return true;
  else return false;
}

void StopArrowAction::doAction() {stopArrow();}
boolean StopArrowAction::isRemovedOn(String type) {
  if (type.equals("arrow")) return true;
  else return false;
}

void CheckRateAction::doAction() {checkNewRate();}

void StartBurstCheck::doAction() {startBurstCheck();}

void BurstCheck::doAction() {burstCheck();}

Beacon::Beacon(int t) : Action(t) {
  GPSalt = GPS.altitude;
  GPStime = getGPStime();
}
void Beacon::doAction() {
  int newAlt = GPS.altitude;
  int newTime = getGPStime();
  ascentRate = (newAlt - GPSalt)/(newTime - GPStime)*3.28048;
  String message = String(GPS.hour) + ":" + String(GPS.minute) + ":" + String(GPS.seconds) + ",";
  message += String(GPS.latitudeDegrees) + "," + String(GPS.longitudeDegrees) + "," + String(GPS.altitude * 3.28048) + ",";
  if (GPS.fix) message += "Fix";
  else message += "No Fix";
  sendXBee(message);
  sendXBee(String(readPressure()) + " mbar, " + String(sensors.getTempC(thermometer)) + " C");
  sendXBee("Ascent rate: " + String(ascentRate) + " ft/s");
  checkRate = false;
}

void checkActions() {
  for (vector<Action*>::iterator it = actions.begin(); it < actions.end(); it++) {
    if ((*it)->checkTimer()) {
      (*it)->doAction();
      delete *it;
      it--;
      actions.erase(it + 1);
    }
  }
}

