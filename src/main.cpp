#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/* 
TODO: Implement the circuit on a breadboard
TODO: Record video of system working
TODO: Update/Simplify documentation if necessary
TODO: PCB Design

High Level Project Objective: Simulate smart lighting system with an override option for manual lighting control.

High Level Functionality: 
* When motion is detected, esp32 controller activates relay module which in turn turns on the red led.
* When the button is pressed at anytime, the system switches to override mode, where the user can turn on/off the led manually
* If no motion is detected for `timeout` time, then the led is automatically turned off
* RGB led represents system states: Red (override mode), Green (motion detected) and Blue (idle/no motion/override inactive)
*/

// Pin Definitions
#define RELAY_PIN 23
#define PIR_PIN 33
#define BUTTON_PIN 14

// 8RGB LED Configuration
#define LED_PIN 18
#define NUM_LEDS 1        // Using only 1 LED from the strip

// Create NeoPixel strip object
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// State Variables
volatile bool override_mode = false; 
volatile bool relay_state = false;
int motion = 0;

unsigned long last_motion_time = 0;
volatile unsigned long last_pressed_time = 0; 
const unsigned long timeout = 5000;   // 5 seconds

volatile int last_button_state = HIGH; 

// Interrupt Handler
void IRAM_ATTR handleButton() {
  unsigned long now = millis();
  if (now - last_pressed_time > 250) {  // Debouncing
    override_mode = true;
    relay_state = !relay_state;
    last_pressed_time = now;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Button setup with internal pullup
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);

  // Sensor and relay setup
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Start with relay off

  // NeoPixel setup
  strip.begin();           // Initialize NeoPixel strip object
  strip.setBrightness(50); // Set brightness to about 20% (max = 255)
  strip.show();            // Turn OFF all pixels ASAP
  strip.clear();           // Set all pixel colors to 'off'

  Serial.println("Smart Lighting System Initialized");
}

void handleMotion() {
  if (!override_mode) {
    motion = digitalRead(PIR_PIN);

    if (motion == HIGH) {
      last_motion_time = millis();
      relay_state = true;
      Serial.println("Motion detected!");
    }

    // Auto turn-off after timeout if led is on
    if (relay_state && (millis() - last_motion_time > timeout)) {
      relay_state = false;
      Serial.println("Motion timeout - turning off");
    }
  }
}

void checkOverrideTimeout() {
  if (override_mode) {
    if (millis() - last_pressed_time > timeout) {
      override_mode = false;
      Serial.println("Override mode timeout - returning to auto");
    }
  }
}

void updateRelay() {
  digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
}

void updateRGB() {
  if (override_mode) {
    // Red = manual override
    strip.setPixelColor(0, strip.Color(255, 0, 0));
  }
  else if (relay_state) {
    // Green = motion active
    strip.setPixelColor(0, strip.Color(0, 255, 0));
  }
  else {
    // Blue = idle
    strip.setPixelColor(0, strip.Color(0, 0, 255));
  }
  
  strip.show();  // Update the LED with new color
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'm':
        Serial.println("DEBUG: Simulating motion detected");
        last_motion_time = millis();
        relay_state = true;
        break;
      case 't':
        Serial.println("DEBUG: Simulating no motion (trigger timeout)");
        last_motion_time = millis() - timeout - 1000;
        break;
      case 'o':
        Serial.println("DEBUG: Toggling override mode");
        override_mode = !override_mode;
        last_pressed_time = millis();
        break;
      case 's':
        Serial.println("--- Status ---");
        Serial.print("override_mode: "); Serial.println(override_mode ? "true" : "false");
        Serial.print("relay_state: "); Serial.println(relay_state ? "ON" : "OFF");
        Serial.print("last_motion_time: "); Serial.println(millis() - last_motion_time);
        Serial.println("---------------");
        break;
      default:
        Serial.print("Unknown command: "); Serial.println(cmd);
        Serial.println("Commands: m=motion, t=timeout, o=override, s=status");
        break;
    }
  }
}

void loop() {
  handleSerialCommands();
  handleMotion();
  checkOverrideTimeout();
  updateRelay();
  updateRGB();
  
  delay(50);
}