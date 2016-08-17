void sendXBee(String out) {
  Serial3.println(xBeeID + ";" + out + "!");
  openEventlog();
  eventlog.println(flightTimeStr() + "  TX  " + out);
  closeEventlog();
}

void logCommand(int com, String command) {
  openEventlog();
  eventlog.println(flightTimeStr() + "  RX  " + String(com) + "  " + command);
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
  int Com = (command.substring(split + 1, command.length() - 1)).toInt();
  acknowledge();
  lastCommand = command;
  commandTime = millis();
  if (startup && Com == -1) {   //Initial flight start command
    logCommand(Com, "Begin Flight");
    flightStart = millis();
    sendXBee("Flight Start");
    startup = false;
    return;
  }
  switch (Com) {
    case -1:
      //Same as flight start command; resets flight clock on future calls
      logCommand(Com, "Flight Clock Reset");
      flightStart = millis();
      break;

    case -10:
      //debugging request for analogRead()
      logCommand(Com, "Get ventFeed");
      sendXBee(String(analogRead(ventFeed)));
      break;
    
    case 1:
      //Close vent indefinitely
      logCommand(Com, "Close Vent");
      closeVent();
      break;
    
    case 2:
      //Open Vent indefinitely
      logCommand(Com, "Open Vent");
      openVent();
      break;

    case 3:
      //Calibrate vent
      logCommand(Com, "Calibrate Vent");
      calibrateVent();
      sendXBee("Calibration complete");
      break;
    
    case 4:
      //Just extends the arrow. Pretty much just for GT
      logCommand(Com, "Extend Arrow");
      extendArrow();
      break;

    case 5:
      //Retracts arrow. Meant for Ground Test
      logCommand(Com, "Retract Arrow");
      retractArrow();
      break;

    case 10:
      //Poll for vent status
      logCommand(Com, "Poll Vent Status");
      if (analogRead(ventFeed) > ventMax - 5)
        sendXBee("Vent Open");
      else if (analogRead(ventFeed) < ventMin + 8)
        sendXBee("Vent Closed");
      else
        sendXBee("Vent " + String(ventPercent()) + "% open");
      break;

    case 11: {
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
      } break;

    case 12: {
        //Poll for remaining failsafe time in minutes:seconds
        logCommand(Com, "Poll Failsafe Time Remaining");
        int timeLeft = cutTime * 60 - (flightTime() / 1000);
        String timeLeftStr = (String(timeLeft / 60) + ":");
        timeLeft %= 60;
        timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
        sendXBee(timeLeftStr);
      } break;

    case 13:
      //Poll most recent GPS data
      logCommand(Com, "Request GPS data");
      timer = 0;   //forces a GPS message send on next updateGPS() call
      updateGPS();
      break;

    case 20:
      //Get a list of all current AutoVent times and altitudes. Number at front indicates digit to use
      logCommand(Com, "Get list of AutoVents");
      for (int i = 0; i < sizeof(autos) / sizeof(autos[1]); i++) {
        sendXBee(String(i+1) + ": " + String(autos[i].targetAlt) + "ft, " + String(autos[i].ventTime) + "sec");
      }
      break;

    case 21:
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
      break;

    case 42:  //"Not bad for a pointy-eared elvish princeling..."
      //Cutdown and check cutdown status
      logCommand(Com, "Initiate Cutdown);
      Legolas();

    case 43:  //"...But I myself am sitting pretty on fourty-three!"
      //Check cutdown status
      if (Com == 43)
        logCommand(Com, "Poll Cutdown");
      if (isBurst())
        sendXBee("Cutdown Successful");
      else
        sendXBee("Cutdown Failed");
      break;

    case 44:
      //Set hasBurst to true; used to prevent automatic cutdown if burst has occured but not been detected
      logCommand(Com, "Set hasBurst to true");
      hasBurst = true;
      break;

    default:
      if (Com / 10000 == 1) { //Open For Time - five digits starting with 1; 2nd and 3rd are minutes, 4th and 5th are seconds
        logCommand(Com, "Open for Time");
        Com -= 10000;
        int t = (Com / 100 * 60) + (Com % 100);
        openForTime(t);
      }
      
      else if (Com / 100 == 1) { //Add time in minutes to failsafe timer: 3 digits starting with 1; 2nd and third are minutes
          logCommand(Com, "Add time to failsafe");
          int addedTime = Com - 100; //"addedTime" is now the amount of minutes to be added
          cutTime += addedTime;
      }
      else if (Com / 100 == -1) {
        logCommand(Com, "Remove time from failsafe");
        int takenTime = Com + 100; //"addedTime" is now the amount of minutes to be subtracted
        cutTime += takenTime;
      }

      else if (Com / 1000 == 1) {
        //set new auto cut altitude
        logCommand(Com, "Set Auto Cut Altitude");
        Com -= 1000;
        cutAlt = Com * 1000;
      }

      else if (Com / 10000 == 2) { //Set a new duration for an AutoVent
        logCommand(Com, "Set AutoVent time");
        Com -= 20000;
        byte autovent = Com / 1000;
        Com %= 1000;
        autos[autovent - 1].ventTime = (Com / 100 * 60) + (Com % 100);
      }
      
      else if (Com / 10000 == 3) { //Set a new target altitude for an AutoVent
        logCommand(Com, "Set AutoVent target altitude");
        Com -= 30000;
        byte autovent = Com / 1000;
        Com %= 1000;
        autos[autovent - 1].targetAlt = Com * 1000;
      }
      
      else {
        logCommand(Com, "Unknown Command");
        sendXBee(String(Com) + "Command Not Recognized");
      }
  }//end Switch
}
