#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7 

const int FAN_PIN = 11;
const int HEAT_PIN = 13;
const int FAN_MIN = 50;

// These get set true if a (c)ool or (f)ull_stop command is received
boolean cool = false;
boolean full_stop = false;

int fan_dry = 200;
int fan_start = 160;
int fan_end = 100;
int fan_cool = 120;
int fan_spool_step = 10;
int fan_step = 1;
int dry_delay = 4 * 60; // 4 minutes in seconds
int cool_delay = 2 * 60;
int roast_delay = 10; // Delay between fan speed steps in second.

void updateOutput(int heat, int fan) {
  lcd.setCursor(0, 1);
  lcd.print("Fan:");
  lcd.print(fan);
  analogWrite(FAN_PIN, fan);
  if (heat > 0 && fan >= FAN_MIN) {
    digitalWrite(HEAT_PIN, HIGH);
    lcd.print(" Heat:On");
  }
  else {
    digitalWrite(HEAT_PIN, LOW);
    lcd.print(" Heat:Off");
  }
}

void checkCommands() {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    if (buttons & BUTTON_LEFT) {
      full_stop = true;
    }
    if (buttons & BUTTON_RIGHT) {
      cool = true;
    }
  }
}

void doConfig() {
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.setCursor(0, 0);
  lcd.print("Configure");

  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("Fan Start: ");
    lcd.print(fan_start);
    uint8_t buttons = lcd.readButtons();
    if (buttons & BUTTON_UP)
      fan_start++;
    else if (buttons & BUTTON_DOWN)
      fan_start--;
    else if (buttons & BUTTON_RIGHT)
      break;
  }
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.setCursor(0, 0);
  lcd.print("Config done");
  delay(1000);
}

void doRoast() {
  unsigned long end_time = 0UL;
  cool = false;
  full_stop = false;

  // Spool up fan
  lcd.clear();
  lcd.setBacklight(YELLOW);
  lcd.setCursor(0, 0);
  lcd.print("Spooling up");
  for (int i = 0; i <= fan_dry; i += fan_spool_step) {
    updateOutput(0, i);
    delay(10);
  }

  checkCommands();

  // Turn on heat and wait for drying period.
  lcd.clear();
  lcd.setBacklight(YELLOW);
  lcd.setCursor(0, 0);
  lcd.print("Drying");
  updateOutput(1, fan_dry);
  end_time = millis() + ((unsigned long)dry_delay * 1000UL);
  while ((millis() < end_time) && !cool && !full_stop) {
    delay(100);
    checkCommands();
  }

  // Ramp down fan speed over time
  lcd.clear();
  lcd.setBacklight(RED);
  lcd.setCursor(0, 0);
  lcd.print("Roasting");
  for (int i = fan_start; i >= fan_end; i -= fan_step) {
    updateOutput(1, i);
    end_time = millis() + ((unsigned long)roast_delay * 1000UL);
    while ((millis() < end_time) && !cool && !full_stop) {
      delay(100);
      checkCommands();
    }
    if (cool || full_stop)
      break;
  }

  // Cooling period
  lcd.clear();
  lcd.setBacklight(BLUE);
  lcd.setCursor(0, 0);
  lcd.print("Cooling");
  updateOutput(0, fan_cool);
  end_time = millis() + ((unsigned long)cool_delay * 1000UL);
  while ((millis() < end_time) && !full_stop) {
    delay(100);
    checkCommands();
  }

  // Stop
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.setCursor(0, 0);
  lcd.print("Done roasting");
  updateOutput(0, 0);
  while (!lcd.readButtons())
    delay(100);
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.print(" Coffee Roaster ");
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  delay(2000);
}

void loop() {
  // Update display
  lcd.clear();
  lcd.setBacklight(GREEN);
  lcd.setCursor(0, 0);
  lcd.print("Ready");

  // Wait for button push
  while (!lcd.readButtons())
    delay(100);

  uint8_t buttons = lcd.readButtons();
  if (buttons & BUTTON_SELECT)
    doRoast();
  else if (buttons & BUTTON_RIGHT)
    doConfig();
}

