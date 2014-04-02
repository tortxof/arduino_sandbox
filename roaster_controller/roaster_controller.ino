const int FAN_PIN = 11;
const int HEAT_PIN = 13;
const int FAN_MIN = 50;

// These get set true if a (c)ool or (f)ull_stop command is received
boolean cool = false;
boolean full_stop = false;

unsigned long dry_delay = 4 * 60 * 1000; // 4 minutes in milliseconds.
unsigned long cool_delay = 2 * 60 * 1000;

int fan_dry = 200;
int fan_start = 160;
int fan_end = 100;
int fan_cool = 120;
int fan_step = 1;
int ramp_time = 12 * 60; // Time in seconds to ramp fan down.
unsigned long roast_delay = (ramp_time * 1000) / (fan_start - fan_end); // Delay between fan speed steps.

void checkCommands() {
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    switch (inByte) {
    case 'c':
      cool = true;
      break;
    case 'f':
      full_stop = true;
      break;
    }
  }
}

void updateOutput(int heat, int fan) {
  analogWrite(FAN_PIN, fan);
  if (heat > 0 && fan >= FAN_MIN) {
    digitalWrite(HEAT_PIN, HIGH);
    Serial.print('1');
  }
  else {
    digitalWrite(HEAT_PIN, LOW);
    Serial.print('0');
  }
  Serial.print(',');
  Serial.println(fan);
}

void doRoast() {
  unsigned long end_time = 0;

  // Spool up fan
  for (int i = 0; i <= fan_dry; i++) {
    updateOutput(0, i);
    delay(10);
  }

  checkCommands();
  if (full_stop) {
    updateOutput(0, 0);
    return;
  }

  // Turn on heat and wait for drying period.
  updateOutput(1, fan_dry);
  end_time = millis() + dry_delay;
  while (millis() < end_time && !cool && !full_stop) {
    delay(100);
    checkCommands();
  }

  // Ramp down fan speed over time
  for (int i = fan_start; i >= fan_end; i -= fan_step) {
    updateOutput(1, i);
    end_time = millis() + roast_delay;
    while (millis() < end_time && !cool && !full_stop) {
      delay(100);
      checkCommands();
    }
    if (cool || full_stop)
      break;
  }

  // Cooling period
  updateOutput(0, fan_cool);
  end_time = millis() + cool_delay;
  while (millis() < end_time && !full_stop) {
    delay(100);
    checkCommands();
  }

  // Stop
  updateOutput(0, 0);
}

void setup() {
  Serial.begin(9600);
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0 && Serial.read() == 's')
    doRoast();
  delay(100);
}

