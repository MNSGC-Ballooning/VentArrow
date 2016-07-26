void sendXBee(String out) {
  Serial3.println(out);
  eventlog.println(flightTimeStr() + "  TX  " + out);
}

void acknowledge() {
  xBee.println(xBeeID + "\n");
}

String lastCommand = "";
unsigned long commandTime = 0;

void xBeeCommand(){
  boolean complete = false;
  String command = "";
  char inChar;
  while (xBee.available() > 0) {
    inChar = (char)xBee.read();
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
  lastCommand = command;
  commandTime = millis();
  int Com = (command.substring(split + 1, command.length() - 1)).toInt();
  acknowledge();
  openEventlog();
  eventlog.print(flightTimeStr() + "  RX  ");
  if (startup && Com == -1) {   //Initial flight start command
    eventlog.println("Begin Flight  -1");
    flightStart = millis();
    sendXBee("Flight Start");
    startup = false;
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
        eventlog.println("Open Vent  01");
        openVent(); //NEEDS TO BE WRITTEN as of 7/26
        break;
    
     case 00:
        //Close vent indefinitely
        eventlog.println("Close Vent  00");
        closeVent()  //NEEDS TO BE WRITTEN as of 7/26
        break;
        
     case 10:                                   //HELP ME, RYAN! YOU'RE MY ONLY HOPE! 7/26/16
        //Poll for vent status                  //(This currently doesn't work, is based on old hardware)
        eventlog.println("Poll Vent Status  10");
        if (Vent.read() > 11)
        {
          sendXBee("Vent Open");
        }
        else
        {
          sendXBee("Vent Closed");
        }
        break;
      
      case 11:                                //HELP ME, RYAN! YOU'RE MY ONLY HOPE! 7/26/16
        {
          //Poll for total open time in minutes
          eventlog.println("Poll Time Open  11");
          int TimeOpen = TotalOpen / 60000;
          sendXBee(String(TimeOpen));
        }
        break;
        
       case 02:
        //Add some amount of time to failsafe countdown
        //NEEDS TO BE WRITTEN as of 7/26
                                                    //HELP ME, RYAN! YOU'RE MY ONLY HOPE! 7/26/16
        
       case 12:
        {
          //Poll for remaining failsafe time in minutes:
          eventlog.println("Poll failsafe time remaining  12");
          unsigned long timeLeft = cutTime / 60000;
          sendXBee(String(timeLeft));
        }
        break;
        
      case 03:
        {
          //Cutdown and check cutdown status
          eventlog.println("Initiate Cutdown  03");
          murderIt(); //NEEDS TO BE WRITTEN as of 7/26
        }
        
      case 13:
        {
          //Check cutdown status
          eventlog.println("Poll cutdown  13");
          
          if (isBurst())
          {
            sendXBee("Cutdown Successful");
          }
          else
          {
            sendXBee("Cutdown Failed");
          }
        }
        break;
        
      case 04:
        //Just extends the arrow. Pretty much just for GT
        eventlog.println("Extend Arrow  04");
        extendArrow();
        break;
        
      case 05:
        eventlog.println("Retract Arrow  05");
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
      
        
  }
}













