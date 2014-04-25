#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// lcd backlight color
#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

const int NUM_MENU_ITEMS = 4;
const int DELAY_SCROLL = 200;
const int DELAY_SPLASH = 2000;
const unsigned long DAY_IN_MS = 86400000UL;
const char TIME_FORMAT[] = "%02d:%02d:%02d";

unsigned long midnight = 0;

void printMenuText(int selection) {
  lcd.setCursor(0, 1);
  if (selection == 0)
    lcd.print(F("Manual          "));
  if (selection == 1)
    lcd.print(F("Set Time        "));
  if (selection == 2)
    lcd.print(F("Set Schedule    "));
  if (selection == 3)
    lcd.print(F("Auto            "));
}

void printTime() {
  unsigned long now =  millis();
  while (now > (midnight + DAY_IN_MS))
    midnight += DAY_IN_MS;
  unsigned long seconds = (now - midnight) / 1000UL;
  unsigned int minutes = 0;
  unsigned int hours = 0;
  while (seconds > 60) {
    seconds -= 60;
    minutes++;
  }
  while (minutes > 60) {
    minutes -= 60;
    hours++;
  }
  lcd.setCursor(8, 0);
  char time_str[9];
  sprintf(time_str, TIME_FORMAT, hours, minutes, seconds);
  lcd.print(time_str);
}

void doMenuSelection(int selection) {
  if (selection == 0)
    doManual();
  if (selection == 1)
    doSetTime();
  if (selection == 2)
    doSetSchedule();
  if (selection == 3)
    doAuto();
}

void doManual() {
  lcd.clear();
  lcd.print(F("Manual"));
  delay(DELAY_SPLASH);
}

void doSetTime() {
  lcd.clear();
  lcd.print(F("Set Time"));
  delay(DELAY_SPLASH);
}

void doSetSchedule() {
  lcd.clear();
  lcd.print(F("Schedule"));
  delay(DELAY_SPLASH);
}

void doAuto() {
  lcd.clear();
  lcd.print(F("Auto"));
  delay(DELAY_SPLASH);
}

void setup() {
  lcd.begin(16, 2);
}

void loop() {
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.print(F("Menu"));

  int menu_selection = 0;
  while (!(lcd.readButtons() & BUTTON_SELECT)) {
    uint8_t buttons = lcd.readButtons();
    if (buttons & BUTTON_UP) {
      menu_selection--;
      delay(DELAY_SCROLL);
    }
    if (buttons & BUTTON_DOWN) {
      menu_selection++;
      delay(DELAY_SCROLL);
    }
    if (menu_selection < 0)
      menu_selection = 0;
    if (menu_selection >= NUM_MENU_ITEMS)
      menu_selection = NUM_MENU_ITEMS - 1;
    printMenuText(menu_selection);
    printTime();
  }

  doMenuSelection(menu_selection);
}