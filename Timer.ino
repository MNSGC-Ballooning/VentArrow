Event::Event(String a, int t) {
  startTime = millis() + t * 1000;
  action = a;
}

boolean Event::checkTimer() {
  if (millis() > startTime) return true;
  else return false;
}

boolean Event::checkAction(String a) {
  if (a.equals(action)) return true;
  else return false;
}

void checkEvents() {
  for (vector<Event>::iterator it = events.begin(); it < events.end(); it++) {
    if ((*it).checkTimer()) {
      if ((*it).checkAction("stopVent")) stopVent();
      else if ((*it).checkAction("openVent")) openVent();
      else if ((*it).checkAction("closeVent")) closeVent();
      it--;
      events.erase(it + 1);
      it++;
    }
  }
}

