void sendXBee(String out) {
  Serial3.println(out);
  eventlog.println(flightTimeStr() + "  TX  " + out);
}

void acknowledge() {
  xBee.println(xBeeID + "\n");
}

String lastCommand = "";
unsigned long commandTime = 0;

void xBeeCommand() {
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
  }
}

