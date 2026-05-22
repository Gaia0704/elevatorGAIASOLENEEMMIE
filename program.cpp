#include "floor.h"
#include "cabin.h"

//                      temps
#define TIME_OPENED      2000
#define TIME_DOORS        350
#define TIME_FLOOR_SHORT 1000
#define TIME_FLOOR_LONG  1300

#define PIN_STOP A1    // bouton Stop branché sur A1

#define FLOOR_NUM 7

floor_info building[FLOOR_NUM] = {
  // {nom étage,    touche, affichage, défaut, ledUp, ledDown, callUp, callDown, pressed}
  { "Parking    ",   '*', -1, false,  9,   0,  84,   -1,  0 },
  { "RDC        ",   '0',  0, true , 11,  10, 101,   93,  0 },
  { "Etage 1    ",   '1',  1, false, 13,  12, 118,  110,  0 },
  { "Etage 2    ",   '2',  2, false,  1,   2, 135,  127,  0 },
  { "Etage 3    ",   '3',  3, false,  3,   4, 152,  144,  0 },
  { "Etage 4    ",   '4',  4, false,  5,   6, 169,  161,  0 },
  { "Etage 5    ",   '5',  5, false,  7,   8, 186,  178,  0 },
};

enum states {
  STATE_OPENED,
  STATE_MOVING,
  STATE_OPENING,
  STATE_CLOSING
};

states state = STATE_OPENED;
timer_ms timer;
int target = -1;
bool stopped = false;  // true = ascenseur arrêté par le bouton Stop

unsigned long movetime();

void setup() {
  Serial.begin(9600);
  pinMode(PIN_STOP, INPUT_PULLUP);  // INPUT_PULLUP : HIGH au repos, LOW quand appuyé
  cabin_init(
    floor_init(building, FLOOR_NUM)
  );
}

void loop() {
  const char* status = nullptr;

  // Lecture du bouton Stop (logique inversée avec PULLUP)
  if (digitalRead(PIN_STOP) == LOW) {
    stopped = !stopped;
    if (stopped) {
      cabin_stop();
      cabin_door(CABIN_DOOR_STOP);
    }
    delay(200);  // anti-rebond
  }

  // Si arrêté, on affiche le message et on ne fait rien d'autre
  if (stopped) {
    floor_feedback(cabin_current_floor(), "** ARRET **     ");
    return;
  }

  floor_readbtns();
  switch(state) {
    case STATE_OPENED:
    cabin_door(CABIN_DOOR_OPEN); 
      target = floor_requested(cabin_current_floor());
      status = "(waiting...)   ";
      if(timer_elapsed(timer, TIME_OPENED) && target >= 0) {
        cabin_door(CABIN_DOOR_CLOSE);
        state = STATE_CLOSING;
      }
      break;

    case STATE_CLOSING:
      cabin_door(CABIN_DOOR_CLOSE);
      status = "(closing doors)";
      if(timer_elapsed(timer, TIME_DOORS)) {
        cabin_door(CABIN_DOOR_STOP);
        state = STATE_MOVING;
      }
      break;

    case STATE_MOVING:
      status = "(moving)       ";
      if(cabin_move(timer, target, movetime()) == target) {
        cabin_stop();
        state = STATE_OPENING;
      }
      break;

    case STATE_OPENING:
      status = "(opening doors)";
      cabin_door(CABIN_DOOR_OPEN);
      if(timer_elapsed(timer, TIME_DOORS)) {
        cabin_door(CABIN_DOOR_STOP);
        state = STATE_OPENED;
      }
      break;
  }
  floor_feedback(cabin_current_floor(), status);
}

unsigned long movetime() {
  auto cur = cabin_current_floor();
  auto odd = (cur % 2) != 0;
  if(target < cur && odd || target > cur && !odd) {
    return TIME_FLOOR_SHORT;
  }
  else {
    return TIME_FLOOR_LONG;
  }
}
