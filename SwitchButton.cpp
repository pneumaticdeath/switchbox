/* Copyright 2015, Mitch Patenaude  patenaude@gmail.com */

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
  this->is_down = (digitalRead(this->pin) == 0); 
  digitalWrite(this->pin, LOW);

  this->button_debounce_timer = millis();
  this->clear_state();
}

void SwitchButton::clear_state() {
  this->is_pressed = false;
  this->is_released = false;
  this->is_long_press = false;
  this->has_event = false;
}

boolean SwitchButton::read() {
  int val;

  if ( this->button_debounce_timer > millis() ) { // We've rolled over
    this->button_debounce_timer = 0;
  }
  
  this->clear_state();
  if ( (millis() - this->button_debounce_timer) < this->debounce_time ) return false;

  digitalWrite(this->pin, HIGH); // set pull-up resistor for read
  val = digitalRead(this->pin);  // read the state of the switch/button. 0 is down/closed.
  digitalWrite(this->pin, LOW);  // clears pull-up resistor to save power

  if ( val == 0 ) { // 0 represents down/closed.
    if ( ! this->is_down ) {  // if not previously down/closed
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
      this->is_down = false;
      this->is_released = true;
      this->has_event = true;
      this->prev_press_duration = millis() - this->button_debounce_timer;
      this->button_debounce_timer = millis();
      if ( this->up_tone && this->up_tone_duration ) {
        tone(this->speaker_pin, this->up_tone, this->up_tone_duration);
      }
      return true;
    }
  }

  return false;
}

uint32_t SwitchButton::press_duration() {
  if ( this->is_released ) {
    return this->prev_press_duration;
  } else if ( this->is_down ) {
    return millis() - this->button_debounce_timer;
  }
  
  // logic is that when the button is up and not newly released we should return 0.
  return 0;
}

uint32_t SwitchButton::idle_time() {
  return millis() - this->button_debounce_timer;
}

