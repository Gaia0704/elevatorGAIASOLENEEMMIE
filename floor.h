/**
 * @file floor.h
 * @brief Floor/Elevator component management functions.
 */
#ifndef _FLOOR_H
#define _FLOOR_H

#include "wino/Adafruit_LEDBackpack.h"

#define FLOOR_FEEDBACK_PERIOD 100 

#define I2C_LCD       0x20
#define I2C_7SEG_BASE	0x70

#define PIN_LEDS      15
#define PIN_CALLBTNS  A0
#define PIN_KEYMAP_ROWS { 13, 12, 11, 10 }
#define PIN_KEYMAP_COLS {  9,  8,  7,  6 }

#define KEYMAP "123A" "456B" "789C" "*0#D"

#define MAX_LEDS         14
#define MAX_LCD_COLS     16
#define MAX_FLOOR_TITLE  MAX_LCD_COLS+1

/**
 * @struct floor_info
 * @brief Holds configuration and state information for a specific floor.
 */
struct floor_info {
  /** @brief String title/label of the floor (e.g., "Lobby", "Basement"). */
  char title[MAX_FLOOR_TITLE]; 
  
  /** @brief Character mapped to the cabin keypad for this floor. */
  char key; 
  
  /** @brief Numeric value to show on 7-segment displays (e.g., -1 for Parking). */
  int displayVal; 
  
  /** @brief Flag set to true if this is the default starting floor. */
  bool def; 
  
  /** @brief Index of the NeoPixel LEDs for the 'Up' and 'Down' floor call buttons. */
  int ledUp, ledDown; 
  
  /** @brief Expected analog resistance values to detect 'Up' and 'Down' button presses. */
  int callUp, callDown; 
  
  /** @brief Bitmask or flag representing the current pressed state of floor buttons. */
  int pressed; 
  
  /** @brief Dedicated 7-segment display object located at this floor's landing. */
  Adafruit_7segment upper7seg;
};

/**
 * @brief Initializes the hardware components for all floors.
 * * Configures displays, LEDs, and pins based on the provided floor configuration array.
 * It identifies the designated starting floor during the process.
 * @param floors Array of floor_info structures.
 * @param floor_num Total number of floors in the system.
 * @return int The index of the starting floor (determined by the 'def' field).
 */
int  floor_init     (floor_info floors[], size_t floor_num);

/**
 * @brief Polls the state of all elevator buttons.
 * Reads inputs from the landing call buttons (hall calls) and the internal 
 * cabin keypad (car calls).
 */
void floor_readbtns ();

/**
 * @brief Provides visual/auditory feedback to the users.
 * Updates the leds, 7-segment displays at every landing and the cabin LCD with the 
 * current floor number.
 * @param current The index of the current floor the elevator is at.
 * @param status Optional string message to display on the second line of the cabin LCD (e.g., "Moving", "Door Open").
 */
void floor_feedback (int current, const char *status = nullptr);

/**
 * @brief Determines the next destination floor based on active requests.
 * Evaluates pending calls to decide where the elevator should go next relative 
 * to its current position. This function also clears (sets to 0) the request flags 
 * for the floor provided in the 'from' parameter once arrived.
 * @param from The index of the floor where the elevator is currently located or starting from.
 * @return int The index of the next floor to visit, or a sentinel value if no floors are requested.
 */
int  floor_requested(int from);

#endif