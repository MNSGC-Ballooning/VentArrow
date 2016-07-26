void sendXBee(String out) {
  Serial3.println(out);
  eventlog.println(flightTimeStr() + "  TX  " + out);
}

void acknowledge() {
  xBee.println(xBeeID + "\n");
}

