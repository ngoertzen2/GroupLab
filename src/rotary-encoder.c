/**************************************************************************//**
 *
 * @file rotary-encoder.c
 *
 * @author (STUDENTS -- Nolan Hill)
 * @author (STUDENTS -- TYPE YOUR NAME HERE)
 *
 * @brief Code to determine the direction that a rotary encoder is turning.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

 #include <CowPi.h>
 #include "interrupt_support.h"
 #include "rotary-encoder.h"
 
 #define A_WIPER_PIN         (16)
 #define B_WIPER_PIN         (A_WIPER_PIN + 1)
 
 typedef enum {
     HIGH_HIGH, HIGH_LOW, LOW_LOW, LOW_HIGH, UNKNOWN
 } rotation_state_t;
 
 static rotation_state_t volatile state;
 static direction_t volatile direction = STATIONARY;
 static int volatile clockwise_count = 0;
 static int volatile counterclockwise_count = 0;
 
 static cowpi_ioport_t volatile *ioport;
 
 static void handle_quadrature_interrupt();
 
 void initialize_rotary_encoder() {
     cowpi_set_pullup_input_pins((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN));
     uint32_t quadrature = get_quadrature(); 
     direction = STATIONARY;
 
     switch (quadrature) {
         case 0b11: 
             state = HIGH_HIGH; 
             break;
         case 0b10: 
             state = HIGH_LOW; 
             break;
         case 0b00: 
             state = LOW_LOW; 
             break;
         case 0b01: 
             state = LOW_HIGH; 
             break;
         default:   
             state = UNKNOWN; 
             break;
     }
 
     register_pin_ISR((1 << A_WIPER_PIN) | (1 << B_WIPER_PIN), handle_quadrature_interrupt);
 }
 
 uint8_t get_quadrature() {
     ioport = (cowpi_ioport_t *)(0xD0000000);
 
     uint32_t quadrature = ioport->input & (0b11 << 16);
     return (quadrature >> 16);
 }
 
 char *count_rotations(char *buffer) {
     sprintf(buffer, "CW:%d CCW:%d", clockwise_count, counterclockwise_count);
     return buffer;
 }
 
 direction_t get_direction() {
     direction_t result = direction;
     direction = STATIONARY;
     return result;
 }
 
 static void handle_quadrature_interrupt() {
     static rotation_state_t last_state = UNKNOWN;
     rotation_state_t previous_state = state;
     uint8_t quadrature = get_quadrature();
     
 
     switch (quadrature) {
         case 0b11:
             state = HIGH_HIGH;
             break;
 
         case 0b10:
             state = HIGH_LOW;
             break;
 
         case 0b00:
             if ((previous_state == HIGH_LOW && last_state == HIGH_HIGH)) {
                 clockwise_count++;
                 direction = CLOCKWISE;
                 state = LOW_LOW;
             } else if ((previous_state == LOW_HIGH && last_state == HIGH_HIGH)) {
                 counterclockwise_count++;
                 direction = COUNTERCLOCKWISE;
                 state = LOW_LOW;
             }
             break;
 
         case 0b01:
             state = LOW_HIGH;
             break;
 
         default:
             state = UNKNOWN;
             break;
     }
 
     last_state = previous_state;
 }