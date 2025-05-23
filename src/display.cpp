/**************************************************************************//**
 *
 * @file display.cpp
 *
 * @author Christopher A. Bohn
 *
 * @brief  @copybrief display.h
 *
 * @copydetails display.h
 *
 ******************************************************************************/

/* SSD1306 display functions (c) 2024 Christopher A. Bohn
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <CowPi.h>
#include <CowPi_stdio.h>
#include <stdlib.h>
#include "display.h"

#if __has_include(<OneBitDisplay.h>)
#define ONEBIT
#include <OneBitDisplay.h>
#endif
#if __has_include(<Adafruit_SSD1306.h>)
#define ADAFRUITSSD1306
#include <Adafruit_SSD1306.h>
#endif
#if defined (ONEBIT) && defined (ADAFRUITSSD1306)
#warning "Both the OneBitDisplay and the Adafruit_SSD1306 libraries have been imported."
#elif !defined (ONEBIT) && !defined (ADAFRUITSSD1306)
#error "Neither the OneBitDisplay library nor the Adafruit_SSD1306 library has been imported."
#endif

#if defined (__AVR__)
#define CORELIBRARY ("avr-libc")
#elif defined (__MBED__)
#define CORELIBRARY ("MBED")
#elif defined(ARDUINO_ARCH_RP2040) && !defined(__MBED__)
#define CORELIBRARY ("PicoSDK")
#else
#define CORELIBRARY ("unknown")
#endif


static int column_count;
static int row_count;
static int character_width;
static int character_height;

static inline void library_specific_initialize_display(int number_of_columns);

static char rows[8][23] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};

#if defined ONEBIT

static uint8_t logo[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x07, 0x07, 0x0f, 0x1f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x1f, 0x0f, 0x07, 0x07, 0x0f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x83, 0x3b, 0xf9, 0xfd, 0xfd, 0xfe, 0x1e, 0x1e, 0x1e, 0x1e, 0x1c, 0x3d, 0x3d, 0x79,
        0xfb, 0xf3, 0x77, 0x2f, 0x0f, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x1c, 0x1d, 0x3d, 0x7d, 0xfd,
        0xfd, 0xfc, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfc, 0xf8, 0xf8, 0xf8, 0xf0, 0xf0, 0xf0, 0xef, 0xef,
        0xf7, 0xf3, 0xfb, 0xf9, 0x3d, 0x3d, 0x1c, 0x1e, 0x1e, 0x1e, 0x1e, 0xfe, 0xfd, 0xfd, 0xf9, 0x3b,
        0x83, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x1f,
        0x1f, 0x0f, 0x0f, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xe7, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xfe, 0xf9, 0xf3, 0xef, 0xcf, 0xdf, 0xbe, 0xbc, 0x3c, 0x38, 0x38, 0x3c, 0x1e,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xfe, 0x3c, 0x38, 0x38, 0x3c, 0xbc, 0xbe, 0xdf, 0xdf, 0xef, 0xf3, 0xf9, 0xfe,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xe1, 0xe0, 0xf0, 0xf8, 0xf8,
        0xfc, 0xfe, 0xfe, 0xfe, 0xfe, 0xff, 0x3f, 0x02, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfe, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x3f, 0x3f, 0x3f, 0x1e, 0x00, 0x00, 0x80, 0xc0,
        0xe0, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xc0, 0xc0, 0xc0, 0xe1, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x7f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfc, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x03, 0x80, 0xc0,
        0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf0, 0xf6, 0xf7, 0xe7, 0xe7,
        0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xf7, 0xf7, 0xf7, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb,
        0xf3, 0xf7, 0xe7, 0xcf, 0x98, 0x01, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfe, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xf8, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xc3, 0x83, 0x83, 0x07, 0x0f, 0x1f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x0f, 0x07, 0x87, 0x83, 0xc3, 0xc7, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
        0x0f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xff, 0xff, 0xff, 0xff,
        0x7f, 0x7f, 0x3f, 0x1f, 0x1f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xf1, 0xcf, 0x9f, 0x3f,
        0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x7f, 0x3f, 0x9f, 0xcf, 0xf3, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xe1, 0xe0, 0xe0,
        0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xfe, 0xf8, 0xf0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xf0,
        0xf0, 0xf8, 0xfc, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfe, 0xfe, 0xfc, 0xfd, 0xf9, 0xfb, 0xfb, 0xf3, 0xf7, 0xf7, 0xf7, 0xf7, 0xe7, 0xe7, 0xef, 0xef,
        0xef, 0xef, 0xef, 0xef, 0xef, 0xe7, 0xe7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf3, 0xfb, 0xfb, 0xf9, 0xfd,
        0xfc, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint8_t backbuffer[1024] = {0};
static OBDISP display;
static int font;

static inline void library_specific_initialize_display(int number_of_columns) {
    obdI2CInit(&display, OLED_128x64, -1, 0, 0, 1, -1, -1, -1, 400000L);
    obdSetBackBuffer(&display, backbuffer);
    switch (number_of_columns) {
        case 21:
            font = FONT_6x8;
            break;
        case 16:
            font = FONT_8x8;
            break;
        case 10:
            font = FONT_12x16;
            break;
        case 8:
            font = FONT_16x16;
            break;
        default:
            fprintf(stderr, "no font available");
    }
}

void clear_display(void) {
    obdFill(&display, OBD_WHITE, 0);
    refresh_display();
}

void draw_logo() {
    memcpy(backbuffer, logo, 1024);
    refresh_display();
}

void refresh_display(void) {
    for (int row = 0; row < row_count; ++row) {
        obdWriteString(&display, 0, 0, character_height * row, (char *) rows[row], font, OBD_BLACK, 0);
    }
    obdDumpBuffer(&display, backbuffer);
}


#elif defined ADAFRUITSSD1306

static uint8_t logo[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xfc, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf8, 0x7f, 0xff, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf8, 0x3f, 0xff, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf8, 0x1f, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf8, 0x1f, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xfe, 0x07, 0xf8, 0x0f, 0xbe, 0x03, 0xfc, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xf1, 0xf0, 0xf8, 0x00, 0x00, 0x03, 0xe1, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xc7, 0xfe, 0x38, 0x1f, 0xff, 0x03, 0x8f, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xdf, 0xff, 0x98, 0xff, 0xff, 0xe3, 0x3f, 0xff, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07,
        0xdf, 0xff, 0xe0, 0x1f, 0xff, 0xfc, 0xff, 0xff, 0x7f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07,
        0xdf, 0x07, 0xf0, 0x07, 0xff, 0xff, 0xfc, 0x1f, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
        0xcf, 0x01, 0xe0, 0x03, 0xff, 0xff, 0xf0, 0x1e, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
        0xef, 0x00, 0xc0, 0x01, 0xff, 0xff, 0xf0, 0x1e, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
        0xef, 0x80, 0x80, 0x00, 0xff, 0xff, 0xe0, 0x3e, 0xff, 0xf0, 0x06, 0x03, 0xff, 0xc0, 0x7f, 0xff,
        0xf7, 0xc1, 0x80, 0x00, 0xff, 0xff, 0xf0, 0x7d, 0xff, 0xe0, 0x7f, 0x07, 0xff, 0xe0, 0x7f, 0xff,
        0xf3, 0xf3, 0x00, 0x00, 0x7f, 0xff, 0xf9, 0xf9, 0xff, 0xc0, 0xfe, 0x07, 0xff, 0xc0, 0x7f, 0xff,
        0xfb, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfb, 0xff, 0xc3, 0xfe, 0x07, 0xff, 0xc0, 0x7f, 0xff,
        0xfc, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xf7, 0xff, 0xe7, 0xfe, 0x07, 0xff, 0x80, 0xff, 0xff,
        0xfe, 0x7e, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0x80, 0xff, 0xff,
        0xff, 0x80, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0x80, 0xff, 0xff,
        0xff, 0xe0, 0x00, 0x00, 0x3f, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0x80, 0xff, 0xff,
        0xff, 0xfc, 0x00, 0xe0, 0x3f, 0x1f, 0xf7, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0x01, 0xff, 0xff,
        0xff, 0xfc, 0x01, 0xf0, 0x3e, 0x0f, 0xf7, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0x01, 0xff, 0xff,
        0xff, 0xfc, 0x01, 0xf0, 0x7e, 0x0f, 0xf7, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0x01, 0xff, 0xff,
        0xff, 0xfc, 0x01, 0xf0, 0x7e, 0x0f, 0xf7, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0x01, 0xff, 0xff,
        0xff, 0xfc, 0x01, 0xf0, 0x7e, 0x0f, 0xf7, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xfe, 0x03, 0xff, 0xff,
        0xff, 0xfc, 0x00, 0xe0, 0xff, 0x1f, 0xf7, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xfe, 0x03, 0xff, 0xff,
        0xff, 0xfc, 0x00, 0x01, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xfe, 0x03, 0xff, 0xff,
        0xff, 0xfc, 0x00, 0x03, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xfe, 0x03, 0xff, 0xff,
        0xff, 0xfc, 0x00, 0x07, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xfc, 0x03, 0xff, 0xff,
        0xff, 0xfc, 0x00, 0x0f, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xfc, 0x07, 0xff, 0xff,
        0xff, 0xf8, 0x00, 0x0f, 0xff, 0x00, 0x73, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xfc, 0x07, 0xff, 0xff,
        0xff, 0xf8, 0x1f, 0xe0, 0x00, 0xff, 0x1b, 0xff, 0xff, 0xff, 0xe0, 0x7f, 0xfc, 0x07, 0xff, 0xff,
        0xff, 0xf8, 0x7f, 0xfc, 0x07, 0xff, 0xcb, 0xff, 0xff, 0xff, 0xe0, 0x7f, 0xf8, 0x07, 0xff, 0xff,
        0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xe0, 0x7f, 0xf8, 0x0f, 0xff, 0xff,
        0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xf8, 0x0f, 0xff, 0xff,
        0xff, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xf8, 0x0f, 0xff, 0xff,
        0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xf8, 0x0f, 0xff, 0xff,
        0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0x80, 0xff, 0xf0, 0x0f, 0xff, 0xff,
        0xff, 0xe7, 0xfe, 0x3f, 0xff, 0xcf, 0xfe, 0xff, 0xff, 0xff, 0x80, 0xff, 0xf0, 0x1f, 0xff, 0xff,
        0xff, 0xef, 0xfc, 0x1f, 0xff, 0x07, 0xfe, 0xff, 0xff, 0xff, 0x80, 0xff, 0xf0, 0x1f, 0xff, 0xff,
        0xff, 0xef, 0xfc, 0x0f, 0xfe, 0x07, 0xfe, 0xff, 0xff, 0xff, 0x01, 0xff, 0xf0, 0x1f, 0xff, 0xff,
        0xff, 0xef, 0xfe, 0x07, 0xfc, 0x07, 0xfe, 0xff, 0xff, 0xff, 0x01, 0xff, 0xf0, 0x1f, 0xe7, 0xff,
        0xff, 0xef, 0xff, 0x07, 0xfc, 0x1f, 0xfe, 0xff, 0xff, 0xfe, 0x01, 0xff, 0xf0, 0x1f, 0xc3, 0xff,
        0xff, 0xef, 0xff, 0xc7, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xfe, 0x01, 0xff, 0xf0, 0x0f, 0x07, 0xff,
        0xff, 0xef, 0xff, 0xef, 0xfe, 0xff, 0xfe, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xf0, 0x00, 0x07, 0xff,
        0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xf8, 0x00, 0x1f, 0xff,
        0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xf8, 0x00, 0x3f, 0xff,
        0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xfc, 0x00, 0x7f, 0xff,
        0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xfe, 0x01, 0xff, 0xff,
        0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xc7, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf0, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xfe, 0x03, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static Adafruit_SSD1306 display(128, 64);

static inline void library_specific_initialize_display(int number_of_columns) {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextSize((number_of_columns <= 10) ? 2 : 1);
    display.setTextColor(SSD1306_WHITE);
}

void clear_display(void) {
    display.clearDisplay();
    refresh_display();
}

void draw_logo() {
    display.drawBitmap(0, 0, logo, 128, 64, 1);
    display.display();
}

void refresh_display(void) {
    display.clearDisplay();
    for (int row = 0; row < row_count; ++row) {
        display.setCursor((int16_t) ((128 - (character_width * column_count)) / 2), (int16_t) (character_height * row));
        display.print(rows[row]);
    }
    display.display();
}


#endif


void initialize_display(int number_of_columns) {
    record_build_timestamp(__FILE__, __DATE__, __TIME__);
    if ((number_of_columns != 8) && (number_of_columns != 10) && (number_of_columns != 16) && (number_of_columns != 21)) {
        fprintf(stderr, "number of columns cannot be %d.\n", number_of_columns);
    }
    column_count = number_of_columns;
    row_count = (number_of_columns <= 10) ? 4 : 8;
    character_width = (number_of_columns <= 10) ? 12 : 6;
    character_height = (number_of_columns <= 10) ? 16 : 8;
    library_specific_initialize_display(number_of_columns);
    clear_display();
}

void display_string(int row, char const string[]) {
    static char buffer[23] = {"                      "};
    size_t string_length = strlen(string);
    if (0 <= row && row < row_count) {
        sprintf(buffer, "%-*s", column_count, string);
    }
    if (string[string_length - 1] == '\n') {
        buffer[string_length - 1] = '\0';
        memcpy(rows[row], buffer, 21);
        refresh_display();
    } else {
        memcpy(rows[row], buffer, 21);
    }
}


void print_versions(void) {
    char message[22];
    if (column_count >= 16) {
        sprintf(message, "gcc %*d.%d", column_count - 8, __GNUC__, __GNUC_MINOR__);
        display_string(0, message);
        sprintf(message, "Core %*s", column_count - 5, CORELIBRARY);
        display_string(1, message);
        sprintf(message, "CowPi %*s", column_count - 6, COWPI_VERSION);
        display_string(2, message);
        sprintf(message, "CowPi_stdio%*s", column_count - 11, COWPI_STDIO_VERSION);
        display_string(3, message);
        refresh_display();
    } else {
        sprintf(message, "gcc%*d", column_count - 3, __GNUC__);
        display_string(0, message);
        sprintf(message, "CowPi%*.*s", column_count - 5, column_count - 5, COWPI_VERSION);
        display_string(1, message);
        sprintf(message, "stdio%*.*s", column_count - 5, column_count - 5, COWPI_STDIO_VERSION);
        display_string(2, message);
        refresh_display();
    }
}

struct build_timestamp {
    char filename[22];
    char date[9];
    char time[7];
};

static struct build_timestamp records[8] = {{}, {}, {}, {}, {}, {}, {}, {}};
static int number_of_records = 0;

static int compare_build_timestamps(void const *a, void const *b) {
    struct build_timestamp const *r1 = (struct build_timestamp const *) a;
    struct build_timestamp const *r2 = (struct build_timestamp const *) b;
    // if the dates are different, then the latest date is the most recent build
    int comparison = strcmp(r2->date, r1->date);
    if (comparison) return comparison;
    // if the dates are the same but the times are different, then the latest time is the most recent build
    comparison = strcmp(r2->time, r1->time);
    if (comparison) return comparison;
    // if the dates and times are the same, then we'll go alphabetically
    return strcmp(r1->filename, r2->filename);
}

void record_build_timestamp(const char *filename, const char *date, const char *time) {
    int month;
    if (!strncmp("Jan", date, 3)) month = 1;
    else if (!strncmp("Feb", date, 3)) month = 2;
    else if (!strncmp("Mar", date, 3)) month = 3;
    else if (!strncmp("Apr", date, 3)) month = 4;
    else if (!strncmp("May", date, 3)) month = 5;
    else if (!strncmp("Jun", date, 3)) month = 6;
    else if (!strncmp("Jul", date, 3)) month = 7;
    else if (!strncmp("Aug", date, 3)) month = 8;
    else if (!strncmp("Sep", date, 3)) month = 9;
    else if (!strncmp("Oct", date, 3)) month = 10;
    else if (!strncmp("Nov", date, 3)) month = 11;
    else if (!strncmp("Dec", date, 3)) month = 12;
    else month = 99;
    if (!strncmp(filename, "src/", 4)) {
        strncpy(records[number_of_records].filename, filename + 4, 22);
    } else {
        strncpy(records[number_of_records].filename, filename, 22);
    }
    strncpy(records[number_of_records].date, date + 7, 4);                  // year
    sprintf(records[number_of_records].date + 4, "%02d", month);            // month
    records[number_of_records].date[6] = date[4] == ' ' ? '0' : date[4];    // day (tens place)
    records[number_of_records].date[7] = date[5];                           // day (ones place)
    records[number_of_records].date[8] = '\0';
    int i = -1;
    int number_of_skipped_characters = 0;
    char c;
    do {
        c = time[++i];
        if (c == ':') {
            number_of_skipped_characters++;
        } else {
            records[number_of_records].time[i - number_of_skipped_characters] = c;
        }
    } while (c);
    number_of_records++;
}

void print_build_timestamps(bool only_most_recent) {
    // sort the records, with the most-recent timestamp first
    qsort(records, number_of_records, sizeof(struct build_timestamp), compare_build_timestamps);
    char timestamp[17];
    if (only_most_recent) {
        switch (column_count) {
            case 16:
            case 21:
//                sprintf(timestamp, "%8.8s/%6.6s\n", records[0].date, records[0].time);
                sprintf(timestamp, "%8.8s/%4.4s\n", records[0].date, records[0].time);
                break;
            case 10:
                sprintf(timestamp, "%6.6s%4.4s\n", records[0].date + 2, records[0].time);
                break;
            case 8:
                sprintf(timestamp, "%4.4s%4.4s\n", records[0].date + 4, records[0].time);
                break;
            default:
                sprintf(timestamp, "ERROR");
        }
        display_string(row_count - 1, timestamp);
    } else {
        for (int i = 0; i < min(number_of_records, row_count); i++) {
            switch (column_count) {
                case 16:
                case 21:
                    sprintf(timestamp, "%-*.*s%6s",
                            column_count - 6, column_count - 6,
                            records[i].filename, records[i].time);
                    break;
                case 10:
                case 8:
                    sprintf(timestamp, "%-*.*s%4.4s",
                            column_count - 4, column_count - 4,
                            records[i].filename, records[i].time);
                    break;
                default:
                    sprintf(timestamp, "ERROR");
            }
            display_string(i, timestamp);
        }
    }
    refresh_display();
}

// TODO: this will break without the rows being partially exposed
void count_visits(int row) {
    static uint8_t counters[8] = {0};
    int counter_position = column_count - 2;
    for (int i = 0; i < counter_position; i++) {
        if (!rows[row][i]) {
            rows[row][i] = ' ';
        }
    }
    sprintf(rows[row] + counter_position, "%02X", ++counters[row]);
    refresh_display();
}
