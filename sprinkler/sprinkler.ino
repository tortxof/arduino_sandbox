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

const int VALVE_PIN = 12;
const int NUM_MENU_ITEMS = 4;
const int NUM_CYCLES = 4;
const int MAX_CYCLE_LENGTH = 240; // minutes
const int DELAY_SCROLL = 200;
const int DELAY_SPLASH = 2000;
const unsigned long DAY_IN_MS = 86400000UL;
const unsigned long HOUR_IN_MS = 3600000UL;
const unsigned long MINUTE_IN_MS = 60000UL;
const unsigned long SECOND_IN_MS = 1000UL;
const char TIME_FORMAT[] = "%02d:%02d:%02d";

unsigned long midnight = 0; // midnight, in the past, for comparison to millis()
unsigned int start_time[NUM_CYCLES];  // start times in minutes after midnight
unsigned int cycle_length[NUM_CYCLES]; // length of each cycle in minutes
boolean cycle_enabled[NUM_CYCLES];

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

// Returns time of day in seconds.
unsigned long getTime() {
  unsigned long now =  millis();
  while (now - midnight >= DAY_IN_MS)
    midnight += DAY_IN_MS;
  return (now - midnight) / 1000UL;
}

// Prints time of day to lcd.
void printTime() {
  unsigned long seconds = getTime();
  unsigned int minutes = 0;
  unsigned int hours = 0;
  while (seconds >= 60) {
    seconds -= 60;
    minutes++;
  }
  while (minutes >= 60) {
    minutes -= 60;
    hours++;
  }
  lcd.setCursor(8, 0);
  char time_str[9];
  sprintf(time_str, TIME_FORMAT, hours, minutes, seconds);
  lcd.print(time_str);
}

void printStartTime(int minutes) {
  int hours = 0;
  while (minutes >= 60) {
    minutes -= 60;
    hours++;
  }
  char time_str[6];
  sprintf(time_str, "%02d:%02d", hours, minutes);
  lcd.setCursor(11, 1);
  lcd.print(time_str);
}

void doMenuSelection(int selection) {
  delay(DELAY_SCROLL);
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

  int duration = 0;

  lcd.setCursor(0, 1);
  lcd.print(F("Duration"));

  while (true) {
    uint8_t buttons = lcd.readButtons();
    printTime();
    printStartTime(duration);
    if (buttons & BUTTON_RIGHT)
      break;
    else if (buttons & BUTTON_UP)
      duration++;
    else if (buttons & BUTTON_DOWN)
      duration--;
    duration = constrain(duration, 0, MAX_CYCLE_LENGTH);
    delay(DELAY_SCROLL);
  }

  lcd.setCursor(0, 1);
  lcd.print(F("On              "));

  unsigned long start_time = getTime();
  unsigned long now = start_time;
  digitalWrite(VALVE_PIN, HIGH);
  while (!lcd.readButtons() && (now - start_time < ((unsigned long)duration * 60UL))) {
    printTime();
    printStartTime((start_time + ((unsigned long)duration * 60UL) - now) / 60UL);
    now = getTime();
  }
  digitalWrite(VALVE_PIN, LOW);
}

void doSetTime() {
  lcd.clear();
  lcd.print(F("Set Time"));

  uint8_t buttons = 0;

  lcd.setCursor(0, 1);
  lcd.print(F("Hours           "));
  while (true) {
    printTime();
    buttons = lcd.readButtons();
    if (buttons & BUTTON_UP) {
      midnight += HOUR_IN_MS;
      delay(DELAY_SCROLL);
    }
    else if (buttons & BUTTON_DOWN) {
      midnight -= HOUR_IN_MS;
      delay(DELAY_SCROLL);
    }
    else if (buttons & BUTTON_RIGHT)
      break;
  }

  lcd.setCursor(0, 1);
  lcd.print(F("Minutes         "));
  while (true) {
    printTime();
    buttons = lcd.readButtons();
    if (buttons & BUTTON_UP) {
      midnight += MINUTE_IN_MS;
      delay(DELAY_SCROLL);
    }
    else if (buttons & BUTTON_DOWN) {
      midnight -= MINUTE_IN_MS;
      delay(DELAY_SCROLL);
    }
    else if (buttons & BUTTON_RIGHT)
      break;
  }

  lcd.setCursor(0, 1);
  lcd.print(F("Seconds         "));
  while (true) {
    printTime();
    buttons = lcd.readButtons();
    if (buttons & BUTTON_UP) {
      midnight += SECOND_IN_MS;
      delay(DELAY_SCROLL);
    }
    else if (buttons & BUTTON_DOWN) {
      midnight -= SECOND_IN_MS;
      delay(DELAY_SCROLL);
    }
    else if (buttons & BUTTON_RIGHT)
      break;    
  }
}

void doSetSchedule() {
  uint8_t buttons = 0;
  lcd.clear();
  lcd.print(F("Sched"));

  for (int i = 0; i < NUM_CYCLES; i++) {
    lcd.setCursor(6, 0);
    lcd.print(i + 1);
    lcd.print(F(" Start"));

    lcd.setCursor(0, 1);
    lcd.print(F("Hour       "));
    delay(DELAY_SCROLL);
    while (true) {
      buttons = lcd.readButtons();
      if (buttons & BUTTON_RIGHT)
        break;
      else if (buttons & BUTTON_UP) {
        start_time[i] += 60;
        delay(DELAY_SCROLL);
      }
      else if (buttons & BUTTON_DOWN) {
        start_time[i] -= 60;
        delay(DELAY_SCROLL);
      }
      start_time[i] = constrain(start_time[i], 0, 1440);
      printStartTime(start_time[i]);
    }

    lcd.setCursor(0, 1);
    lcd.print(F("Minute     "));
    delay(DELAY_SCROLL);
    while (true) {
      buttons = lcd.readButtons();
      if (buttons & BUTTON_RIGHT)
        break;
      else if (buttons & BUTTON_UP) {
        start_time[i]++;
        delay(DELAY_SCROLL);
      }
      else if (buttons & BUTTON_DOWN) {
        start_time[i]--;
        delay(DELAY_SCROLL);
      }
      start_time[i] = constrain(start_time[i], 0, 1440);
      printStartTime(start_time[i]);
    }

    lcd.setCursor(8, 0);
    lcd.print(F("        "));

    lcd.setCursor(0, 1);
    lcd.print(F("Duration   "));
    delay(DELAY_SCROLL);
    while (true) {
      buttons = lcd.readButtons();
      if (buttons & BUTTON_RIGHT)
        break;
      else if (buttons & BUTTON_UP) {
        cycle_length[i]++;
        delay(DELAY_SCROLL);
      }
      else if (buttons & BUTTON_DOWN) {
        cycle_length[i]--;
        delay(DELAY_SCROLL);
      }
      cycle_length[i] = constrain(cycle_length[i], 0, MAX_CYCLE_LENGTH);
      printStartTime(cycle_length[i]);
    }

    lcd.setCursor(0, 1);
    lcd.print(F("Enable     "));
    delay(DELAY_SCROLL);
    while (true) {
      buttons = lcd.readButtons();
      if (buttons & BUTTON_RIGHT)
        break;
      else if ((buttons & BUTTON_UP) || (buttons & BUTTON_DOWN)) {
        cycle_enabled[i] = !cycle_enabled[i];
        delay(DELAY_SCROLL);
      }
      lcd.setCursor(11, 1);
      if (cycle_enabled[i])
        lcd.print(F("On   "));
      else
        lcd.print(F("Off  "));
    }
  }
}

void doAuto() {
  lcd.clear();
  lcd.print(F("Auto"));
  while (!lcd.readButtons()) {
    printTime();
    unsigned long now = getTime();
    lcd.setCursor(0, 1);
    for (int i = 0; i < NUM_CYCLES; i++) {
      if (cycle_enabled[i] && (now > ((unsigned long)start_time[i] * 60UL)) && (now < (unsigned long)(start_time[i] + cycle_length[i]) * 60UL)) {
        digitalWrite(VALVE_PIN, HIGH);
        lcd.print(i);
        lcd.print(F(":+ "));
      }
      else {
        digitalWrite(VALVE_PIN, LOW);
        lcd.print(i);
        lcd.print(F(":- "));
      }
    }
  }
  digitalWrite(VALVE_PIN, LOW);
}

void setup() {
  // initialise auto vars
  for (int i = 0; i < NUM_CYCLES; i++) {
    start_time[i] = 0;
    cycle_length[i] = 30;
    cycle_enabled[i] = false;
  }

  lcd.begin(16, 2);
  pinMode(VALVE_PIN, OUTPUT);
}

void loop() {
  lcd.clear();
  lcd.setBacklight(WHITE);
  lcd.print(F("Menu"));
  delay(DELAY_SCROLL);

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
