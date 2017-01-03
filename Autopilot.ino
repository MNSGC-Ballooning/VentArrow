//global variables needed across calls to autopilot()
int gpsAlt;
int lastAlt;
int lastRate = 0;
boolean altCut = false;

//two rate calculation functions used solely for convenience later
int AutoVent::rate1() {
  lastRate = (alts[1] - alts[0])/(times[1] - times[0]);
  return lastRate;
}

int AutoVent::rate2() {
  lastRate = (alts[3] - alts[2])/(times[3] - times[2]);
  return lastRate;
}

//Primary AutoVent function. Checks current altitude, and if that altitude reaches various thresholds relative
//to the target altitude, takes various actions.
void AutoVent::autoCheck() {
  switch (reached) { //Ensures each phase of the AutoVent must be carried out in sequence
    case 0:          //First, save an altitude and time well before venting begins
      if (gpsAlt > (targetAlt - 5000)) {
        alts[0] = gpsAlt;
        times[0] = getGPStime();
        reached++;
      }
      break;
    case 1:         //Once target altitude is reached, save another alt and time, then open vent for set time
      if (gpsAlt > targetAlt) {
        alts[1] = gpsAlt;
        times[1] = getGPStime();
        sendXBee("Ascent Rate: " + String(rate1())); //Report initial ascent rate
        sendXBee("Reached " + String(targetAlt) + "ft, venting for " + String(ventTime) + "s");
        openForTime(ventTime);
        alts[2] = GPS.altitude * 3.28048;
        times[2] = getGPStime();    //save another set of altitude and time
        closeVent(); //An extra safeguard to make sure vent closes properly
        gpsAlt = GPS.altitude * 3.28048;
        reached++;
      }
      break;
    case 2:         //Well after venting has stopped, save another altitude and time and report new ascent rate
      if (getGPStime() - times[2] > 5 * 60000) {
        alts[3] = gpsAlt;
        times[3] = getGPStime();
        sendXBee("Ascent Rate: " + String(rate2()));
        reached++;
      }
      break;
  }
}

//Constructor. Requires altitude in 1000s of feet, and time to vent in seconds
AutoVent::AutoVent(int alt, int vent) {
  targetAlt = alt * 1000;
  ventTime = vent;
  reached = 0;
}

//Primary autopilot function. Checks to see if altitude or time has triggered various events
void autopilot() {
  lastAlt = gpsAlt;
  gpsAlt = GPS.altitude * 3.28048;
  if (gpsAlt - lastAlt > 100) return; //Throws out outlier data that can fool system into an early vent or cut
  for (int i = 0; i < sizeof(autos) / sizeof(autos[0]); i++) { //Check all AutoVents
    autos[i].autoCheck();
  }
  
  if (!hasBurst && gpsAlt > cutAlt && !altCut) { //Check if automatic cut altitude has been reached
    logAction("Auto Cut - Altitude " + String(gpsAlt));
    sendXBee("Auto Cut - Altitude");
    Legolas();
    while (!hasBurst && GPS.fix == true){ //reattempt cutdown if GPS lock exists and cutdown not detected
      sendXBee("Cutdown failed, reattempting now");
      Legolas();
      isBurst();
    }
    if (hasBurst)
      sendXBee("Cutdown Successful");
    else        //Do not continue to attempt cutdown if no GPS lock
      sendXBee("No fix, confirmation of burst required");
    altCut = true;
  }

  if (!hasBurst && flightTime() > cutTime * 60000) {    //Check if automatic cut time has occured
    logAction("Auto Cut - Time " + String(cutTime) + " minutes");
    sendXBee("Auto Cut - Time");
    Legolas();
    while (!hasBurst && GPS.fix){
      sendXBee("Cutdown failed, reattempting now");
      Legolas();
      isBurst();
    }
    if (hasBurst)
      sendXBee("Cutdown Successful");
    else {          //Keep attempting cutdown, even if no fix.
      while (!hasBurst) {
        isBurst();
        sendXBee("No fix, reattempting cutdown until burst detected or confirmation sent");
        extendArrow();
        Legolas();
      }  
    }
  retractArrow();
  }
}
