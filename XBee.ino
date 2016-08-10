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
  if (command.equals(lastCommand) && (millis() - commandTime < 30000)) return;
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
    
    case 0:
      //Close vent indefinitely
      eventlog.println("Close Vent  0");
      closeVent();
      break;
    
    case 1:
      //Open Vent indefinitely
      eventlog.println("Open Vent  1");
      openVent();
      break;

    case 4:
      //Just extends the arrow. Pretty much just for GT
      eventlog.println("Extend Arrow  4");
      extendArrow();
      break;

    case 5:
      eventlog.println("Retract Arrow  5");
      retractArrow();
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

    case 12:
      {
        //Poll for remaining failsafe time in minutes:seconds
        eventlog.println("Poll failsafe time remaining  12");
        int timeLeft = cutTime * 60 - (flightTime() / 1000);
        String timeLeftStr = (String(timeLeft / 60) + ":");
        timeLeft %= 60;
        timeLeftStr += (String(timeLeft / 10) + String(timeLeft % 10));
        sendXBee(timeLeftStr);
      }
      break;

    case 42:  //"Not bad for a pointy-eared elvish princeling..."
      {
        //Cutdown and check cutdown status
        eventlog.println("Initiate Cutdown  42");
        Legolas();
      }

    case 43:  //"...But I myself am sitting pretty on fourty-three!"
      {
        //Check cutdown status
        if (Com == 43)
          eventlog.println("Poll cutdown  43");
        if (isBurst())
          sendXBee("Cutdown Successful");
        else
          sendXBee("Cutdown Failed");
      }
      break;

    default:
      if (Com / 10000 == 1) { //Open For Time - five digits starting with 1; 2nd and 3rd are minutes, 4th and 5th are seconds
        eventlog.println("Open for Time  " + String(Com));
        Com -= 10000;
        int t = (Com / 100 * 60) + (Com % 100);
        openForTime(t);
      }
      else if (Com / 100 == 1) { //Add time in minutes to failsafe timer: 3 digits starting with 1; 2nd and third are minutes
          eventlog.println("Adding time to failsafe: " + String(Com));
          int addedTime = Com - 100; //"addedTime" is now the amount of minutes to be added
          cutTime += addedTime;
      }
      else if (Com / 100 == -1) {
        eventlog.println("Removing time from failsafe: " + String(Com));
        int takenTime = Com + 100; //"addedTime" is now the amount of minutes to be subtracted
        cutTime += takenTime;
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
