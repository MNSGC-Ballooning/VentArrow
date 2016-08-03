int AutoVent::rate1() {
  return (alts[1] - alts[0])/(times[1] - times[0]);
}

int AutoVent::rate2() {
  return (alts[3] - alts[2])/(times[3] - times[2]);
}

void AutoVent::autoCheck() {
  switch (reached) {
    case 0:
      if (GPS.altitude > (targetAlt - 5000)) {
        alts[0] = GPS.altitude;
        times[0] = getGPStime();
        reached++;
      }
      break;
    case 1:
      if (GPS.altitude > targetAlt) {
        alts[1] = GPS.altitude;
        times[1] = getGPStime();
        openEventlog();
        openForTime(ventTime);
        closeEventlog();
        alts[2] = GPS.altitude;
        times[2] = getGPStime();
        reached++;
      }
      break;
    case 2:
      if (getGPStime() - times[2] > 60000) {
        alts[3] = GPS.altitude;
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
  for (AutoVent autovent : autos) {
    autovent.autoCheck();
  }
}

