void sendXBee(String out) {
  Serial3.println(xBeeID + ";" + out + "!");
  openEventlog();
  eventlog.println(flightTimeStr() + "  TX  " + out);
  closeEventlog();
}

void logCommand(String com, String command) {
  openEventlog();
  eventlog.println(flightTimeStr() + "  RX  " + com + "  " + command);
  closeEventlog();
}

void acknowledge() {
  Serial3.println(xBeeID + "\n");
}

String lastCommand = "";
unsigned long commandTime = 0;

void xBeeCommand() {
  boolean complete = false;
  String command = "";
  char inChar;
  while (Serial3.available() > 0) {
    inChar = (char)Serial3.read();
    if (inChar != ' ') {
      command += inChar;
      if (inChar == '!') {
        complete = true;
        break;
      }
    }
    delay(10);
  }
  if (!complete) return;
  if (command.equals(lastCommand) && (millis() - commandTime < 30000)) return;
  int split = command.indexOf('?');
  if (!(command.substring(0, split)).equals(xBeeID)) return;
  lastCommand = command;
  String Com = command.substring(split + 1, command.length() - 1);
  acknowledge();
  commandTime = millis();
  if (startup && Com.equals("FS")) {   //Initial flight start command
    logCommand(Com, "Begin Flight");
    flightStart = millis();
    sendXBee("Flight Start");
    startup = false;
    return;
  }
  else if (Com.equals("FS")) {
    //Same as flight start command; resets flight clock on future calls
    logCommand(Com, "Flight Clock Reset");
    flightStart = millis();
    }

  else if (Com.equals("VF")) {
    //debugging request for analogRead()
    logCommand(Com, "Get ventFeed");
    sendXBee(String(analogRead(ventFeed)));
  }

  
  else if (Com.equals("OV")) {
    //Open Vent indefinitely
    logCommand(Com, "Open Vent");
    openVent();
  }
  
  else if (Com.equals("CV")) {
    //Close vent indefinitely
    logCommand(Com, "Close Vent");
    closeVent();
  }

  else if (Com.equals("CAL")) {
    //Calibrate vent
    logCommand(Com, "Calibrate Vent");
    calibrateVent();
    sendXBee("Calibration complete");
  }
    
  else if (Com.equals("EXT")) {
    //Just extends the arrow. Pretty much just for GT
    logCommand(Com, "Extend Arrow");
    extendArrow();
  }

  else if (Com.equals("RET")) {
    //Retracts arrow. Meant for Ground Test
    logCommand(Com, "Retract Arrow");
    retractArrow();
  }

  else if (Com.equals("VS")) {
    //Poll for vent status
    logCommand(Com, "Poll Vent Status");
    if (analogRead(ventFeed) > ventMax - 5)
      sendXBee("Vent Open");
    else if (analogRead(ventFeed) < ventMin + 8)
      sendXBee("Vent Closed");
    else
      sendXBee("Vent " + String(ventPercent()) + "% open");
  }

  else if (Com.equals("TO")) {
    //Poll for total open time in minutes:seconds
    logCommand(Com, "Poll Time Open");
    unsigned long t = totalOpen;
    if (ventIsOpen)
      t += millis() - openTime;
    t /= 1000;
    String tStr = String(t / 60) + ":";
    t %= 60;
    tStr += String(t / 10) + String(t % 10);
    sendXBee(tStr);
  }

  else if (Com.equals("TL")) {
    //Poll for remaining failsafe time in minutes:seconds
    logCommand(Com, "Poll Failsafe Time Remaining");
    int timeLeft = cutTime * 60 - (flightTime() / 1000);
    String timeLeftStr = (String(timeLeft / 60) + ":");
    timeLeft %= 60;
    timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
    sendXBee(timeLeftStr);
  }

  else if (Com.equals("GPS")) {
    //Poll most recent GPS data
    logCommand(Com, "Request GPS data");
    timer = 0;   //forces a GPS message send on next updateGPS() call
    updateGPS();
  }

  else if (Com.equals("LS")) {
    //Get a list of all current AutoVent times and altitudes. Number at front indicates digit to use
    logCommand(Com, "Get list of AutoVents");
    for (int i = 0; i < sizeof(autos) / sizeof(autos[1]); i++) {
      sendXBee(String(i+1) + ": " + String(autos[i].targetAlt) + "ft, " + String(autos[i].ventTime) + "sec");
    }
  }

  else if (Com.equals("AR")) {
    //Poll most recent ascent rate
    logCommand(Com, "Poll ascent rate");
    if (lastRate == 0) {
      updateGPS();
      int alt1 = GPS.altitude, time1 = getGPStime();
      unsigned long t = millis();
      while (millis() - t < 10000) {
        updateGPS();
        xBeeCommand();
      }
      updateGPS();
      lastRate = (GPS.altitude - alt1) / (getGPStime() - time1);
    }
    sendXBee(String(lastRate) + "ft/s");
  }

  else if (Com.equals("ELF")) {
    //Cutdown and check cutdown status
    logCommand(Com, "Initiate Cutdown");
    Legolas();
    if (isBurst())
      sendXBee("Cutdown Successful");
    else
      sendXBee("Cutdown Failed");
  }

  else if (Com.equals("IBS")) {
    //Check cutdown status
    if (Com == 43)
      logCommand(Com, "Poll Cutdown");
    if (isBurst())
      sendXBee("Cutdown Successful");
    else
      sendXBee("Cutdown Failed");
  }

  else if (Com.equals("HBS")) {
    //Set hasBurst to true; used to prevent automatic cutdown if burst has occured but not been detected
    logCommand(Com, "Set hasBurst to true");
    hasBurst = true;
  }

  else if (Com.equals("NBS")) {
    //Set hasBurst to false; unlikely to be used, but will reenable automatic cutdowns if needed
    logCommand(Com, "Set hasBurst to false");
    hasBurst = false;
  }

  else if ((Com.substring(0,2)).equals("OT")) {
    //Open For Time - minutes and seconds follow OT prefix
    logCommand(Com, "Open for Time");
    int t = (Com.substring(2, Com.length())).toInt();
    t = (t / 100 * 60) + (t % 100);
    openForTime(t);
  }
      
  else if ((Com.substring(0,2)).equals("AT")) {
    //Add time in minutes to failsafe
    logCommand(Com, "Add time to failsafe");
    int addedTime = (Com.substring(2, Com.length())).toInt();
    cutTime += addedTime;
    int timeLeft = cutTime * 60 - (flightTime() / 1000);
    String timeLeftStr = (String(timeLeft / 60) + ":");
    timeLeft %= 60;
    timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
    sendXBee(timeLeftStr);
  }
  
  else if ((Com.substring(0,2)).equals("RT")) {
    //Remove time in minutes from failsafe
    logCommand(Com, "Remove time from failsafe");
    int takenTime = (Com.substring(2, Com.length())).toInt();
    cutTime -= takenTime;
    int timeLeft = cutTime * 60 - (flightTime() / 1000);
    String timeLeftStr = (String(timeLeft / 60) + ":");
    timeLeft %= 60;
    timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
    sendXBee(timeLeftStr);
  }

  else if ((Com.substring(0,2)).equals("AC")) {
    //set new auto cut altitude
    logCommand(Com, "Set Auto Cut Altitude");
    int alt = (Com.substring(2, Com.length())).toInt();
    cutAlt = alt * 1000;
    sendXBee("New auto cut alt: " + String(cutAlt) + "ft");
  }

  else if ((Com.substring(0,2)).equals("AL")) {
    //Set a new duration for an AutoVent
    logCommand(Com, "Set AutoVent time");
    int autovent = (Com.substring(2,3)).toInt();
    int t = (Com.substring(3, Com.length())).toInt();
    autos[autovent - 1].ventTime = (t / 100 * 60) + (t % 100);
  }
      
  else if ((Com.substring(0,2)).equals("AA")) {
    //Set a new target altitude for an AutoVent
    logCommand(Com, "Set AutoVent target altitude");
    int autovent = (Com.substring(2,3)).toInt();
    int alt = (Com.substring(3, Com.length())).toInt();
    autos[autovent - 1].targetAlt = alt * 1000;
  }
      
  else {
    //If no recognizable command was received, inform ground station
    logCommand(Com, "Unknown Command");
    sendXBee(String(Com) + "  Command Not Recognized");
  }
}
