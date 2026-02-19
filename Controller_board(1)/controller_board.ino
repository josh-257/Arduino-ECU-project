#include <math.h>

// Pins
const int btn1_pin = 4;
const int btn2_pin = 5;
const int btn3_pin = 6;
const int thermistor_pin = A0;

// Globals
int btn1_state = HIGH;
int btn2_state = HIGH;
int btn3_state = HIGH;
int btn1_prev = HIGH;
int btn2_prev = HIGH;
int btn3_prev = HIGH;
unsigned long last_debounce_time1 = 0;
unsigned long last_debounce_time2 = 0;
unsigned long last_debounce_time3 = 0;
const int max_temp = 40;
const float supply_vol = 5.0;
const float res_1 = 10000.0;
const float nom_res = 10000.0;
const float nominal_temp = 25;
const float beta = 3950;
const int debounce_delay = 50;
bool fault = false;
unsigned long lastFaultTime;

void setup() {
  // Set Baud rate and GPIO pins
  Serial.begin(9600);
  pinMode(btn1_pin, INPUT_PULLUP);
  pinMode(btn2_pin, INPUT_PULLUP);
  pinMode(btn3_pin, INPUT_PULLUP);
  pinMode(thermistor_pin, INPUT);
}

void loop() {
  
  // EGT over temp
  // Check current temperature against max temperature
  if(find_temp(thermistor_pin) > max_temp && !fault){
    // Send type of fault to display-board over Serial
    Serial.println("EGT OVRTMP");
    // Start fault timeout
    fault = true;
    lastFaultTime = millis();
  }

  /* Button debounce logic adapted from: 
  Udemy course: Arduino Programming and Hardware Fundamentals with Hackster, by Shawn Hymel
  https://www.udemy.com/course/arduino-programming-and-hardware-fundamentals-with-hackster/
  */

  // Engine over speed
  int btn1_read = digitalRead(btn1_pin);

  // Remember when button one changed states
  if (btn1_read != btn1_prev){
    last_debounce_time1 = millis();
  }

  // Wait before checking the state of the button again
  if (millis() > (last_debounce_time1 + debounce_delay)){
    if (btn1_read != btn1_state) {
      btn1_state = btn1_read;
      if (btn1_state == LOW && !fault){
        Serial.println("OVR SPEED");
        fault = true;
        lastFaultTime = millis();
      }
    }
  }
  btn1_prev = btn1_read;

  // Engine oil over temp
  int btn2_read = digitalRead(btn2_pin);

  if (btn2_read != btn2_prev){
    last_debounce_time2 = millis();
  }

  if (millis() > (last_debounce_time2 + debounce_delay)){
    if (btn2_read != btn2_state) {
      btn2_state = btn2_read;
      if (btn2_state == LOW && !fault){
        Serial.println("OIL TMP");
        fault = true;
        lastFaultTime = millis();
      }
    }
  }
  btn2_prev = btn2_read;

  // Engine fire
  int btn3_read = digitalRead(btn3_pin);

  if (btn3_read != btn3_prev){
    last_debounce_time3 = millis();
  }

  if (millis() > (last_debounce_time3 + debounce_delay)){
    if (btn3_read != btn3_state) {
      btn3_state = btn3_read;
      if (btn3_state == LOW && !fault){
        Serial.println("ENG FIRE");
        fault = true;
        lastFaultTime = millis();
      }
    }
  }
  btn3_prev = btn3_read;

  if (fault == true && millis() - lastFaultTime >= 10000) {
  fault = false;
  }
}

float find_temp (int pin){
  // Calculate resistance of thermistor from voltage divider
  float temp_val = analogRead(pin);
  float temp_vol = (temp_val / 1023) * supply_vol;
  float temp_res = (temp_vol * res_1) / (supply_vol - temp_vol);

  /* Use Steinhart's two-point beta equation to find the temperature
  References: https://www.edn.com/ntc-thermistor-beta-steinhart-hart-guide/, 
  AI assistance (Gemini) */
  
  //Thermistor resistance (R1) / 10k resistor (R2)
  float steinhart_value = temp_res / nom_res; 
  // log(R1 / R2)
  steinhart_value = log(steinhart_value); 
  // (1/Beta) * log(R1 / R2)
  steinhart_value /= beta; 
  // (1/nominal temp) + (1/Beta) * log(R1 / R2)
  steinhart_value += 1.0 / (nominal_temp + 273.15); 
  // 1 / ((1/nominal temp) + (1/B) * log(R1 / R2))
  steinhart_value = 1.0 / steinhart_value; 
  // Convert from Kelvin to Celsius
  steinhart_value -= 273.15; 
  return steinhart_value;
}

