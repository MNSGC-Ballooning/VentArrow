boolean datalogOpen = false;
boolean eventlogOpen = false;

void openDatalog(){
  if(!datalogOpen){
    datalog = SD.open(datafile, FILE_WRITE);
    datalogOpen = true;
    digitalWrite(dataLED, HIGH);
  }
}

void closeDatalog(){
  if(datalogOpen){
    datalog.close();
    datalogOpen = false;
    if(!eventlogOpen)
      digitalWrite(dataLED, LOW);
  }
}

void openEventlog(){
  if(!eventlogOpen){
    eventlog = SD.open(eventfile, FILE_WRITE);
    eventlogOpen = true;
    digitalWrite(dataLED, HIGH);
  }
}

void closeEventlog(){
  if(eventlogOpen){
    eventlog.close();
    eventlogOpen = false;
    if(!datalogOpen)
      digitalWrite(dataLED, LOW);
  }
}
