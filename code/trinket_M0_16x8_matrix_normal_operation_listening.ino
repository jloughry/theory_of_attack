#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <assert.h> // Note: assert fails quietly in Arduino environment, but stops.

// If the board seems to freeze unexpectedly, it probably hit an assert().

// For the in-built RGB LED
#include <Adafruit_DotStar.h>
#define NUMPIXELS 1
#define DATAPIN 7
#define CLOCKPIN 8
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

const int display_width = 16; // pixels
const int character_width = 6; // pixels
char string_to_display[] = "Normal operation";
const int max_padded_string_length = 100;
char padded_string[max_padded_string_length];
unsigned padding = 1 + display_width / character_width;
int limit = display_width - (strlen(string_to_display) + padding) * character_width;
int time_since_last_blink = 0;
int elapsed_time = 0;
const int blink_interval = 1000; // milliseconds
const int bit_interval = 350; // milliseconds
const int bit_sampling_time = 50; // milliseconds
const int tolerance = 1000; // milliseconds (made larger for handheld testing)

void setup() {
  //while (!Serial);
  Serial.begin(9600);
  Serial.println("Hello World");

  // For the in-built RGB LED
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
  // If I wanted to turn it on:
  // strip.setPixelColor(0, 0xffffff); // white
  // strip.show();

  assert(limit < 0);
  assert(limit >= -127);
  assert(strlen(string_to_display) + padding < max_padded_string_length);
  
  strcpy(padded_string, string_to_display);
  for (int i=0; i<padding; i++) {
    strcat(padded_string, " ");
  }
  assert(strlen(padded_string) < max_padded_string_length);
  assert(strlen(padded_string) > strlen(string_to_display));
  
  matrix.begin(0x70);  // pass in the address
  pinMode(LED_BUILTIN, OUTPUT);
}

int8_t x = display_width - 1;

void loop() {
  matrix.setTextSize(1);
  matrix.setTextWrap(false);
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
  
  matrix.clear();
  matrix.setCursor(x,0);
  matrix.print(padded_string);
  matrix.writeDisplay();
  delay(40);
  time_since_last_blink += 50; // milliseconds

  --x;
  if (x < limit) {
    x = display_width - 1;
  }

  // Only blink the LED if it's time to.
  if (time_since_last_blink > blink_interval) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    time_since_last_blink = 0;
  }

  pinMode(LED_BUILTIN, INPUT);
  while(digitalRead(LED_BUILTIN)) {
    delay(bit_sampling_time);
    time_since_last_blink += bit_sampling_time;
    elapsed_time += bit_sampling_time;
  }
  if(elapsed_time > bit_interval) {
    Serial.println("I saw light!");
    delay(5000); // milliseconds
    elapsed_time = 0;
  }
}
