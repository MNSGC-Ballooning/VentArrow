Event::Event(String a, int t) {
  startTime = millis() + t * 1000;
  action = a;
}

Event::checkTimer() {
  if (millis() > startTime) {
    if (action.equals("openVent"))
      openVent();
  }
}

