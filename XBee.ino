void sendXBee(String out) {
  Serial3.println(xBeeID + ";" + out + "!");
  eventlog.println(flightTimeStr() + "  TX  " + out);
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
    command += inChar;
    if (inChar == '!') {
      complete = true;
      break;
    }
    delay(10);
  }
  if (!complete) return;
  if (command.equals(lastCommand) && (millis() - commandTime < 10000)) return;
  int split = command.indexOf('?');
  if (!(command.substring(0, split)).equals(xBeeID)) return;
  int Com = (command.substring(split + 1, command.length() - 1)).toInt();
  acknowledge();
  openEventlog();
  eventlog.print(flightTimeStr() + "  RX  ");
  if (startup && Com == -1) {   //Initial flight start command
    eventlog.println("Begin Flight  -1");
    flightStart = millis();
    sendXBee("Flight Start");
    startup = false;
    closeEventlog();
    return;
  }
  switch (Com) {
    case -1:
      //Same as flight start command; resets flight clock on future calls
      eventlog.println("Flight Clock Reset  -1");
      flightStart = millis();
      break;
    case 01:
      //Open Vent indefinitely
      eventlog.println("Open Vent  1");
      openVent();
      break;

    case 00:
      //Close vent indefinitely
      eventlog.println("Close Vent  0");
      closeVent();
      break;

    case 10:
      //Poll for vent status
      eventlog.println("Poll Vent Status  10");
      if (analogRead(ventFeed) > 30)
      {
        sendXBee("Vent Open");
      }
      else
      {
        sendXBee("Vent Closed");
      }
      break;

    case 11:
      {
        //Poll for total open time in minutes:seconds
        eventlog.println("Poll Time Open  11");
        unsigned long t = totalOpen;
        if (ventIsOpen)
          t += millis() - openTime;
        t /= 1000;
        String tStr = String(t / 60) + ":";
        t %= 60;
        tStr += String(t / 10) + String(t % 10);
        sendXBee(tStr);
      }
      break;

    case 12: //fix
      {
        //Poll for remaining failsafe time in minutes:seconds
        eventlog.println("Poll failsafe time remaining  12");
        int timeLeft = cutTime - (flightStart / 1000);
        String timeLeftStr = (String(timeLeft / 60) + ":");
        timeLeft %= 60;
        timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
        sendXBee(timeLeftStr);
      }
      break;

    case 03:
      {
        //Cutdown and check cutdown status
        eventlog.println("Initiate Cutdown  3");
        Legolas();
      }

    case 13:
      {
        //Check cutdown status
        if (Com == 13)
          eventlog.println("Poll cutdown  13");
        if (isBurst())
          sendXBee("Cutdown Successful");
        else
          sendXBee("Cutdown Failed");
      }
      break;

    case 04:
      //Just extends the arrow. Pretty much just for GT
      eventlog.println("Extend Arrow  4");
      extendArrow();
      break;

    case 05:
      eventlog.println("Retract Arrow  5");
      retractArrow();
      break;

    /* We are not entirely sure if this is needed 7/26       //HELP ME RYAN YOU'RE MY ONLY HOPE 7/26/16
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      {
      byte n = Com % 20;
      eventlog.println("Poll ascent rate " + String(n));
      int altDif = altitudes[1] - altitudes[0];
      double timeDif = (times[1] - times[0]) / 60000.0;
      double ascent = altDif / timeDif;
      sendXBee(String(ascent) + "ft/min");
      }
      break;
    */

    default:
      if (Com / 10000 == 1) { //Open For Time - five digits starting with 1; 2nd and 3rd are minutes, 4th and 5th are seconds
        eventlog.println("Open for Time  " + String(Com));
        Com -= 10000;
        int t = (Com / 100 * 60) + (Com % 100);
        openForTime(t);
      }
      else if (Com / 100 == 1) { //Add time in minutes to failsafe timer: 3 digits starting with 1; 2nd and third are minutes
        if (Com > 0) {
          eventlog.println("Adding time to failsafe: " + String(Com));
          int addedTime = Com - 100; //"addedTime" is now the amount of minutes to be added
          addedTime *= 60000; //"addedTime" is now converted into milliseconds
          cutTime += addedTime;
        }
        else
        {
          eventlog.println("Removing time from failsafe: " + String(Com));
          int takenTime = Com - 100; //"addedTime" is now the amount of minutes to be subtracted
          takenTime *= 60000; //"addedTime" is now converted into milliseconds
          cutTime -= takenTime;
        }
      }



      /*else if (Com / 1000 > 0 && Com / 1000 < 10){
        byte a = Com /1000;
        eventlog.println("Set autoTime" + String(a) + "  " + String(Com));
        Com %= 1000;
        unsigned long t = Com / 100 * 60000;
        Com %= 100;
        t += Com * 1000;
        if (a == 1)
          autoTime1 = t;
        else if (a == 2)
          autoTime2 = t;
        else if (a == 3)
          autoTime3 = t;
        } */
      else {
        eventlog.println("Unknown command  " + String(Com));
        sendXBee(String(Com) + "Command Not Recognized");
      }
  }//end Switch
  lastCommand = command;
  commandTime = millis();
  closeEventlog();
}
