#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

const int FAN_PIN = 11;
const int HEAT_PIN = 13;
const int FAN_MIN = 50;

// These get set true if a (c)ool or (f)ull_stop command is received
boolean cool = false;
boolean full_stop = false;

unsigned long dry_delay = 4UL * 60UL * 1000UL; // 4 minutes in milliseconds.
unsigned long cool_delay = 2UL * 60UL * 1000UL;

int fan_dry = 200;
int fan_start = 160;
int fan_end = 100;
int fan_cool = 120;
int fan_step = 1;
unsigned long roast_delay = 10000UL; // Delay between fan speed steps.

void updateOutput(int heat, int fan) {
  analogWrite(FAN_PIN, fan);
  if (heat > 0 && fan >= FAN_MIN) {
    digitalWrite(HEAT_PIN, HIGH);
  }
  else {
    digitalWrite(HEAT_PIN, LOW);
  }
}

void checkCommands() {
}

void doRoast() {
  unsigned long end_time = 0UL;
  cool = false;
  full_stop = false;

  // Spool up fan
  for (int i = 0; i <= fan_dry; i++) {
    updateOutput(0, i);
    delay(10);
  }

  checkCommands();

  // Turn on heat and wait for drying period.
  updateOutput(1, fan_dry);
  end_time = millis() + dry_delay;
  while ((millis() < end_time) && !cool && !full_stop) {
    delay(100);
    checkCommands();
  }

  // Ramp down fan speed over time
  for (int i = fan_start; i >= fan_end; i -= fan_step) {
    updateOutput(1, i);
    end_time = millis() + roast_delay;
    while ((millis() < end_time) && !cool && !full_stop) {
      delay(100);
      checkCommands();
    }
    if (cool || full_stop)
      break;
  }

  // Cooling period
  updateOutput(0, fan_cool);
  end_time = millis() + cool_delay;
  while ((millis() < end_time) && !full_stop) {
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
  doRoast();
  delay(100);
}

