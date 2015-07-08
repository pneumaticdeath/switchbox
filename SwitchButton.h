/* Copyright 2015, Mitch Patenaude  patenaude@gmail.com */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class SwitchButton {
  public:
    SwitchButton(uint8_t pin, uint32_t long_press_time=1000, uint32_t debouce_time=50,
                 uint8_t speaker_pin=0, uint16_t down_tone=0, uint16_t down_tone_duration=0,
                                        uint16_t up_tone=0, uint16_t up_tone_duration=0);
  
    uint8_t pin;
    boolean is_down;
    boolean is_pressed;
    boolean is_released;
    boolean has_event;
    boolean is_long_press;
    uint32_t button_debounce_timer;

    boolean read();
    uint32_t press_duration();

  private:
    uint32_t long_press_time;
    uint32_t debounce_time;
    uint8_t  speaker_pin;
    uint32_t prev_press_duration;
    uint16_t down_tone;
    uint16_t down_tone_duration;
    uint16_t up_tone;
    uint16_t up_tone_duration;

    void clear_state();

};
#endif
