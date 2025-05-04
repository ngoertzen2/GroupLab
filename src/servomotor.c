/**************************************************************************//**
 *
 * @file rotary-encoder.c
 *
 * @author Nick Goertzen
 * @author Nolan Hill
 *
 * @brief Code to control a servomotor.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "servomotor.h"
#include "interrupt_support.h"

#define SERVO_PIN           (22)
#define PULSE_INCREMENT_uS  (500)
#define SIGNAL_PERIOD_uS    (20000)

static int volatile pulse_width_us;
static int32_t next_rising_edge = 0;
static int32_t next_falling_edge = 0;
volatile cowpi_ioport_t *ioport = (cowpi_ioport_t *)(0xD0000000);

static void handle_timer_interrupt();

void initialize_servo() {
    cowpi_set_output_pins(1 << SERVO_PIN);
    center_servo();
    register_periodic_timer_ISR(0, PULSE_INCREMENT_uS, handle_timer_interrupt);
}

char *test_servo(char *buffer) {
    if (cowpi_left_button_is_pressed()) {
        center_servo();
    } else {
        if (cowpi_left_switch_is_in_left_position()) {
            rotate_full_clockwise();
        } else {
            rotate_full_counterclockwise();
        }
    }

    return buffer;
}

void center_servo() {
    pulse_width_us = 1500;
}

void rotate_full_clockwise() {
    pulse_width_us = 500;
}

void rotate_full_counterclockwise() {
    pulse_width_us = 2500;
}

static void handle_timer_interrupt() {
    next_rising_edge -= PULSE_INCREMENT_uS;
    next_falling_edge -= PULSE_INCREMENT_uS;
    if (next_rising_edge <= 0) {
        ioport->output |= 1 << 22;
        next_rising_edge = SIGNAL_PERIOD_uS;
        next_falling_edge = pulse_width_us;
    }
    if (next_falling_edge <= 0) {
        ioport->output &= ~(1 << 22);
    }
}
