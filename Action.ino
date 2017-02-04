void OpenVentAction::doAction() {openVent();}
boolean OpenVentAction::isRemovedOn(String type) {
  if (type.equals("vent")) return true;
  else return false;
}


void CloseVentAction::doAction() {closeVent();}
boolean CloseVentAction::isRemovedOn(String type) {
  if (type.equals("vent")) return true;
  else return false;
}

void StopVentAction::doAction() {stopVent();}
boolean StopVentAction::isRemovedOn(String type) {
  if (type.equals("vent")) return true;
  else return false;
}

void ExtendArrowAction::doAction() {extendArrow();}
boolean ExtendArrowAction::isRemovedOn(String type) {
  if (type.equals("arrow")) return true;
  else return false;
}

void RetractArrowAction::doAction() {retractArrow();}
boolean RetractArrowAction::isRemovedOn(String type) {
  if (type.equals("arrow")) return true;
  else return false;
}

void StopArrowAction::doAction() {stopArrow();}
boolean StopArrowAction::isRemovedOn(String type) {
  if (type.equals("arrow")) return true;
  else return false;
}

void CheckRateAction::doAction() {checkNewRate();}

void StartBurstCheck::doAction() {startBurstCheck();}

void BurstCheck::doAction() {burstCheck();}

void checkActions() {
  for (vector<Action*>::iterator it = actions.begin(); it < actions.end(); it++) {
    if ((*it)->checkTimer()) {
      (*it)->doAction();
      delete *it;
      it--;
      actions.erase(it + 1);
    }
  }
}

