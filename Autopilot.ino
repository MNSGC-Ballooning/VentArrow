int gpsAlt;
int lastRate = 0;

int AutoVent::rate1() {
  lastRate = (alts[1] - alts[0])/(times[1] - times[0]);
  return lastRate;
}

int AutoVent::rate2() {
  lastRate = (alts[3] - alts[2])/(times[3] - times[2]);
  return lastRate;
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
        sendXBee("Ascent Rate: " + String(rate1()));
        sendXBee("Reached " + String(targetAlt) + "ft, venting for " + String(ventTime) + "s");
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
        sendXBee("Ascent Rate: " + String(rate2()));
        closeEventlog();
        reached++;
      }
      break;
  }
}

AutoVent::AutoVent(int alt, int vent) {
  targetAlt = alt * 1000;
  ventTime = vent;
  reached = 0;
}

void autopilot() {
  gpsAlt = GPS.altitude * 3.28048;
  for (int i = 0; i < sizeof(autos) / sizeof(autos[0]); i++) {
    autos[i].autoCheck();
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

