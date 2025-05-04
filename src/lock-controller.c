/**************************************************************************//**
 *
 * @file lock-controller.c
 *
 * @author (STUDENTS -- Nolan Hill)
 * @author (STUDENTS -- Nick Goertzen)
 *
 * @brief Code to implement the "combination lock" mode.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

 #include <CowPi.h>
 #include "display.h"
 #include "lock-controller.h"
 #include "rotary-encoder.h"
 #include "servomotor.h"
 
 
 typedef enum {
     LOCKED, UNLOCKED, CHANGING, ALARMED
 } lock_state_t;
 
 static lock_state_t state = LOCKED;
 static uint8_t combination[3] __attribute__((section (".uninitialized_ram.")));
 static int bad_tries = 0;
 static uint8_t entered_combination[3];
 static int visible_counts[3] = {0, 0, 0};
 static uint8_t current_number = 0;
 static int entry_index = 0;
 static int entry_stage = 0;
 char display_buffer[9];
 
 uint8_t const *get_combination() {
     return combination;
 }
 
 void force_combination_reset() {
     combination[0] = 5;
     combination[1] = 10;
     combination[2] = 15;
 }
 
 bool check_combination(void) {
     return entered_combination[0] == combination[0]
         && entered_combination[1] == combination[1]
         && entered_combination[2] == combination[2]
         && visible_counts[0] >= 3
         && visible_counts[1] >= 2
         && visible_counts[2] >= 1;
 }
 
 static uint8_t one_through_fifteen(uint8_t n) {
     return (n + 16) % 16;
 }
 
 void update_combination_visual() { //not sure if this works how Id want it to but I havent tested it 
     snprintf(display_buffer, sizeof(display_buffer), "%02d-%02d-%02d",
              one_through_fifteen(entered_combination[0]),
              one_through_fifteen(entered_combination[1]),
              one_through_fifteen(entered_combination[2]));
     display_string(0, display_buffer);
 }
 
 void blink_leds(){
     // need to make this yet
 }
 
 // enter_number()  needs to be made
 
 
 void initialize_lock_controller() {
     state = LOCKED;
     entry_index = 0;
     bad_tries = 0;
     for (int i = 0; i < 3; i++) {
         entered_combination[i] = 0xFF;
         visible_counts[i] = 0;
     }
     current_number = 0;
 
     cowpi_illuminate_left_led();
     cowpi_deluminate_right_led();
     rotate_full_clockwise();
     display_string(0, "  -  -  ");
 }
 
 
 void control_lock() {
     if (state == ALARMED) {
         blink_leds();
         return;
     }
 
     direction_t dir = get_direction();
 
     if(state == LOCKED){
         if (entry_stage < 3) {
             //enter_number()
         } else {
             if (dir == COUNTERCLOCKWISE) {
                 //clear combination here
                 display_string(0, "-- -- --");
             }
             if (cowpi_left_button_is_pressed()) {
                 if (check_combination()) {
                     state = UNLOCKED;
                     display_string(0, "OPEN");
                     cowpi_deluminate_left_led();
                     cowpi_illuminate_right_led();
                     rotate_full_counterclockwise();
                 } else {
                     bad_tries++;
                     if (bad_tries >= 3) {
                         state = ALARMED;
                         display_string(0, "alert!");
                     } else{
                     char msg[11];
                     snprintf(msg, sizeof(msg), "bad try %d", bad_tries);
                     display_string(0, msg);
                     // make leds blink twice here
                     //clear combination here
                     display_string(0, "-- -- --");
                     }
                 }
             }
         }
     }
 
     else if(state == UNLOCKED){
         if (cowpi_left_button_is_pressed() && cowpi_right_button_is_pressed()) {
             state = LOCKED;
             cowpi_illuminate_left_led();
             cowpi_deluminate_right_led();
             rotate_full_clockwise();
             //clear combination here
             display_string(0, "-- -- --");
         }
     } 
     
     if(state == CHANGING){
         //needs to be able to change the combination after solving one I believe
     }
 
 }
 
 
 
  
 
 
 