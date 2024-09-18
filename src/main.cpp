#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include "U8g2lib.h"
#include <Wire.h> // library required for IIC communication
#include <esp32-hal.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// ultrasonic
#define trigger 12
#define echo 14
float TotalT = 0;
float Speed = 0.000343; // m/us
float Distance = 0;

// buzze
#define buzzer 19

// buttons
#define button_1 5
#define button_2 17
#define button_3 16

/////////////////////////////////////////////////////////// first page //////////////////////////////////////////////////////////////////////////
// 'itc logo', 16x16px
const unsigned char epd_bitmap_teams_logo[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x3c, 0xdc, 0x7b, 0xee, 0x77, 0xee, 0x77,
    0xee, 0x77, 0xec, 0x37, 0xd8, 0x1b, 0x9e, 0x79, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// 'BE logo', 16x16px
const unsigned char epd_bitmap_BE_logo[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x20, 0x04, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08,
    0x20, 0x04, 0xc0, 0x03, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x00, 0x00};
// 'teams logo', 16x16px
const unsigned char epd_bitmap_itc_logo[] PROGMEM = {
    0x00, 0x00, 0x10, 0x00, 0x30, 0x00, 0x00, 0x00, 0xfe, 0x1f, 0xfe, 0x3f, 0x30, 0x00, 0xb6, 0x01,
    0xb6, 0x01, 0xb6, 0x01, 0xb6, 0x01, 0xb6, 0x01, 0xb6, 0x01, 0xb6, 0x7f, 0xb6, 0x3f, 0x00, 0x00};
// 'event logo', 16x16px
const unsigned char epd_bitmap_event_logo[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x05, 0x50, 0x05, 0xa8, 0x0a, 0x04, 0x10, 0x54, 0x15,
    0x04, 0x10, 0x54, 0x15, 0x04, 0x10, 0x54, 0x15, 0x04, 0x10, 0x04, 0x10, 0xf8, 0x0f, 0x00, 0x00};
// 'games logo', 16x16px
const unsigned char epd_bitmap_games_logo[] PROGMEM = {
    0x00, 0x04, 0x00, 0x02, 0x00, 0x07, 0xf0, 0x0f, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x12, 0x48,
    0x3a, 0x54, 0x12, 0x48, 0x02, 0x40, 0x82, 0x41, 0x44, 0x22, 0x44, 0x22, 0x38, 0x1c, 0x00, 0x00};
// 'emergence logo', 16x16px
const unsigned char epd_bitmap_emergency_logo[] PROGMEM = {
    0x00, 0x00, 0x7c, 0x00, 0x42, 0x00, 0x42, 0x00, 0x42, 0x7f, 0x62, 0x41, 0x22, 0x49, 0x12, 0x5d,
    0x12, 0x49, 0x22, 0x41, 0x62, 0x7f, 0x42, 0x06, 0x42, 0x02, 0x42, 0x00, 0x7c, 0x00, 0x00, 0x00};

const unsigned char *bitmap_icons[6] = {
    epd_bitmap_itc_logo,
    epd_bitmap_BE_logo,
    epd_bitmap_teams_logo,
    epd_bitmap_event_logo,
    epd_bitmap_games_logo,
    epd_bitmap_emergency_logo};

// 'cursor points', 8x64px
const unsigned char epd_bitmap_cursor_points[] PROGMEM = {
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00,
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00,
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00,
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00};
// 'selection', 128x21px
const unsigned char epd_bitmap_selection[] PROGMEM = {
    0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01,
    0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
    0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

////////////////////////////////////////////////////// about itc ///////////////////////////////////////////////////////////////////////////////
// 'itc screen (1)', 128x64px
const unsigned char epd_bitmap_itc_screen__1_[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xdc, 0x87, 0x01, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x00, 0x48, 0x14, 0x00,
    0x00, 0x00, 0x08, 0x41, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02, 0x00,
    0x00, 0x00, 0x08, 0x41, 0x30, 0x8a, 0x92, 0x4c, 0x97, 0x80, 0xc1, 0x59, 0xc6, 0x5c, 0x92, 0x01,
    0x00, 0x00, 0x08, 0x41, 0x48, 0x55, 0x95, 0x52, 0x92, 0x40, 0x62, 0x44, 0x25, 0x49, 0x57, 0x00,
    0x00, 0x00, 0x08, 0x41, 0x4a, 0x51, 0x94, 0x52, 0x92, 0x40, 0x82, 0x45, 0x23, 0x49, 0x52, 0x00,
    0x00, 0x00, 0x1c, 0x81, 0x31, 0x51, 0xe4, 0x52, 0xe2, 0x84, 0xe3, 0x58, 0x2e, 0x49, 0x92, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x20, 0x00, 0x00, 0x80, 0x00, 0x44, 0x00, 0x63, 0x64, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x20, 0x00, 0x00, 0x80, 0x00, 0x04, 0x80, 0x94, 0x16, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x58, 0xe9, 0x60, 0x66, 0xcc, 0x19, 0x47, 0x06, 0xd4, 0x74, 0x38, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x29, 0x11, 0x51, 0x92, 0x94, 0x44, 0x09, 0xb3, 0x94, 0x90, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x29, 0x11, 0x31, 0x92, 0x8c, 0x44, 0x89, 0x90, 0x94, 0x90, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x58, 0xce, 0x60, 0xe1, 0x9c, 0x38, 0x43, 0x89, 0x67, 0x64, 0x10, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x10, 0x00, 0x00, 0x02, 0x08, 0x00, 0x80, 0x24, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x90, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
    0x20, 0x00, 0xb8, 0x63, 0xcc, 0x50, 0x8c, 0x89, 0x32, 0xce, 0x18, 0xa2, 0xee, 0x64, 0x00, 0x00,
    0x60, 0x00, 0x8c, 0x94, 0xa2, 0x70, 0x52, 0xaa, 0x2a, 0x29, 0x15, 0xaa, 0x24, 0x95, 0x00, 0x00,
    0x00, 0x00, 0xb0, 0x94, 0x62, 0x90, 0x52, 0xaa, 0x1a, 0x29, 0x0d, 0xaa, 0x24, 0x95, 0x00, 0x00,
    0xfc, 0x3f, 0x9c, 0xe4, 0xc2, 0x91, 0x92, 0x51, 0x72, 0xc6, 0x39, 0x94, 0x24, 0x95, 0x00, 0x00,
    0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6c, 0x03, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x28, 0x22, 0x00, 0x00, 0x08,
    0x6c, 0x03, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x11, 0x00, 0x00, 0x08,
    0x6c, 0x7f, 0x28, 0x46, 0x39, 0xc6, 0x1c, 0x19, 0x14, 0x63, 0x12, 0x2e, 0x11, 0x63, 0xc6, 0x1c,
    0x6c, 0x3f, 0x54, 0xa5, 0x4a, 0x25, 0x06, 0x25, 0xaa, 0x94, 0x12, 0xa9, 0xbb, 0x12, 0x25, 0x09,
    0x00, 0x00, 0x44, 0x23, 0x4a, 0x23, 0x18, 0x25, 0xa2, 0x94, 0x12, 0x29, 0x91, 0x11, 0x23, 0x09,
    0x00, 0x00, 0x44, 0x2e, 0x32, 0x2e, 0x0e, 0x25, 0x22, 0x97, 0x1c, 0x26, 0x11, 0x17, 0x2e, 0x09,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x50, 0x20, 0x04, 0x00, 0x10, 0x80, 0x04, 0x00, 0x00, 0x90, 0x28, 0x08, 0x04, 0x00,
    0x00, 0x00, 0x08, 0x20, 0x04, 0x00, 0x10, 0x80, 0x00, 0x00, 0x00, 0x10, 0x04, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x48, 0x26, 0xe7, 0x30, 0x73, 0xcc, 0x65, 0x0e, 0x18, 0xbb, 0x24, 0xcb, 0x04, 0x00,
    0x00, 0x00, 0x5c, 0xa5, 0x34, 0x88, 0x94, 0x92, 0x14, 0x03, 0xa4, 0x90, 0xae, 0x28, 0x05, 0x00,
    0x00, 0x00, 0x48, 0xa3, 0xc4, 0x88, 0x94, 0x92, 0x14, 0x0c, 0xa4, 0x90, 0xa4, 0x28, 0x05, 0x00,
    0x00, 0x00, 0x48, 0x2e, 0x73, 0x08, 0x63, 0x8c, 0x64, 0x47, 0xb8, 0x90, 0x24, 0xcb, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x04, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0xa1, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x81, 0x00, 0x00,
    0x00, 0x00, 0x64, 0xce, 0x94, 0x31, 0xc6, 0x98, 0x01, 0x91, 0x39, 0x40, 0x31, 0xa7, 0x0c, 0x00,
    0x00, 0x00, 0x94, 0xa4, 0x54, 0x49, 0x25, 0x45, 0x01, 0x55, 0x49, 0xa0, 0x4a, 0xa9, 0x0a, 0x00,
    0x00, 0x00, 0x94, 0x64, 0xd4, 0x48, 0x23, 0xc5, 0x00, 0xd5, 0x48, 0x20, 0x4a, 0xa9, 0x06, 0x00,
    0x00, 0x00, 0x94, 0xc4, 0x95, 0x73, 0x2e, 0x99, 0x13, 0x8a, 0x33, 0x22, 0x32, 0xa6, 0x1c, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// 'itc screen (2)', 128x64px
const unsigned char epd_bitmap_itc_screen__2_[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x9c, 0x29, 0x93, 0x31, 0x8a, 0x31, 0x07, 0x67, 0x5c, 0xc6, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x52, 0xa9, 0x52, 0x4a, 0x55, 0x49, 0x82, 0x54, 0x46, 0x29, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xd2, 0x90, 0x51, 0x4a, 0xd1, 0x48, 0x82, 0x34, 0x58, 0x29, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x8c, 0x13, 0x97, 0x39, 0x91, 0x4b, 0x22, 0xe3, 0x4e, 0x2e, 0x09, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x20, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00, 0x06, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x40, 0x02, 0x00, 0x01, 0x80, 0x04, 0x00, 0x41, 0x00, 0xa0, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x80, 0x04, 0x80, 0x00, 0x00, 0xa0, 0x00,
    0x00, 0x00, 0xdc, 0x99, 0x31, 0xe6, 0x8e, 0x31, 0x19, 0xc3, 0x1d, 0x83, 0x58, 0xc6, 0xb8, 0x12,
    0x00, 0x00, 0x86, 0x44, 0x49, 0x49, 0x52, 0x49, 0xa5, 0x84, 0xa4, 0xc2, 0x45, 0x25, 0xa5, 0x12,
    0x00, 0x00, 0x98, 0xc4, 0x48, 0x49, 0xd2, 0x48, 0xa5, 0x84, 0xa4, 0x81, 0x44, 0x23, 0xa5, 0x12,
    0x00, 0x00, 0x8e, 0x84, 0x4b, 0x4e, 0x92, 0x4b, 0x25, 0x87, 0x24, 0x87, 0x44, 0x2e, 0x99, 0x1c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x24, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x20, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00,
    0x20, 0x00, 0xcc, 0x64, 0x2e, 0x63, 0x1c, 0x48, 0x26, 0x61, 0xcc, 0x60, 0x4c, 0xc6, 0x70, 0x06,
    0x60, 0x00, 0xa2, 0x94, 0xa4, 0x94, 0x06, 0x70, 0x29, 0x91, 0xa2, 0x90, 0x52, 0x29, 0x21, 0x09,
    0x00, 0x00, 0x62, 0x94, 0xa4, 0x94, 0x18, 0x20, 0x29, 0x91, 0x62, 0x90, 0x52, 0x29, 0x21, 0x09,
    0xfc, 0x3f, 0xc2, 0xe5, 0x24, 0x93, 0x8e, 0x10, 0xc6, 0xe1, 0xc2, 0xe1, 0x4c, 0xc9, 0x21, 0x06,
    0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0xc0, 0x00, 0x00,
    0x6c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6c, 0x03, 0x02, 0x00, 0x00, 0x20, 0x20, 0x00, 0x02, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x04,
    0x6c, 0x03, 0x02, 0x00, 0x00, 0x20, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
    0x6c, 0x7f, 0x8e, 0x29, 0x83, 0x21, 0x73, 0x18, 0x41, 0x31, 0x8a, 0xb1, 0x8c, 0xc3, 0x18, 0x07,
    0x6c, 0x3f, 0x52, 0xaa, 0x42, 0xa2, 0x24, 0xa4, 0xa3, 0x2a, 0x55, 0x8a, 0xca, 0x20, 0xa5, 0x04,
    0x00, 0x00, 0x52, 0x92, 0x41, 0xa2, 0x24, 0x24, 0x21, 0x1a, 0x51, 0x8a, 0x06, 0x23, 0xa5, 0x04,
    0x00, 0x00, 0x92, 0x13, 0x87, 0x23, 0x23, 0x18, 0x21, 0x72, 0x91, 0x89, 0xdc, 0xc1, 0x25, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x48, 0x00, 0x02, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x09, 0x00,
    0x00, 0x00, 0x4c, 0x19, 0x63, 0x19, 0x63, 0xc6, 0xe1, 0x8c, 0xe1, 0x50, 0x72, 0x8e, 0x3b, 0x06,
    0x00, 0x00, 0x8a, 0xa4, 0x12, 0x95, 0x14, 0x65, 0x90, 0x52, 0x42, 0xa8, 0x1a, 0x03, 0x49, 0x05,
    0x00, 0x00, 0x46, 0xa5, 0x11, 0x8d, 0x14, 0x83, 0x91, 0x52, 0x42, 0x88, 0x62, 0x0c, 0x49, 0x03,
    0x00, 0x00, 0x5c, 0x1d, 0x17, 0xb9, 0x64, 0xee, 0x60, 0x4c, 0x42, 0x88, 0x3a, 0x07, 0x49, 0x0e,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x08, 0x04, 0xb8, 0xcf, 0x40, 0x00, 0x28, 0x00, 0x11,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x22, 0x21, 0x00, 0x20, 0x80, 0x2a,
    0x00, 0x00, 0xec, 0x18, 0x63, 0x86, 0x33, 0xc8, 0x64, 0x10, 0x22, 0x20, 0x46, 0xa9, 0x84, 0x2a,
    0x00, 0x00, 0x22, 0xa5, 0x14, 0x05, 0x49, 0x28, 0x95, 0x10, 0x22, 0x70, 0xa9, 0xaa, 0x04, 0x00,
    0x00, 0x00, 0x22, 0xa5, 0x14, 0x03, 0x49, 0x28, 0x95, 0x10, 0x22, 0x21, 0x29, 0xaa, 0x04, 0x00,
    0x00, 0x00, 0x2c, 0xb9, 0x64, 0x0e, 0x31, 0xc8, 0x94, 0x38, 0xc2, 0x20, 0x2e, 0x2a, 0x27, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

///////////////////////////////////////////////////////emergency//////////////////////////////////////////////////////////////////
// 'Nouveau projet', 128x64px
const unsigned char epd_bitmap_Nouveau_projet[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x09, 0x00, 0x90, 0x80, 0x90, 0x00, 0x00, 0x05, 0xc0, 0x01, 0x80, 0x03, 0x00, 0x28,
    0xe0, 0x0f, 0x09, 0x00, 0x80, 0x80, 0x94, 0x00, 0x00, 0x04, 0x40, 0x02, 0x80, 0x04, 0x00, 0x08,
    0x20, 0x08, 0xcf, 0x38, 0xd3, 0x99, 0xf0, 0x8c, 0x73, 0x1d, 0xc3, 0x31, 0x86, 0x63, 0x92, 0x29,
    0x20, 0x09, 0x29, 0x8d, 0x94, 0xa4, 0x90, 0xd2, 0x18, 0xa5, 0x44, 0x2a, 0x89, 0x94, 0x52, 0x2a,
    0xa0, 0x0b, 0x29, 0xb1, 0x94, 0xa4, 0x94, 0x12, 0x63, 0xa5, 0x44, 0x1a, 0x89, 0x94, 0x52, 0x2a,
    0x20, 0x09, 0xc9, 0x9c, 0x93, 0xb8, 0x90, 0xdc, 0x39, 0x19, 0x87, 0x71, 0x09, 0x63, 0x9c, 0x2b,
    0x20, 0x08, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x20, 0x08, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3c, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0xf3, 0x48, 0x87, 0x74, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x70, 0x1c, 0xa4, 0x14, 0x48, 0x88, 0x84, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x04, 0x34, 0x64, 0x48, 0x86, 0x64, 0x38, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x04, 0x2c, 0x83, 0x70, 0x08, 0x87, 0x48, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xa4, 0x90, 0x40, 0x08, 0x84, 0x48, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x98, 0x67, 0x40, 0x07, 0x74, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x06, 0x08, 0x20, 0x00, 0x00, 0x80, 0x07, 0x20, 0x80, 0x04, 0x00, 0x00, 0x00, 0x01,
    0xfc, 0x07, 0x09, 0x08, 0x20, 0x00, 0x00, 0x88, 0x00, 0x20, 0x80, 0x04, 0x00, 0x00, 0x00, 0x01,
    0x04, 0x19, 0x49, 0x39, 0x29, 0x63, 0xcc, 0x80, 0x93, 0x2c, 0xcc, 0x65, 0x1c, 0xce, 0x98, 0x33,
    0x04, 0x1f, 0xaf, 0x4a, 0xa9, 0x94, 0xa2, 0x80, 0x90, 0x22, 0x92, 0x94, 0x06, 0x23, 0x25, 0x29,
    0x04, 0x11, 0x29, 0x4a, 0xa9, 0x94, 0x62, 0x88, 0x90, 0x22, 0x92, 0x94, 0x18, 0x2c, 0x25, 0x19,
    0xfc, 0x1f, 0x29, 0x32, 0x2e, 0x97, 0xcc, 0x81, 0xe7, 0x22, 0x9c, 0xe4, 0x0e, 0xc7, 0x25, 0x71,
    0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x98, 0xf7, 0x0c, 0xc6, 0xf0, 0x0c, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x24, 0x84, 0x12, 0x29, 0x81, 0x12, 0x29, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x34, 0x42, 0x1a, 0x08, 0x41, 0x0c, 0xa8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x2c, 0x42, 0x16, 0xc6, 0x40, 0x12, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x24, 0x21, 0x12, 0x21, 0x20, 0x12, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x12, 0x18, 0x21, 0x0c, 0xef, 0x21, 0x0c, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x44, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x12, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x08, 0x2f, 0x00, 0x14, 0x08, 0x02, 0x00, 0x00, 0x25, 0x00, 0x20, 0x63, 0x04, 0x00, 0x00,
    0x04, 0x04, 0x01, 0x00, 0x02, 0x08, 0x02, 0x00, 0x88, 0x25, 0x00, 0xb0, 0x94, 0x06, 0x00, 0x00,
    0x02, 0x08, 0x27, 0x33, 0x92, 0x39, 0x67, 0xcc, 0x01, 0x25, 0x8c, 0xa1, 0x86, 0x04, 0x00, 0x00,
    0x04, 0x10, 0xa1, 0x28, 0x57, 0x4a, 0x52, 0x62, 0x00, 0x39, 0x52, 0xa0, 0x65, 0x04, 0x00, 0x00,
    0x08, 0x08, 0xa1, 0x18, 0x52, 0x4a, 0x32, 0x82, 0x09, 0x21, 0x52, 0xa0, 0x14, 0x04, 0x00, 0x00,
    0x10, 0x24, 0xa1, 0x70, 0x92, 0x4b, 0xe2, 0xe2, 0x00, 0x21, 0x4c, 0x20, 0xf3, 0x04, 0x00, 0x00,
    0xe0, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x03, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xfc, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc4, 0x11, 0x06, 0x14, 0x00, 0xa0, 0x07, 0x00, 0x64, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc8, 0x09, 0x09, 0x04, 0x00, 0x31, 0x04, 0x00, 0x96, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x48, 0x08, 0xc9, 0x94, 0x19, 0x20, 0x82, 0x31, 0xd4, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x04, 0x27, 0x55, 0x14, 0x20, 0x42, 0x0a, 0xb4, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0x07, 0x21, 0x55, 0x0c, 0x21, 0x41, 0x0a, 0x94, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x04, 0xc1, 0x94, 0x39, 0x20, 0x81, 0x09, 0x64, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/////////////////////////////////////////////////////// expressions //////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////// Games///////////////////////////////////////////////////////////////////////////

int b_up_clicked = 0; // only perform action when button is clicked, and wait until another press
int b_anter_clicked = 0;
int b_down_clicked = 0;

int current_screen = 0;
// screen 1
const int NUM_item = 6;
char menu_items[NUM_item][20]{
    {"itc"},
    {"BE members"},
    {"Teams"},
    {"Events"},
    {"Games"},
    {"Emergency"}};

int selected = 0;
int previous;
int next;

void setup()
{
  pinMode(button_1, INPUT_PULLUP);
  pinMode(button_2, INPUT_PULLUP);
  pinMode(button_3, INPUT_PULLUP);
  pinMode(buzzer, INPUT);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  u8g2.begin();
  u8g2.setBitmapMode(1); // screen mode
  Serial.begin(115200);
}
void loop()
{
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  TotalT = pulseIn(echo, HIGH);
  Distance = (TotalT * Speed) / 2;

  if (Distance >= 1)
  {
  }

  else
  {
    u8g2.setFont(u8g_font_7x14);
    u8g2.setColorIndex(1);
    if (current_screen == 0 || current_screen == 1)
    {
      if (digitalRead(button_3) == LOW && (b_up_clicked == 0))
      {
        tone(buzzer, 660, 20);
        selected = selected - 1;
        b_up_clicked = 1;
        if (selected < 0)
        {
          selected = NUM_item - 1;
        }
      }

      if (digitalRead(button_2) == LOW && (b_down_clicked == 0))
      {
        tone(buzzer, 600, 20);
        selected = selected + 1;
        b_down_clicked = 1;
        if (selected >= NUM_item)
        {
          selected = 0;
        }
      }

      if ((digitalRead(button_3) == HIGH) && (b_up_clicked == 1))
      { // unclick
        b_up_clicked = 0;
      }
      if ((digitalRead(button_2) == HIGH) && (b_down_clicked == 1))
      { // unclick
        b_down_clicked = 0;
      }
    }

    previous = selected - 1;
    if (previous < 0)
    {
      previous = NUM_item - 1;
    } // turn to the last
    next = selected + 1;
    if (next >= NUM_item)
    {
      next = 0;
    } // turn to the first

    if ((digitalRead(button_1) == LOW) && (b_anter_clicked == 0))
    { // select button clicked, jump between screens
      tone(buzzer, 523, 60);
      tone(buzzer, 659, 40);
      tone(buzzer, 900, 40);
      b_anter_clicked = 1; // set button to clicked to only perform the action once
      if (current_screen == 0)
      {
        current_screen = 1;
      } // menu items screen --> screenshots screen
      else if (current_screen == 1 && (bitmap_icons[selected] == epd_bitmap_teams_logo))
      {
        current_screen = 2;
      } // screenshots screen --> home with it being formation
      else
      {
        current_screen = 0;
      } // --> menu items screen
    }
    if ((digitalRead(button_1) == HIGH) && (b_anter_clicked == 1))
    { // unclick
      b_anter_clicked = 0;
    }

    u8g2.firstPage();
    do
    {
      if (current_screen == 0)
      {
        u8g2.setFont(u8g_font_7x14);
        u8g2.drawStr(25, 15, menu_items[previous]);
        u8g2.drawXBMP(4, 2, 16, 16, bitmap_icons[previous]);

        // selected
        u8g2.setFont(u8g_font_7x14B);
        u8g2.drawStr(25, 37, menu_items[selected]);
        u8g2.drawXBMP(4, 24, 16, 16, bitmap_icons[selected]);

        // next
        u8g2.setFont(u8g_font_7x14);
        u8g2.drawStr(25, 59, menu_items[next]);
        u8g2.drawXBMP(4, 46, 16, 16, bitmap_icons[next]);

        u8g2.drawXBMP(0, 22, 128, 21, epd_bitmap_selection);
        u8g2.drawXBMP(120, 0, 8, 64, epd_bitmap_cursor_points);
        u8g2.drawBox(125, 64 / 6 * selected, 8, 64 / 6); // 6 is the number of total items and selected is the number of the selected item
      }
      else if (current_screen == 1)
      {
        if (bitmap_icons[selected] == epd_bitmap_itc_logo)
        {
          u8g2.clearBuffer();
          u8g2.drawXBMP(0, 0, 128, 64, epd_bitmap_itc_screen__1_);
          u8g2.sendBuffer();
        }
        else if (bitmap_icons[selected] == epd_bitmap_emergency_logo)
        {
          u8g2.clearBuffer();
          u8g2.drawXBMP(0, 0, 128, 64, epd_bitmap_Nouveau_projet);
          u8g2.sendBuffer();
        }
        Serial.print("hello");
      }
      else
      {
        u8g2.clearBuffer();
        u8g2.drawXBMP(0, 0, 128, 64, epd_bitmap_itc_screen__2_);
        u8g2.sendBuffer();
        Serial.print("hi");
      }
      /*
      if(bitmap_screenshots[selected] == epd_bitmap_ones_at_the_moment){
        u8g2.setFont(u8g_font_7x14);
        u8g2.drawStr(3, 21, menu_items_[previous_]);

        //selected
        u8g2.setFont(u8g_font_7x14B);
        u8g2.drawStr(3, 40, menu_items_[selected_]);

        //next
        u8g2.setFont(u8g_font_7x14);
        u8g2.drawStr(3, 58, menu_items_[next_]);

        //u8g2.drawBitmapP( 0, 28, 128/8, 16, epd_bitmap_selection_2); //selection
        //u8g2.drawBitmapP( 0, 0, 128/8, 9, epd_bitmap_ones_at_the_moment_1);

      }
      else{
        //u8g2.drawBitmapP( 0, 0, 128/8, 64, bitmap_screenshots[selected]); // draw screenshot
        u8g2.drawXBMP(0, 0, 128, 64, bitmap_screenshots[selected]);
      }
      */

    } while (u8g2.nextPage());
  }
  /*
  else{u8g2.firstPage(); // Start clearing the first page
    do {
      u8g2.setColorIndex(0); // Set the color index to 0 (clear)
      u8g2.drawBox(0, 0, u8g2.getWidth(), u8g2.getHeight()); // Draw a box covering the entire display to clear it
    } while (u8g2.nextPage()); // Continue to the next page if available
  }
  */

  delay(10); // this speeds up the simulation
}
