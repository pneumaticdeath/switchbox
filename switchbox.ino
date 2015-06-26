#include <Adafruit_NeoPixel.h>
#include "SwitchButton.h"
#include "pitches.h"

#define NEOPIXEL_PIN 6
#define NUM_PIXELS 16

#define SW1_PIN 5
#define SW2_PIN 4

#define REDBTN_OUT 13
#define REDBTN_IN  12
#define GRNBTN_OUT 11
#define GRNBTN_IN  10
#define BLUBTN_OUT  9
#define BLUBTN_IN   8

#define BIGBTN_IN   7

#define SPKR_PIN    2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
int curr_mode = -1;
uint32_t btn_debounce_timer = 0;
#define DEBOUNCE_DELAY 50
#define LONGPRESS_TIME 1000
#define BEEP_TIME 250

SwitchButton red_button = SwitchButton(REDBTN_IN, LONGPRESS_TIME, DEBOUNCE_DELAY, SPKR_PIN, NOTE_C3, BEEP_TIME);
SwitchButton grn_button = SwitchButton(GRNBTN_IN, LONGPRESS_TIME, DEBOUNCE_DELAY, SPKR_PIN, NOTE_D3, BEEP_TIME);
SwitchButton blu_button = SwitchButton(BLUBTN_IN, LONGPRESS_TIME, DEBOUNCE_DELAY, SPKR_PIN, NOTE_E3, BEEP_TIME);
SwitchButton big_button = SwitchButton(BIGBTN_IN, LONGPRESS_TIME, DEBOUNCE_DELAY, SPKR_PIN, NOTE_F3, BEEP_TIME);

SwitchButton sw1 = SwitchButton(SW1_PIN, LONGPRESS_TIME, DEBOUNCE_DELAY, SPKR_PIN, NOTE_C2, BEEP_TIME, NOTE_E2, BEEP_TIME);
SwitchButton sw2 = SwitchButton(SW2_PIN, LONGPRESS_TIME, DEBOUNCE_DELAY, SPKR_PIN, NOTE_D2, BEEP_TIME, NOTE_F2, BEEP_TIME);

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

typedef enum
{
  rainbow_wheel,
  rainbow_chase,
  rotating_chase,
  color_chase,
  rainbow_wipe,
  rotating_wipe,
  color_wipe,
  chaser,
  erase
} animation_t ;

animation_t animation_mode;

uint32_t next_animation_time = 0;
uint16_t anim_i, anim_j, anim_k;
int16_t dir_r, dir_g, dir_b;
uint8_t pos_r, pos_g, pos_b;
uint32_t animation_target_color = 0;
uint16_t wipe_animation_delay = 25;
uint16_t chase_animation_delay = 50;
uint16_t rainbow_wheel_speed = 1;
uint16_t red_blink_interval, grn_blink_interval, blu_blink_interval, blink_counter;

void animation_init() {
  anim_i = anim_j = anim_k = 0;
  dir_r = dir_g = dir_b = 5;
  pos_r = 0;
  pos_g = 256/3;
  pos_b = 256*2/3;
  red_blink_interval = 256;
  grn_blink_interval = 512;
  blu_blink_interval = 1024;
  blink_counter = 0;
}

void animate_np() {
  uint32_t now = millis();
  if (now < next_animation_time) return;
  
  switch (animation_mode) {
    case rainbow_wheel:
      for(anim_i=0; anim_i< strip.numPixels(); anim_i++) {
        strip.setPixelColor(anim_i, Wheel(((anim_i * 256 / strip.numPixels()) + anim_j) & 255));
      }
      strip.show();
      anim_j = (anim_j + rainbow_wheel_speed) & 255;
      next_animation_time = now + 20;
      break;
      
    case rainbow_chase:
      for(anim_i=0; anim_i<strip.numPixels(); anim_i+=3) {
        strip.setPixelColor(anim_i + ((anim_k+2)%3), 0);
        strip.setPixelColor(anim_i + anim_k, Wheel(((anim_i*256 / strip.numPixels()) + anim_j) & 255));
      }
      strip.show();
      anim_j = (anim_j + 1) & 255;
      anim_k = (anim_k + 1) % 3;
      next_animation_time = now + chase_animation_delay;
      break;
      
    case rotating_chase:
      animation_target_color = Wheel(anim_j);
      anim_j = (anim_j + 7) & 255;
      // fallthrough intentional
    case color_chase:
      for(anim_i=0; anim_i<strip.numPixels(); anim_i+=3) {
        strip.setPixelColor(anim_i + ((anim_k+2)%3), 0);
        strip.setPixelColor(anim_i + anim_k, animation_target_color);
      }
      strip.show();
      anim_k = (anim_k + 1) % 3;
      next_animation_time = now + chase_animation_delay;   
      break;
      
    case rotating_wipe:
      animation_target_color = Wheel(anim_j);
      anim_j = (anim_j + 7) & 255;
      // fallthrough intentional
    case color_wipe:
      if ( !anim_k ) {
        strip.setPixelColor(anim_i, animation_target_color);
      } else {
        strip.setPixelColor(anim_i, 0);
      }
      strip.show();
      anim_i++;
      if ( anim_i >= strip.numPixels() ) {
        anim_i = 0;
        anim_k = !(anim_k);
      }
      next_animation_time = now + wipe_animation_delay;
      break;

    case rainbow_wipe:
      if ( !anim_k ) {
        strip.setPixelColor(anim_i, Wheel(((anim_i*256 / strip.numPixels()) + anim_j) & 255));
        anim_j = (anim_j + 7) & 255;
      } else {
        strip.setPixelColor(anim_i, 0);
      }
      strip.show();
      anim_i++;
      if ( anim_i >= strip.numPixels() ) {
        anim_i = 0;
        anim_k = !(anim_k);
      }
      next_animation_time = now + wipe_animation_delay;
      break;

    case chaser:
      for(anim_i = 0; anim_i < strip.numPixels(); anim_i++) {
        strip.setPixelColor(anim_i, strip.Color((pos_r*strip.numPixels()/256)==anim_i?255:0, 
                                                (pos_g*strip.numPixels()/256)==anim_i?255:0,
                                                (pos_b*strip.numPixels()/256)==anim_i?255:0));
      }
      strip.show();
      pos_r += dir_r;
      if ( pos_r < 0 ) {
        pos_r += 256;
      } else if ( pos_r >= 256 ) {
        pos_r -= 256;
      }
      pos_g += dir_g;
      if ( pos_g < 0 ) {
        pos_g += 256;
      } else if ( pos_g >= 256 ) {
        pos_g -= 256;
      }
      pos_b += dir_b;
      if ( pos_b < 0 ) {
        pos_b += 256;
      } else if ( pos_b >= 256 ) {
        pos_b -= 256;
      }
      next_animation_time = now + 20;
      break;
      
    default:
      animation_target_color = strip.Color(0,0,0);
      animation_mode = color_wipe;
      // don't set next_animation_time, so we start next time
  }   
      
}

void read_all_buttons() {
  red_button.read();
  grn_button.read();
  blu_button.read();
  
  big_button.read();
  
  sw1.read();
  sw2.read();
}

void setup() {
  // Random seed setup
  randomSeed(analogRead(0));
  
  pinMode(REDBTN_OUT,   OUTPUT);
  pinMode(GRNBTN_OUT,   OUTPUT);
  pinMode(BLUBTN_OUT,   OUTPUT);
  pinMode(SPKR_PIN,     OUTPUT);
  
  strip.begin();
  strip.show();
  animation_init();
}

void loop() {

  read_all_buttons();
  
  if ( sw1.has_event || sw2.has_event || curr_mode < 0 ) {
    int new_mode = (sw1.is_down?1:0) + (sw2.is_down?2:0);
    switch (new_mode) {
      case 0:
        animation_mode = chaser;
        dir_r = dir_b = 5;
        dir_g = -5;
        digitalWrite(REDBTN_OUT, HIGH);
        digitalWrite(GRNBTN_OUT, HIGH);
        digitalWrite(BLUBTN_OUT, HIGH);
        break;
      case 1:
        animation_mode = rainbow_chase;
        red_blink_interval = 256;
        grn_blink_interval = 512;
        blu_blink_interval = 1024;
        blink_counter = 0;
        break;
      case 2:
        animation_mode = rotating_chase;
        break;
      case 3:
        animation_mode = rainbow_wheel;
        break;
      default:
        // Can't happen
        animation_mode = erase;
    }
    curr_mode = new_mode;
  }
  
  switch (curr_mode) {
    case 0: // chaser
      if ( red_button.has_event ) {  // red button pressed
        if ( red_button.is_down ) {
          if ( dir_r > 0 && dir_r < 32 ) {
            dir_r += 1;
          } else if ( dir_r <= 0 && dir_r > -32 ) {
            dir_r -= 1;
          }
          dir_r *= -1;
          digitalWrite(REDBTN_OUT, LOW);
        } else {                                  // red button released
          digitalWrite(REDBTN_OUT, HIGH);
        }
      }
      
      if ( grn_button.has_event ) {  // green button pressed
        if ( grn_button.is_down ) {
          if ( dir_g > 0 && dir_g < 32 ) {
            dir_g += 1;
          } else if ( dir_g <= 0 && dir_g > -32 ) {
            dir_g -= 1;
          }
          dir_g *= -1;
          digitalWrite(GRNBTN_OUT, LOW);
        } else {
          digitalWrite(GRNBTN_OUT, HIGH);
        }
      }
      
      if ( blu_button.has_event ) {  // blue button pressed
        if ( blu_button.is_down ) {
          if ( dir_b > 0 && dir_b < 32 ) {
            dir_b += 1;
          } else if ( dir_b <= 0 && dir_b > -32 ) {
            dir_b -= 1;
          }
          dir_b *= -1;
          digitalWrite(BLUBTN_OUT, LOW);
        } else {
          digitalWrite(BLUBTN_OUT, HIGH);
        }
      }
      
      if ( big_button.has_event ) { // big button pressed
        if ( big_button.is_down ) {
          if ( dir_b == 5 && dir_r == 5 && dir_g == -5 ) {
            dir_b = random(-10, 11);
            dir_g = random(-10, 11);
            dir_r = random(-10, 11);
          } else {
            dir_b = 5;
            dir_r = 5;
            dir_g = -5;
          }
        }
      }
      break;
    case 1:
      digitalWrite(REDBTN_OUT, ((blink_counter&red_blink_interval) != 0)?HIGH:LOW);
      digitalWrite(GRNBTN_OUT, ((blink_counter&grn_blink_interval) != 0)?HIGH:LOW);
      digitalWrite(BLUBTN_OUT, ((blink_counter&blu_blink_interval) != 0)?HIGH:LOW);
      
      if ( red_button.is_pressed ) {
        if ( red_blink_interval >= 4096 ) {
          red_blink_interval = 128;
        } else {
          red_blink_interval <<= 1;
        }
      }
      
      if ( grn_button.is_pressed ) {
        if ( grn_blink_interval >= 4096 ) {
          grn_blink_interval = 128;
        } else {
          grn_blink_interval <<= 1;
        }
      }
      
      if ( blu_button.is_pressed ) {
        if ( blu_blink_interval >= 4096 ) {
          blu_blink_interval = 128;
        } else {
          blu_blink_interval <<= 1;
        }
      }
      
      if ( big_button.is_pressed ) {
        red_blink_interval = 256;
        grn_blink_interval = 512;
        blu_blink_interval = 1024;
        blink_counter = 0;
      }
      
      blink_counter = (blink_counter + 1) % 8192;
      break;
    case 2:
      if ( big_button.is_pressed ) {
        switch ( animation_mode ) {
          case rotating_chase:
            animation_mode = color_chase;
            break;
          case color_chase:
            animation_mode = rainbow_chase;
            break;
          case rainbow_chase:
            animation_mode = rotating_wipe;
            break;
          case rotating_wipe:
            animation_mode = color_wipe;
            break;
          case color_wipe:
            animation_mode = rainbow_wipe;
            break;
          default: // hopefully rainbow_wipe
            animation_mode = rotating_chase;
            break;
        }
      }
      
      if ( grn_button.is_pressed ) {
        digitalWrite(GRNBTN_OUT, HIGH);
        switch ( animation_mode ) {
          case rotating_wipe:
          case color_wipe:
          case rainbow_wipe:
            wipe_animation_delay = random(10, 100);
            break;
          case color_chase:
          case rotating_chase:
          case rainbow_chase:
            chase_animation_delay = random(25, 125);
            break;
        }
      } else if ( grn_button.is_long_press ) {
        if ( (((millis() - grn_button.button_debounce_timer)/512)%2) == 1 ) {
          digitalWrite(GRNBTN_OUT, LOW);
        } else {
          digitalWrite(GRNBTN_OUT, HIGH);
        }
        wipe_animation_delay = 25;
        chase_animation_delay = 50;
      } else if ( ! grn_button.is_down ) {
        digitalWrite(GRNBTN_OUT, LOW);
      }
      
      if ( red_button.is_pressed ) {
        digitalWrite(REDBTN_OUT, HIGH);
        switch ( animation_mode ) {
          case rotating_wipe:
          case color_wipe:
          case rainbow_wipe:
            if ( wipe_animation_delay > 10 ) {
              wipe_animation_delay -= 5;
            }
            break;
          case color_chase:
          case rotating_chase:
          case rainbow_chase:
            if ( chase_animation_delay > 25 ) {
              chase_animation_delay -= 5;
            }
            break;
        }
      } else if ( red_button.is_long_press ) {
        if ( (((millis() - btn_debounce_timer)/512)%2) == 1 ) {
          digitalWrite(REDBTN_OUT, LOW);
        } else {
          digitalWrite(REDBTN_OUT, HIGH);
        }
        wipe_animation_delay = 10;
        chase_animation_delay = 25;
      } else if ( ! red_button.is_down ) {
        digitalWrite(REDBTN_OUT, LOW);
      }
      
      if ( blu_button.is_pressed ) {
        digitalWrite(BLUBTN_OUT, HIGH);
        switch ( animation_mode ) {
          case rotating_wipe:
          case color_wipe:
          case rainbow_wipe:
            if ( wipe_animation_delay < 100 ) {
              wipe_animation_delay += 5;
            }
            break;
          case color_chase:
          case rotating_chase:
          case rainbow_chase:
            if ( chase_animation_delay < 150 ) {
              chase_animation_delay += 5;
            }
            break;
        }
      } else if ( blu_button.is_long_press ) {
        if ( (((millis() - btn_debounce_timer)/512)%2) == 1 ) {
          digitalWrite(BLUBTN_OUT, LOW);
        } else {
          digitalWrite(BLUBTN_OUT, HIGH);
        }
        wipe_animation_delay = 100;
        chase_animation_delay = 150;
      } else if ( ! blu_button.is_down ) {
        digitalWrite(BLUBTN_OUT, LOW);
      }
      break;
    case 3:
      if ( red_button.is_pressed ) {
        rainbow_wheel_speed = rainbow_wheel_speed^1;
      }
      
      if ( grn_button.is_pressed ) {
        rainbow_wheel_speed = rainbow_wheel_speed^2;
      }
      
      if ( blu_button.is_pressed ) {
        rainbow_wheel_speed = rainbow_wheel_speed^4;
      }
      
      // Write status of bits out
      digitalWrite(REDBTN_OUT, ((rainbow_wheel_speed&1)!=0)?HIGH:LOW);
      digitalWrite(GRNBTN_OUT, ((rainbow_wheel_speed&2)!=0)?HIGH:LOW);
      digitalWrite(BLUBTN_OUT, ((rainbow_wheel_speed&4)!=0)?HIGH:LOW);
      
      break;
    default:
      // nothing
      break;
  }
  
  // to keep the loop from going too fast   
  delay(1);
  animate_np(); 
       
}

