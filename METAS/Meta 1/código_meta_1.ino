int led = 6;
int button = 7;

int on = 0;
int estado = 0;
int intensit = 0;
void setup(){
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
}
void loop(){
  if(intensit == 255){
    on = 1;
  }
  if(intensit == 0){
    on = 0;
  }
  estado = digitalRead(button);
  if(estado ==HIGH){
    if(on == 1){
      intensit = intensit - 51;
    }
    else{
      intensit = intensit + 51;
    }
  }
  analogWrite(led,intensit);
  delay(300);
}
