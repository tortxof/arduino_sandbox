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

// Arrows
byte arrow_l[8] = {
  B00000,
  B00000,
  B00010,
  B00110,
  B01110,
  B00110,
  B00010,
  B00000,
};
byte arrow_r[8] = {
  B00000,
  B00000,
  B01000,
  B01100,
  B01110,
  B01100,
  B01000,
  B00000,
};
byte arrow_u[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B00000,
};
byte arrow_d[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000,
};

const int FAN_PIN = 11;
const int HEAT_PIN = 10;
const int FAN_MIN = 50;
const int DELAY_BUTTON = 10; // Time to wait in button loops

int fan_dry = 200;
int fan_start = 160;
int fan_end = 100;
int fan_cool = 120;
int fan_spool_step = 10;
int fan_step = 1;
int dry_delay = 4 * 60; // 4 minutes in seconds
int cool_delay = 2 * 60;
int roast_delay = 10; // Delay between fan speed steps in second.
int man_fan_step = 5;

void updateOutput(int heat, int fan) {
  lcd.setCursor(0, 1);
  lcd.print("Fan:    ");
  lcd.setCursor(4, 1);
  lcd.print(fan);
  lcd.setCursor(8, 1);
  analogWrite(FAN_PIN, fan);
  if (heat > 0 && fan >= FAN_MIN) {
    digitalWrite(HEAT_PIN, HIGH);
    lcd.print("Heat:On ");
  }
  else {
    digitalWrite(HEAT_PIN, LOW);
    lcd.print("Heat:Off");
  }
}

void printTimeRemaining(unsigned long end_time) {
  lcd.setCursor(13, 0);
  lcd.print((end_time - millis()) / 1000);
  lcd.print("  ");
}

void printTimeElapsed(unsigned long start_time) {
  lcd.setCursor(9, 0);
  lcd.print((millis() - start_time) / 1000);
  lcd.print("  ");
}

void checkCommands(boolean *cool, boolean *full_stop) {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    if (buttons & BUTTON_LEFT) {
      *full_stop = true;
    }
    if (buttons & BUTTON_RIGHT) {
      *cool = true;
    }
  }
}

int setParam(int value, int step_size, char desc[]) {
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print(desc);
    lcd.print(value);
    lcd.print("                ");
    uint8_t buttons = lcd.readButtons();
    if (buttons & BUTTON_UP)
      value += step_size;
    else if (buttons & BUTTON_DOWN)
      value -= step_size;
    else if (buttons & BUTTON_RIGHT)
      break;
  }
  delay(200);
  return value;
}

void doConfig() {
  lcd.clear();
  lcd.setBacklight(VIOLET);
  lcd.setCursor(0, 0);
  lcd.print("Configure");

  fan_start = setParam(fan_start, 5, "Fan Start: ");
  fan_end = setParam(fan_end, 1, "Fan End: ");
  fan_dry = setParam(fan_dry, 5, "Fan Dry: ");
  fan_cool = setParam(fan_cool, 5, "Fan Cool: ");
  dry_delay = setParam(dry_delay, 10, "Dry Time: ");
  roast_delay = setParam(roast_delay, 1, "Roast Delay: ");
  cool_delay = setParam(cool_delay, 10, "Cool Time: ");
  man_fan_step = setParam(man_fan_step, 1, "Man Fan Step: ");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Config done");
  delay(1000);
}

void doRoast() {
  unsigned long end_time = 0UL;
  unsigned long start_time = 0UL;
  boolean cool = false;
  boolean full_stop = false;

  // Spool up fan
  lcd.clear();
  lcd.setBacklight(YELLOW);
  lcd.print("Spooling up");
  for (int i = 0; i <= fan_dry; i += fan_spool_step) {
    updateOutput(0, i);
    delay(10);
  }

  checkCommands(&cool, &full_stop);

  // Turn on heat and wait for drying period.
  lcd.clear();
  lcd.setBacklight(YELLOW);
  lcd.print("Drying");
  updateOutput(1, fan_dry);
  start_time = millis();
  end_time = millis() + ((unsigned long)dry_delay * 1000UL);
  while ((millis() < end_time) && !cool && !full_stop) {
    printTimeElapsed(start_time);
    printTimeRemaining(end_time);
    delay(DELAY_BUTTON);
    checkCommands(&cool, &full_stop);
  }

  // Ramp down fan speed over time
  lcd.clear();
  lcd.setBacklight(RED);
  lcd.print("Roasting");
  for (int i = fan_start; i >= fan_end; i -= fan_step) {
    end_time = millis() + ((unsigned long)roast_delay * 1000UL);
    updateOutput(1, i);
    while ((millis() < end_time) && !cool && !full_stop) {
      printTimeRemaining(end_time);
      printTimeElapsed(start_time);
      delay(DELAY_BUTTON);
      checkCommands(&cool, &full_stop);
    }
    if (cool || full_stop)
      break;
  }

  // Cooling period
  lcd.clear();
  lcd.setBacklight(BLUE);
  lcd.print("Cooling");
  updateOutput(0, fan_cool);
  end_time = millis() + ((unsigned long)cool_delay * 1000UL);
  while ((millis() < end_time) && !full_stop) {
    printTimeRemaining(end_time);
    printTimeElapsed(start_time);
    delay(DELAY_BUTTON);
    checkCommands(&cool, &full_stop);
  }

  // Stop
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.print("Done");
  printTimeElapsed(start_time);
  updateOutput(0, 0);
  while (!lcd.readButtons())
    delay(DELAY_BUTTON);
}

void doManual() {
  int fan = 0;
  int heat = 0;
  unsigned long start_time = millis();

  lcd.clear();
  lcd.setBacklight(BLUE);
  lcd.print("Manual");
  while (!(lcd.readButtons() & BUTTON_SELECT)) {
    uint8_t buttons = lcd.readButtons();
    if (buttons & BUTTON_RIGHT) {
      lcd.setBacklight(RED);
      heat = 1;
    }
    if (buttons & BUTTON_LEFT) {
      lcd.setBacklight(BLUE);
      heat = 0;
    }
    if (buttons & BUTTON_UP) {
      fan += man_fan_step;
      if (fan < FAN_MIN)
        fan = FAN_MIN;
    }
    if (buttons & BUTTON_DOWN) {
      fan -= man_fan_step;
      if (fan < FAN_MIN)
        fan = 0;
    }
    printTimeElapsed(start_time);
    updateOutput(heat, fan);
    delay(DELAY_BUTTON);
  }
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.print("Manual halted.");
  updateOutput(0, 0);
  delay(1000);
}

void setup() {
  pinMode(HEAT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  lcd.begin(16, 2);

  lcd.createChar(0, arrow_u);
  lcd.createChar(1, arrow_d);
  lcd.createChar(2, arrow_l);
  lcd.createChar(3, arrow_r);

  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.print(" Coffee Roaster ");
  delay(2000);
}

void loop() {
  // Define menu line
  char menu_line[] = " Man S=Auto Cfg ";
  menu_line[0] = byte(2);
  menu_line[15] = byte(3);

  // Update display
  lcd.clear();
  lcd.setBacklight(GREEN);
  lcd.print("     Ready      ");
  lcd.setCursor(0, 1);
  lcd.print(menu_line);

  // Wait for button push
  while (!lcd.readButtons())
    delay(DELAY_BUTTON);

  uint8_t buttons = lcd.readButtons();
  if (buttons & BUTTON_SELECT)
    doRoast();
  else if (buttons & BUTTON_RIGHT)
    doConfig();
  else if (buttons & BUTTON_LEFT)
    doManual();
}

