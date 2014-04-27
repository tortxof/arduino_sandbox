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
const int DEFAULT_MANUAL_DURATION = 30; // default for manual_duration
const int DELAY_SCROLL = 100;
const int DELAY_SPLASH = 2000;
const int DAY_IN_MINUTES = 1440;
const unsigned long DAY_IN_MS = 86400000UL;
const unsigned long HOUR_IN_MS = 3600000UL;
const unsigned long MINUTE_IN_MS = 60000UL;
const unsigned long SECOND_IN_MS = 1000UL;
const char TIME_FORMAT[] = "%02d:%02d:%02d";

unsigned long midnight = 0; // midnight, in the past, for comparison to time
unsigned long time = 0; // time as returned by millis()
unsigned long time_of_day = 0; // Seconds since midnight
int manual_duration = DEFAULT_MANUAL_DURATION; // duration of manual cycle in minutes
int set_cycle = 0; // current cycle being set in s_set_sched... states
unsigned int start_time[NUM_CYCLES];  // start times in minutes after midnight
unsigned int cycle_length[NUM_CYCLES]; // length of each cycle in minutes
boolean cycle_enabled[NUM_CYCLES];
uint8_t buttons = 0; // button state

void (*state)() = NULL;

void (*previous_state)() = NULL;

unsigned long state_change_time = 0;

// updates time midnight and time_of_day
void updateTime() {
  time =  millis();
  while (time - midnight >= DAY_IN_MS)
    midnight += DAY_IN_MS;
  time_of_day = (time - midnight) / SECOND_IN_MS;
}

// Prints time of day to lcd.
void printTime() {
  unsigned long seconds = time_of_day;
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

// like constrain but wraps. Unlike constrain, upper is non inclusive.
int constrain_wrap(int i, int lower, int upper) {
  while (i >= upper)
    i -= upper;
  while (i < 0)
    i += upper;
  return i;
}

// print time being set HH:MM
void printSetTime(int minutes) {
  if (minutes < 0) // minutes should be positive
    minutes = 0;
  int hours = 0;
  while (minutes >= 60) {
    minutes -= 60;
    hours++;
  }
  hours = constrain(hours, 0, 99); // our string can only hold 2 hours digits
  char time_str[6];
  sprintf(time_str, "%02d:%02d", hours, minutes);
  lcd.setCursor(11, 1);
  lcd.print(time_str);
}

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

void menuSelect(int selection) {
  if (selection == 0)
    state = s_manual_begin;
  if (selection == 1)
    state = s_set_time_begin;
  if (selection == 2)
    state = s_set_sched_begin;
  if (selection == 3)
    state = s_auto_begin;
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

  state = s_splash_begin;
}

void loop() {
  updateTime();

  if (state != previous_state) {
    state_change_time = time;
    previous_state = state;
  }

  buttons = lcd.readButtons();

  state();

  delay(DELAY_SCROLL);
}

void s_splash_begin() {
  lcd.clear();
  lcd.print(F("Sprinkler Timer"));
  state = s_splash;
}

void s_splash() {
  if (time - state_change_time > DELAY_SPLASH)
    state = s_menu_begin;
}

void s_menu_begin() {
  lcd.clear();
  lcd.print(F("Menu"));
  state = s_menu;
}

void s_menu() {
  static int selection = 0;
  printTime();
  if (buttons & BUTTON_UP)
    selection++;
  if (buttons & BUTTON_DOWN)
    selection--;
  selection = constrain_wrap(selection, 0, NUM_MENU_ITEMS);
  printMenuText(selection);
  if (buttons & BUTTON_SELECT)
    menuSelect(selection);
}

void s_auto_begin() {
  lcd.clear();
  lcd.print(F("Auto"));
  state = s_auto;
}

void s_auto() {
  printTime();
  lcd.setCursor(0, 1);
  for (int i = 0; i < NUM_CYCLES; i++) {
    if (cycle_enabled[i] && (time_of_day > ((unsigned long)start_time[i] * 60UL)) && (time_of_day < (unsigned long)(start_time[i] + cycle_length[i]) * 60UL)) {
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
  if (buttons)
    state = s_auto_end;
}

void s_auto_end() {
  digitalWrite(VALVE_PIN, LOW);
  state = s_menu_begin;
}

void s_manual_begin() {
  lcd.clear();
  lcd.print(F("Manual"));
  lcd.setCursor(0, 1);
  lcd.print(F("Duration"));
  manual_duration = DEFAULT_MANUAL_DURATION;
  state = s_manual_set;
}

void s_manual_set() {
  printTime();
  if (buttons & BUTTON_RIGHT)
    state = s_manual_wait_begin;
  else if (buttons & BUTTON_UP)
    manual_duration++;
  else if (buttons & BUTTON_DOWN)
    manual_duration--;
  manual_duration = constrain_wrap(manual_duration, 0, MAX_CYCLE_LENGTH + 1);
  printSetTime(manual_duration);
}

void s_manual_wait_begin() {
  lcd.setCursor(0, 1);
  lcd.print(F("On              "));
  digitalWrite(VALVE_PIN, HIGH);
  state = s_manual_wait;
}

void s_manual_wait() {
  printTime();
  unsigned long time_remaining = ((unsigned long)manual_duration * MINUTE_IN_MS) - (time - state_change_time);
  if (time_remaining >= HOUR_IN_MS) { // if time_remaining is an hour or more, convert to minutes
    time_remaining = time_remaining / MINUTE_IN_MS; // convert from ms to minutes
  }
  else { // less than an hour, convert to seconds
    time_remaining = time_remaining / SECOND_IN_MS;// convert from ms to seconds
  }
  printSetTime((int)time_remaining);
  if ((time - state_change_time) > ((unsigned long)manual_duration * MINUTE_IN_MS))
    state = s_manual_end;
  if (buttons)
    state = s_manual_end;
}

void s_manual_end() {
  digitalWrite(VALVE_PIN, LOW);
  state = s_menu_begin;
}

void s_set_time_begin() {
  lcd.clear();
  lcd.print(F("Set Time"));
  lcd.setCursor(0, 1);
  lcd.print(F("Hours           "));
  state = s_set_time_hours;
}

void s_set_time_hours() {  
  printTime();
  if (buttons & BUTTON_DOWN) {
    midnight += HOUR_IN_MS;
  }
  else if (buttons & BUTTON_UP) {
    midnight -= HOUR_IN_MS;
  }
  else if (buttons & BUTTON_RIGHT) {
    lcd.setCursor(0, 1);
    lcd.print(F("Minutes         "));
    state = s_set_time_minutes;
  }
}

void s_set_time_minutes() {
  printTime();
  if (buttons & BUTTON_DOWN) {
    midnight += MINUTE_IN_MS;
  }
  else if (buttons & BUTTON_UP) {
    midnight -= MINUTE_IN_MS;
  }
  else if (buttons & BUTTON_RIGHT) {
    lcd.setCursor(0, 1);
    lcd.print(F("Seconds         "));
    state = s_set_time_seconds;
  }
}

void s_set_time_seconds() {  
  printTime();
  if (buttons & BUTTON_DOWN) {
    midnight += SECOND_IN_MS;
  }
  else if (buttons & BUTTON_UP) {
    midnight -= SECOND_IN_MS;
  }
  else if (buttons & BUTTON_RIGHT)
    state = s_menu_begin;
}

void s_set_sched_begin() {
  lcd.clear();
  lcd.print(F("Sched "));
  lcd.print(set_cycle + 1);
  lcd.print(F(" Start"));
  lcd.setCursor(0, 1);
  lcd.print(F("Hour       "));
  state = s_set_sched_hour;
}

void s_set_sched_hour() {
  if (buttons & BUTTON_RIGHT) {
    lcd.setCursor(0, 1);
    lcd.print(F("Minute     "));
    state = s_set_sched_minute;
  }
  else if (buttons & BUTTON_UP) {
    start_time[set_cycle] += 60;
  }
  else if (buttons & BUTTON_DOWN) {
    start_time[set_cycle] -= 60;
  }
  start_time[set_cycle] = constrain_wrap(start_time[set_cycle], 0, DAY_IN_MINUTES);
  printSetTime(start_time[set_cycle]);
}

void s_set_sched_minute() {
  if (buttons & BUTTON_RIGHT) {
    lcd.setCursor(8, 0);
    lcd.print(F("        "));
    lcd.setCursor(0, 1);
    lcd.print(F("Duration   "));
    state = s_set_sched_duration;
  }
  else if (buttons & BUTTON_UP) {
    start_time[set_cycle]++;
  }
  else if (buttons & BUTTON_DOWN) {
    start_time[set_cycle]--;
  }
  start_time[set_cycle] = constrain_wrap(start_time[set_cycle], 0, DAY_IN_MINUTES);
  printSetTime(start_time[set_cycle]);
}

void s_set_sched_duration() {
  if (buttons & BUTTON_RIGHT) {
    lcd.setCursor(0, 1);
    lcd.print(F("Enable     "));
    state = s_set_sched_enable;
  }
  else if (buttons & BUTTON_UP) {
    cycle_length[set_cycle]++;
  }
  else if (buttons & BUTTON_DOWN) {
    cycle_length[set_cycle]--;
  }
  cycle_length[set_cycle] = constrain_wrap(cycle_length[set_cycle], 0, MAX_CYCLE_LENGTH + 1);
  printSetTime(cycle_length[set_cycle]);
}

void s_set_sched_enable() {
  if (buttons & BUTTON_RIGHT) {
    state = s_set_sched_end;
  }
  else if ((buttons & BUTTON_UP) || (buttons & BUTTON_DOWN))
    cycle_enabled[set_cycle] = !cycle_enabled[set_cycle];
  lcd.setCursor(11, 1);
  if (cycle_enabled[set_cycle])
    lcd.print(F("On   "));
  else
    lcd.print(F("Off  "));
}

void s_set_sched_end () {
  if (set_cycle >= NUM_CYCLES - 1) {
    set_cycle = 0;
    state = s_menu_begin;
  }
  else {
    set_cycle++;
    state = s_set_sched_begin;
  }
}
