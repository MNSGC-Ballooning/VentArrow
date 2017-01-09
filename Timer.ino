Action::Action(String a, int t) {
  startTime = millis() + t * 1000;
  action = a;
}

boolean Action::checkTimer() {
  if (millis() > startTime) return true;
  else return false;
}

String Action::getAction() {
  return action;
}

void checkActions() {
  for (vector<Action>::iterator it = actions.begin(); it < actions.end(); it++) {
    if ((*it).checkTimer()) {
      String action = (*it).getAction();
      if (action.equals("stopVent")) stopVent();
      else if (action.equals("openVent")) openVent();
      else if (action.equals("closeVent")) closeVent();
      else if (action.equals("stopArrow")) stopArrow();
      else if (action.equals("extendArrow")) extendArrow();
      else if (action.equals("retractArrow")) retractArrow();
      else if (action.equals("checkNewRate")) checkNewRate();
      else if (action.equals("startBurstCheck")) startBurstCheck();
      else if (action.equals("burstCheck")) burstCheck();
      it--;
      actions.erase(it + 1);
    }
  }
}

