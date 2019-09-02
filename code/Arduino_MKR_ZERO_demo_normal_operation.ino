#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <assert.h> // Note: assert fails quietly in Arduino environment.

Adafruit_BicolorMatrix matrix[6];

const int module_width = 8; // How many pixels wide is each LED matrix?
const int character_width = 6; // pixels
const int number_of_modules = 6;

int attack_index = 0;
char attack_character = 'X';

const int bit_interval = 350; // milliseconds
const int tolerance = 200; // milliseconds
const int bit_sampling_time = 5; // milliseconds
int start_bit_duration = 0; // This is a measured value.

char * blank_padded_string = NULL;
char string_to_display[] = "Normal operation.";

void lamp_test(void);
void clear_entire_display(void);
void scroll_horizontal_line_up_and_down(void);
void scroll_vertical_line_all_the_way_across_twice(void);\
void initialise_serial_port(void);
void blank_pad_string_for_display(void);
void initialise_data_structure_for_matrices(void);

void setup() {
  bool valid_start_bit = false;
  int pixel_offset = module_width - 1;

  initialise_data_structure_for_matrices();
  lamp_test();
  initialise_serial_port();
  
  Serial.println("In setup()");
  
  blank_pad_string_for_display();
  
  pinMode(LED_BUILTIN, OUTPUT);

  for(;;) {
    int time_since_last_blink = 0; // milliseconds

    for (int i=0; i< (int)strlen(blank_padded_string); i++) {
      int elapsed_time = 0; // milliseconds
      
      // I had to tune the following value carefully; too small and the
      // target LED never lights up; too large and the display scrolls
      // frustratingly slowly. 60 for the matrix display is a compromise.
      
      const int display_update_interval = 60; // milliseconds
      const int blink_interval = 1000; // milliseconds
      
      int string_length_in_pixels = strlen(blank_padded_string) * character_width;

      // do one iteration of the `Normal operation' display

      for (uint8_t i=0; i<number_of_modules; i++) {
        matrix[i].setTextSize(1);
        matrix[i].setTextWrap(false);
        matrix[i].setTextColor(LED_GREEN);
        matrix[i].setRotation(1);

        matrix[i].clear();
        matrix[i].setCursor(0 - module_width * i + pixel_offset, 0);
        matrix[i].print(blank_padded_string);
        matrix[i].writeDisplay();
      }
      --pixel_offset;
      if(pixel_offset <= 0 - string_length_in_pixels) {
        pixel_offset = module_width - 1;
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

      // I'm not sure if this first digitalRead() is necessary.
      // (void)digitalRead(LED_BUILTIN);

      while(digitalRead(LED_BUILTIN)) {
        delay(bit_sampling_time);
        time_since_last_blink += bit_sampling_time;
        elapsed_time += bit_sampling_time;
      }
      if (elapsed_time > 0) {
        Serial.println("I saw light.");
        if (elapsed_time < bit_interval - tolerance) {
          Serial.print("But it was too short; elapsed_time = ");
          Serial.print(elapsed_time);
          Serial.println(" ms.");
          continue;
        }
        else if (elapsed_time > bit_interval + tolerance) {
          Serial.print("But it was too long; elapsed_time = ");
          Serial.print(elapsed_time);
          Serial.println(" ms.");
          continue;
        }
        else {
          Serial.print("I think I saw a start bit. It lasted ");
          Serial.print(elapsed_time);
          Serial.println(" milliseconds.");
          start_bit_duration = elapsed_time;
          valid_start_bit = true;
          break;
        }
      }
      delay(display_update_interval - elapsed_time);
      time_since_last_blink += (display_update_interval - elapsed_time);
      elapsed_time = 0; // Reset the timer.
    }
    if (valid_start_bit) {
      Serial.println("I'm listening.");
      start_bit_duration = 1.1 * bit_interval;
      Serial.print("I'm resetting the initial start bit duration to ");
      Serial.print(start_bit_duration);
      Serial.println(" ms.");
      break;
    }
  }
}

// Unreachable, but do it anyway on point of principle.
// free(padded_string);

int digit_position = 0;
bool valid_start_bit = false;
int bit_accumulator = 0;

void loop() {
  // We already saw a start bit, so look for data bits now.
  
  pinMode(LED_BUILTIN, INPUT); // Just in case.
  int elapsed_time = 0; // milliseconds

  // Get 8 bits of data, MSB first.
  int word_length = 8; // bits
  int bit_value = 0;

  for (int i=word_length - 1; i >= 0; i--) {
    // Try to look in about the middle of each bit interval.
    elapsed_time = 0;
    while (elapsed_time < 0.5 * start_bit_duration) {
      #ifdef VERBOSE
      Serial.println(digitalRead(LED_BUILTIN));
      #endif
      delay(bit_sampling_time);
      elapsed_time += bit_sampling_time;
    }
    bit_value = digitalRead(LED_BUILTIN);
    switch (bit_value) {
      case 0:
      case 1:
        Serial.print("bit 2^");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(bit_value);
        bit_accumulator += (bit_value << i);
        Serial.print("bit_accumulator = ");
        Serial.println(bit_accumulator);
        break;
      default:
        Serial.print("This can't happen: bit_value[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(bit_value);
        break;
    }
    // Wait out the duration of the data bit.
    elapsed_time = 0;
    while (elapsed_time < 0.5 * start_bit_duration) {
      #ifdef VERBOSE
      Serial.println(digitalRead(LED_BUILTIN));
      #endif
      delay(bit_sampling_time);
      elapsed_time += bit_sampling_time;
    }
    #ifdef VERBOSE
    Serial.println("We are no longer considering the bit.");
    #endif
  }
  // Wait out the last data bit.
  elapsed_time = 0;
  while(elapsed_time < 0.5 * start_bit_duration) {
    #ifdef VERBOSE
    Serial.println(digitalRead(LED_BUILTIN));
    #endif
    delay(bit_sampling_time);
    elapsed_time += bit_sampling_time;
  }
  #ifdef VERBOSE
  Serial.println("We are no longer considering last data bit.");
  #endif
  // Verify that we got a good stop bit for framing.
  elapsed_time = 0;
  while(digitalRead(LED_BUILTIN)) {
    delay(bit_sampling_time);
    elapsed_time += bit_sampling_time; 
  }
  Serial.print("I waited ");
  Serial.print(elapsed_time);
  Serial.println(" ms for it to get dark after the last data bit.");
  // Wait out the stop bit (really ought to watch for light in here).
  elapsed_time = 0;
  while(!digitalRead(LED_BUILTIN)) {
    delay(bit_sampling_time);
    elapsed_time += bit_sampling_time;
    if (elapsed_time >= start_bit_duration) {
      break;
    }
  }
  Serial.print("I waited ");
  Serial.print(elapsed_time);
  Serial.println(" ms for the end of the stop bit.");
  // See what we got.
  attack_character = (char)bit_accumulator;
  Serial.print("I think I got an ASCII \"");
  Serial.print(attack_character);
  Serial.println("\".");

  // Display it in red on the LED matrix.
  matrix[attack_index].clear();
  matrix[attack_index].writeDisplay();
  matrix[attack_index].setTextColor(LED_RED);
  matrix[attack_index].setCursor(0,0);
  matrix[attack_index].print(attack_character);
  matrix[attack_index].writeDisplay();
  ++attack_index;

  bit_accumulator = 0;

  // Wait for another start bit.
  Serial.println("Waiting for another start bit.");

  // It's a state machine.
  elapsed_time = 0;
  while(!digitalRead(LED_BUILTIN)) {
    delay(bit_sampling_time);
    elapsed_time += bit_sampling_time;
  }
  Serial.print("I waited ");
  Serial.print(elapsed_time);
  Serial.println(" ms for light to appear.");

  elapsed_time = 0;
  while(digitalRead(LED_BUILTIN)) {
    delay(bit_sampling_time);
    elapsed_time += bit_sampling_time;
  }
  if (elapsed_time > 0) {
    Serial.println("I saw light.");
    if (elapsed_time < bit_interval - tolerance) {
      Serial.print("But it was too short; elapsed_time = ");
      Serial.print(elapsed_time);
      Serial.println(" ms.");
    }
    else if (elapsed_time > bit_interval + tolerance) {
      Serial.print("But it was too long; elapsed_time = ");
      Serial.print(elapsed_time);
      Serial.println(" ms.");
    }
    else {
      Serial.print("I think I saw a start bit. It lasted ");
      Serial.print(elapsed_time);
      Serial.println(" milliseconds.");
      valid_start_bit = true;
      start_bit_duration = elapsed_time; // Measure it anew every time.
    }
  }
}

void lamp_test(void) {
  clear_entire_display();
  scroll_vertical_line_all_the_way_across();
  scroll_horizontal_line_up_and_down();
}

void clear_entire_display(void) {
  for(uint8_t module=0; module<number_of_modules; module++) {
    matrix[module].clear();
    matrix[module].writeDisplay();
  }
}

void scroll_horizontal_line_up_and_down(void) {
  for (uint8_t row=0; row < module_width; row++) {
    for (uint8_t module=0; module<number_of_modules; module++) {
      matrix[module].drawFastVLine(module_width - row,0, module_width,
        LED_YELLOW);
      matrix[module].writeDisplay();
    }
    delay(3);
    clear_entire_display();
  }
  for (uint8_t row=0; row < module_width; row++) {
    for (uint8_t module=0; module<number_of_modules; module++) {
      matrix[module].drawFastVLine(row,0, module_width, LED_YELLOW);
      matrix[module].writeDisplay();
    }
    delay(3);
    clear_entire_display();
  }
}

void scroll_vertical_line_all_the_way_across() {
  for (uint8_t module=0; module < number_of_modules; module++) {
    for (uint8_t y=0; y<module_width; y++) {
      matrix[module].drawFastHLine(0,y, module_width, LED_YELLOW);
      matrix[module].writeDisplay();
      delay(4);
      matrix[module].clear();
      matrix[module].writeDisplay();
    }
  }
}

void blank_pad_string_for_display(void) {
    // Pad string with enough blanks at beginning to fill entire display.
  int num_blanks = number_of_modules * module_width / character_width - 1;
  blank_padded_string = (char *)malloc(1 + num_blanks + strlen(string_to_display));
  assert(blank_padded_string);
  for (int i=0; i<num_blanks; i++) {
    blank_padded_string[i] = ' ';
  }
  blank_padded_string[num_blanks] = '\0';
  strcat(blank_padded_string, string_to_display);
}

void initialise_serial_port(void) {
  //while (!Serial);
  Serial.begin(9600);
  Serial.println("Hello World");
  // It takes about 2 seconds before the serial port is ready.
  const int serial_port_wait = 5; // seconds
  Serial.print("Waiting ");
  Serial.print(serial_port_wait);
  Serial.print(" seconds for the serial port to wake up: ");
  for (int i = 5; i >= 0; i--) {
    Serial.print(i);
    Serial.print(" ");
    delay(1000);
  }
  Serial.println("\nReady to go.");
}

void initialise_data_structure_for_matrices(void) {
  // Create a data structure for each module.
  for(uint8_t i=0; i<number_of_modules; i++) {
    matrix[i] = Adafruit_BicolorMatrix();
    matrix[i].begin(0x70 + i);
  }
}
