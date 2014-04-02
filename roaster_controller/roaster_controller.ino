const int FAN_PIN = 11;
const int HEAT_PIN = 13;

unsigned long dry_delay = 4 * 60 * 1000; // 4 minutes in milliseconds.
unsigned long cool_delay = 2 * 60 * 1000;
unsigned long roast_delay = 30 * 1000;

int fan_dry = 200;
int fan_start = 160;
int fan_end = 100;
int fan_cool = 120;
// 12 is fast. 20 is medium. 30 is slow.
int intervals = 20;
int fan_step = (fan_start - fan_end) / intervals;

int heat = 0;
int fan = 0;

void doRoast() {
  // Spool up fan
  for (int i = 0; i <= fan_dry; i++) {
    analogWrite(FAN_PIN, i);
    delay(10);
  }
  // Turn on heat and wait for drying period.
  unsigned long dry_end = millis() + dry_delay;
  digitalWrite(HEAT_PIN, HIGH);
  while (millis() < dry_end) {
    delay(10);
  }
  
  // Ramp down fan speed over time
  for (int i = fan_start; i >= fan_end; i -= fan_step) {
    analogWrite(FAN_PIN, i);
    delay(roast_delay); // delay 30 seconds.
  }
  // Cooling period
  digitalWrite(HEAT_PIN, LOW);
  analogWrite(FAN_PIN, fan_cool);
  delay(cool_delay); // Cool for 2 minutes.
  analogWrite(FAN_PIN, 0);
}

void setup() {
  Serial.begin(9600);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
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


