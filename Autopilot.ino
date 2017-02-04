//time between xBee status updates
int beaconRate = 15;
//global variables needed across calls to autopilot()
int gpsAlt;
int gpsTime;
int ascentRate = 0;
boolean altCut = false;
boolean timeCut = false;


//two rate calculation functions used solely for convenience later
int AutoVent::rate1() {
  return (alts[1] - alts[0])/(times[1] - times[0]);
}

int AutoVent::rate2() {
  return (alts[3] - alts[2])/(times[3] - times[2]);
}

void AutoVent::newRate() {
  alts[2] = GPS.altitude * 3.28048;
  times[2] = getGPStime();
  closeVent(); //An extra safeguard to make sure vent closes properly
  gpsAlt = GPS.altitude * 3.28048;
  reached++;
}

void checkNewRate() {
  for (int i = 0; i < sizeof(autos) / sizeof(autos[0]); i++) { //Check all AutoVents
    if (autos[i].autoCheck())
      autos[i].newRate();
  }
}

//Primary AutoVent function. Checks current altitude, and if that altitude reaches various thresholds relative
//to the target altitude, takes various actions.
boolean AutoVent::autoCheck() {
  switch (reached) { //Ensures each phase of the AutoVent must be carried out in sequence
    case 0:          //First, save an altitude and time well before venting begins
      if (gpsAlt > (targetAlt - 5000)) {
        alts[0] = gpsAlt;
        times[0] = gpsTime;
        reached++;
      }
      break;
    case 1:         //Once target altitude is reached, save another alt and time, then open vent for set time
      if (gpsAlt > targetAlt) {
        alts[1] = gpsAlt;
        times[1] = gpsTime;
        sendXBee("Ascent Rate: " + String(rate1())); //Report initial ascent rate
        sendXBee("Reached " + String(targetAlt) + "ft, venting for " + String(ventTime) + "s");
        openForTime(ventingTime);
        CheckRateAction* checkRate = new CheckRateAction(ventingTime + 2 * ventTime);
        actions.push_back(checkRate);
        reached++;
      }
      break;
    case 2:
      return true;
      break;
    case 3:         //Well after venting has stopped, save another altitude and time and report new ascent rate
      if (getGPStime() - times[2] > 5 * 60000) {
        alts[3] = gpsAlt;
        times[3] = gpsTime;
        sendXBee("Ascent Rate: " + String(rate2()));
        reached++;
      }
      break;
  }
  return false;
}

//Constructor. Requires altitude in 1000s of feet, and time to vent in seconds
AutoVent::AutoVent(int alt, int vent) {
  targetAlt = alt * 1000;
  ventingTime = vent;
  reached = 0;
}

//Primary autopilot function. Checks to see if altitude or time has triggered various events
void autopilot() {
  if ((GPS.altitude - gpsAlt)/(getGPStime() - gpsTime) < 5) { //Throws out bad data that can fool system
    gpsAlt = GPS.altitude * 3.28048;
    gpsTime = getGPStime();
  }
  if (!checkRate) {
    Beacon* beacon = new Beacon(beaconRate);
    actions.push_back(beacon);
    checkRate = true;
  }
  for (int i = 0; i < sizeof(autos) / sizeof(autos[0]); i++) { //Check all AutoVents
    autos[i].autoCheck();
  }
  
  if (!hasBurst) {
    if (gpsAlt > cutAlt && !altCut) { //Check if automatic cut altitude has been reached
      logAction("Auto Cut - Altitude " + String(gpsAlt));
      sendXBee("Auto Cut - Altitude");
      if (!checkBurst) {
        Legolas();
        StartBurstCheck* startBurstCheck = new StartBurstCheck(10 + 2 * arrowTime);
        actions.push_back(startBurstCheck);
        checkBurst = true;
      }
      altCut = true;
    }
    else if (altCut && GPS.fix && !checkBurst) {
      sendXBee("Fix acquired, attempting altitude cut");
      Legolas();
      StartBurstCheck* startBurstCheck = new StartBurstCheck(10 + 2 * arrowTime);
      actions.push_back(startBurstCheck);
      checkBurst = true;
    }

    if (flightTime() > cutTime * 60000 && !timeCut) {  //Check if automatic cut time has occured
      logAction("Auto Cut - Time " + String(cutTime) + " minutes");
      sendXBee("Auto Cut - Time");
      if (!checkBurst) {
        Legolas();
        StartBurstCheck* startBurstCheck = new StartBurstCheck(10 + 2 * arrowTime);
        actions.push_back(startBurstCheck);
        checkBurst = true;
      }
      timeCut = true;
    }
    else if (timeCut && !checkBurst) {
      sendXBee("Reattempting time cut");
      Legolas();
      StartBurstCheck* startBurstCheck = new StartBurstCheck(10 + 2 * arrowTime);
      actions.push_back(startBurstCheck);
      checkBurst = true;
    }
  }
}
