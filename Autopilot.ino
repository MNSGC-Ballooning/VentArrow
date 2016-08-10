int gpsAlt;

int AutoVent::rate1() {
  return (alts[1] - alts[0])/(times[1] - times[0]);
}

int AutoVent::rate2() {
  return (alts[3] - alts[2])/(times[3] - times[2]);
}

void AutoVent::autoCheck() {
  switch (reached) {
    case 0:
      if (gpsAlt > (targetAlt - 5000)) {
        alts[0] = gpsAlt;
        times[0] = getGPStime();
        reached++;
      }
      break;
    case 1:
      if (gpsAlt > targetAlt) {
        alts[1] = gpsAlt;
        times[1] = getGPStime();
        openEventlog();
        openForTime(ventTime);
        closeEventlog();
        alts[2] = gpsAlt;
        times[2] = getGPStime();
        reached++;
      }
      break;
    case 2:
      if (getGPStime() - times[2] > 5 * 60000) {
        alts[3] = gpsAlt;
        times[3] = getGPStime();
        openEventlog();
        eventlog.print(flightTimeStr() + "  AC  AutoVent for " + String(ventTime) + " seconds at ");
        eventlog.println(String(targetAlt) + " ft");
        eventlog.println("             Ascent rate lowered from " + String(rate1()) + " to " + String(rate2()) + " ft/s");
        closeEventlog();
        reached++;
      }
      break;
  }
}

AutoVent::AutoVent(int alt, unsigned long vent) {
  targetAlt = alt;
  ventTime = vent;
}

void autopilot() {
  gpsAlt = GPS.altitude * 3.28048;
  for (AutoVent autovent : autos) {
    autovent.autoCheck();
  }
  
  if (!hasBurst && gpsAlt > cutAlt) {
    openEventlog();
    eventlog.println(flightTimeStr() + "  AC  Auto Cut - Altitude " + String(gpsAlt));
    sendXBee("Auto Cut - Altitude");
    Legolas();
    while (!isBurst()){
      sendXBee("Cutdown failed, reattempting now");
      Legolas();
    }
    sendXBee("Cutdown Successful");
    closeEventlog();
  }

  if (!hasBurst && flightTime() > cutTime * 60000) {
    openEventlog();
    eventlog.println(flightTimeStr() + "  AC  Auto Cut - Time " + String(cutTime) + " minutes");
    sendXBee("Auto Cut - Time");
    Legolas();
    while (!isBurst()){
      sendXBee("Cutdown failed, reattempting now");
      Legolas();
    }
    sendXBee("Cutdown Successful");
    closeEventlog();
  }
}

