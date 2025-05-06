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
static direction_t correct_direction;
static uint8_t combination[3] __attribute__((section (".uninitialized_ram.")));
static int bad_tries = 0;
static uint8_t entered_combination[3];
static int visible_counts[3] = {0, 0, 0};
static uint8_t current_number = 0;
static int entry_stage = 0;
static bool handle_keypress = false;
static cowpi_timer_t volatile *timer;
int digit_index = 0;
uint8_t new_combination[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
key_t handle_key;

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
        && visible_counts[1] == 2
        && visible_counts[2] == 1;
}

uint32_t get_microseconds(void) {
    return timer->raw_lower_word;
}
 
void blink_leds(){
    for (int i = 0; i < 2; i++) {
        cowpi_illuminate_left_led();
        cowpi_illuminate_right_led();
        uint32_t start_time = get_microseconds();
        while ((get_microseconds() - start_time) < 250000) {}
        cowpi_deluminate_left_led();
        cowpi_deluminate_right_led();
        start_time = get_microseconds();
        while ((get_microseconds() - start_time) < 250000) {}

    }
}

void clear_combination() {
    entry_stage = 0;
    current_number = 0;
    correct_direction = CLOCKWISE;
    for (int i = 0; i < 3; i++) {
        entered_combination[i] = 0xFF;
        visible_counts[i] = 0;
    }
}

void display_combination() {
    char display[32] = "";
    for (int i = 0; i < 3; i++) {
        char temp[5];
        if (i == entry_stage) {
            sprintf(temp, "%02d", current_number);
            strcat(display, temp);
        } else if (entered_combination[i] != 0xFF) {
            sprintf(temp, "%02d", entered_combination[i]);
            strcat(display, temp);       
        } else {
            strcat(display, "  ");
        }
        if (i < 2) {
            strcat(display, "-");
        }
    }
    display_string(1, display);
}
 
void initialize_lock_controller() {
    state = LOCKED;
    entry_stage = 0;
    correct_direction = CLOCKWISE;
    bad_tries = 0;
    timer = (cowpi_timer_t *) (0x40054000);
    for (int i = 0; i < 3; i++) {
        entered_combination[i] = 0xFF;
        visible_counts[i] = 0;
    }
    current_number = 0;
    cowpi_illuminate_left_led();
    cowpi_deluminate_right_led();
    rotate_full_clockwise();
    display_combination();
}

void control_lock() {
    direction_t dir = get_direction();
    char result[16];

    if (state == ALARMED) {
        while (true) {
            blink_leds();
        }
    } else if (state == LOCKED) {
        
        if (entry_stage < 3) {
            if (dir == correct_direction) {
                int dir_num = (correct_direction == CLOCKWISE) ? 1 : -1;
                if (current_number + dir_num == combination[entry_stage]) {
                    visible_counts[entry_stage]++;
                }  
                if (current_number < 15 && current_number > 0) {
                    (correct_direction == CLOCKWISE) ? current_number++ : current_number--;
                } else if (dir == CLOCKWISE) {
                    if (current_number == 0) {
                        current_number++;
                    } else {
                        current_number = 0;
                    }
                } else if (dir == COUNTERCLOCKWISE) {
                    if (current_number == 15) {
                        current_number--;
                    } else {
                        current_number = 15;
                    }
                } 
            } else if (dir != correct_direction && dir != STATIONARY) {
                entry_stage++;
                correct_direction = (correct_direction == CLOCKWISE) ? COUNTERCLOCKWISE : CLOCKWISE;
            }
            entered_combination[entry_stage] = current_number;
        } 
        if (entry_stage > 1) {
            if (dir == COUNTERCLOCKWISE) {
                clear_combination();
            }
            if (cowpi_left_button_is_pressed()) {
                if (check_combination()) {
                    state = UNLOCKED;
                } else {
                    bad_tries++;
                    if (bad_tries >= 3) {
                        state = ALARMED;
                    } else{
                        clear_combination();
                        blink_leds();
                    }
                }
            }
        }
    } else if(state == UNLOCKED) {
        if (cowpi_left_switch_is_in_right_position() && cowpi_right_button_is_pressed()) {
            state = CHANGING;
        }
        if (cowpi_left_button_is_pressed() && cowpi_right_button_is_pressed()) {
            clear_combination();
            bad_tries = 0;
            display_string(0, "");
            state = LOCKED;
        }
    } else if(state == CHANGING) {
        char combo_display[32] = "";
        char part[6];
        for (int i = 0; i < 6; i++) {
            if (i == 3) {
                strcat(combo_display, "    ");
            }
            if (new_combination[i] == 0xFF) {
                strcat(combo_display, "  ");
            } else {
                sprintf(part, "%02d", new_combination[i]);
                strcat(combo_display, part);
            }
            
            strcat(combo_display, (i < 5 && i != 2) ? "-" : " ");
        }

        key_t key = cowpi_get_keypress();
        
        if (key >= '0' && key <= '9' && digit_index < 6 && !handle_keypress) {
            handle_key = key - '0';
            handle_keypress = true;
        }
        if (handle_keypress && key == NULL) {
            if (new_combination[digit_index] == 0xFF) {
                new_combination[digit_index] = 0;
                new_combination[digit_index] += 10 * handle_key;
            } else {
                new_combination[digit_index] += handle_key;
                digit_index++;
            }
            handle_keypress = false;
        }
        if (cowpi_left_switch_is_in_left_position()) {
            bool incomplete = false;
            bool mismatch = false;
            bool out_of_bounds = false;

            for (int i = 0; i < 3; i++) {
                if (new_combination[i] == 0xFF || new_combination[i + 3] == 0xFF) {
                    incomplete = true;
                }
                if (new_combination[i] != new_combination[i + 3]) {
                    mismatch = true;
                }
                if (new_combination[i] > 15 || new_combination[i + 3] > 15) {
                    out_of_bounds = true;
                }
            }

            if (incomplete || mismatch || out_of_bounds) {
                display_string(2, "no change");
            } else {
                for (int i = 0; i < 3; i++) {
                    combination[i] = new_combination[i];
                }
                display_string(2, "changed");
            }
            state = UNLOCKED;
            digit_index = 0;
            for (int i = 0; i < 6 ; i++) {
                new_combination[i] = 0xFF;
            }
        }
        display_string(1, combo_display);
    }
    if (state == LOCKED) {
        cowpi_illuminate_left_led();
        cowpi_deluminate_right_led();
        rotate_full_clockwise();
        if (bad_tries > 0) {
            char temp[19];
            sprintf(temp, "%s%d", "bad try ", bad_tries);
            display_string(0, temp);
        }
        display_string(2, " ");
        display_combination();
    } else if (state == UNLOCKED) {
        cowpi_deluminate_left_led();
        cowpi_illuminate_right_led();
        rotate_full_counterclockwise();
        display_string(0, "OPEN");
        display_string(1, " ");
    } else if (state == ALARMED) {
        display_string(0, "alert!");
        display_string(1, " ");
    } else if (state == CHANGING) {
        display_string(0, "enter");
    }
    
}
 
 
 
  
 
 
 