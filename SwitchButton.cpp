
#include "SwitchButton.h"

SwitchButton::SwitchButton(uint8_t pin, uint32_t long_press_time, uint32_t debouce_time,
               uint8_t speaker_pin, uint16_t down_tone, uint16_t down_tone_duration,
	       uint16_t up_tone, uint16_t up_tone_duration) {
  this->pin = pin;
  this->long_press_time = long_press_time;
  this->speaker_pin = speaker_pin;
  this->down_tone = down_tone;
  this->down_tone_duration = down_tone_duration;
  this->up_tone = up_tone;
  this->up_tone_duration = up_tone_duration;
  this->debounce_time = debounce_time;

  // initialize the pin
  pinMode(this->pin, INPUT);
  digitalWrite(this->pin, HIGH);

  this->button_debounce_timer = millis();
  this->is_down = (digitalRead(this->pin) == 0); 
  this->clear_state();
}

void SwitchButton::clear_state() {
  this->is_pressed = false;
  this->is_released = false;
  this->is_long_press = false;
  this->has_event = false;
}

boolean SwitchButton::read() {
  if ( this->button_debounce_timer > millis() ) { // We've rolled over
    this->button_debounce_timer = 0;
  }
  
  this->clear_state();
  if ( (millis() - this->button_debounce_timer) < this->debounce_time ) return false;

  if ( digitalRead(this->pin) == 0 ) {
    if ( ! this->is_down ) {
      this->is_down = true;
      this->is_pressed = true;
      this->has_event = true;
      this->button_debounce_timer = millis();
      if ( this->down_tone && this->down_tone_duration ) {
        tone(this->speaker_pin, this->down_tone, this->down_tone_duration);
      }
      return true;
    } else if ( (millis() - this->button_debounce_timer) >= this->long_press_time ) {
      this->is_long_press = true;
    }
  } else { 
    if ( this->is_down ) {
      this->is_down= false;
      this->is_released = true;
      this->has_event = true;
      this->button_debounce_timer = millis();
      if ( this->up_tone && this->up_tone_duration ) {
        tone(this->speaker_pin, this->up_tone, this->up_tone_duration);
      }
      return true;
    }
  }

  return false;
}

