int heat = 0;
int fan = 0;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    heat = Serial.parseInt();
    fan = Serial.parseInt();
    if (heat > 0 && fan >= 80) {
      digitalWrite(13, HIGH);
    }
    else {
      digitalWrite(13, LOW);
    }
    analogWrite(11, fan);
  }
  delay(10);
}


