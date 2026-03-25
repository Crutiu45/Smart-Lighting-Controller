#include <Arduino.h>

/* 
TODO: Change code to not use relay module
TODO: Implement the circuit on a breadboard
TODO: Record video of system working
TODO: Update/Simplify documentation if necessary
TODO: PCB Design

High Level Project Objective: Simulate smart lighting system with an override option for manual lighting control.

High Level Functionality: 
* When motion is detected, esp32 controller activates relay module which in turn turns on the red led.
* When the button is pressed at anytime, the system switches to override mode, where the user can turn on/off the led maually
* If no motion is detected for `timeout` time, then the led is automatically turned off
* RGB led represents system states: Red (override mode), Green (motion detected) and Blue (idle/no motion/override inactive)
*/

#define relay_pin 23
#define pir_pin 33
#define button_pin 14
#define red_pin 19
#define blue_pin 22
#define green_pin 18

// vars shared by ISR and Main should be volatile in case hardware changes its value i.e. any var ISR modifies
volatile bool override_mode = false; 
volatile bool relay_state = false;
int motion = 0;

unsigned long last_motion_time = 0;

volatile unsigned long last_override_activity = 0;
const unsigned long timeout = 10000;   // 10 seconds

volatile unsigned long last_pressed_time = 0; 

// made volatile because ISR modifies it 
volatile int last_button_state = HIGH; 

void IRAM_ATTR handleButton() {
  unsigned long now = millis();
  if (now - last_pressed_time > 250) {
    override_mode = true;
    relay_state = !relay_state;
    //remember the time of pressed for next time 
    last_pressed_time = now;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // uses internal resistor to pull that pin to 3.3V initially at setup time until pressed
  pinMode(button_pin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(button_pin), handleButton, FALLING);

  pinMode(pir_pin, INPUT);
  pinMode(relay_pin, OUTPUT);
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  digitalWrite(relay_pin, LOW); //turn off (sanity check)
}

void handleMotion() {
  if(!override_mode) {
    motion = digitalRead(pir_pin);

    if (motion == HIGH) {
      last_motion_time = millis();
      relay_state = true;
    }

    // Auto turn-off after timeout and if led is on
    // timeout is always gonna be 5s + timeout due to hardware setting for PIR sensor
    if (relay_state == true && (millis() - last_motion_time > timeout)) {
      relay_state = false;
    }
  }
}

void checkOverrideTimeout() {
  if (override_mode) {
    if (millis() - last_pressed_time > timeout) {
      override_mode = false;
      // We don't force relay_state = false here; we let the motion logic 
      // take over immediately on the next loop.
    }
  }
}

void updateRelay() {
  if(relay_state == true) {
    digitalWrite(relay_pin, HIGH);
  } 
  else {
    digitalWrite(relay_pin, LOW);
  }
}

void setColor(int r, int g, int b) {
  analogWrite(red_pin, r);
  analogWrite(green_pin, g);
  analogWrite(blue_pin, b);
}

void updateRGB() {

  if (override_mode) {
    setColor(255, 0, 0);    // Red = manual override
  }
  else if (relay_state) {
    setColor(0, 255, 0);    // Green = motion active
  }
  else {
    setColor(0, 0, 255);    // Blue = idle
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  handleMotion();
  checkOverrideTimeout();
  updateRelay();
  updateRGB();
}
