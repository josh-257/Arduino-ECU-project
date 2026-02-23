#include <Wire.h>
#include <I2C_RTC.h>
#include <math.h>
#include <EEPROM.h>

// Pins
const int btn1_pin = 4;
const int btn2_pin = 5;
const int btn3_pin = 6;
const int btn4_pin = 7;
const int btn5_pin = 8;
const int thermistor_pin = A0;

// Globals
int btn1_state = HIGH;
int btn2_state = HIGH;
int btn3_state = HIGH;
int btn4_state = HIGH;
int btn5_state = HIGH;
int btn1_prev = HIGH;
int btn2_prev = HIGH;
int btn3_prev = HIGH;
int btn4_prev = HIGH;
int btn5_prev = HIGH;
unsigned long last_debounce_time1 = 0;
unsigned long last_debounce_time2 = 0;
unsigned long last_debounce_time3 = 0;
unsigned long last_debounce_time4 = 0;
unsigned long last_debounce_time5 = 0;
const int debounce_delay = 50;
const int max_temp = 40;
const float supply_vol = 5.0;
const float res_1 = 10000.0;
const float nom_res = 10000.0;
const float nominal_temp = 25;
const float beta = 3950;
bool fault = false;
bool scroll = false;
unsigned long lastFaultTime;
const int EEPROM_write_ptr_address = 0;
int last_addr = -1;
unsigned long last_print_time = 0;
bool button_5_pressed = false;


struct FaultData {
  unsigned long time;
  char name[30];
};

void writeFaultToEEPROM (const char* faultname);
void createFault (FaultData& f, unsigned long t, const char* label);

float find_temp(int pin);

static DS1307 RTC;

void setup() {
  // Set Baud rate and GPIO pins
  Serial.begin(9600);
  pinMode(btn1_pin, INPUT_PULLUP);
  pinMode(btn2_pin, INPUT_PULLUP);
  pinMode(btn3_pin, INPUT_PULLUP);
  pinMode(btn4_pin, INPUT_PULLUP);
  pinMode(btn5_pin, INPUT_PULLUP);
  pinMode(thermistor_pin, INPUT);
  RTC.begin();
  
  int next_write_address;
  EEPROM.get(EEPROM_write_ptr_address, next_write_address);
  // Make sure EEPROM write pointer is within bounds
  if(next_write_address < sizeof(int)|| next_write_address >= EEPROM.length()){
    next_write_address = sizeof(int);
    EEPROM.put(EEPROM_write_ptr_address, next_write_address);
  }
}

void loop() {
  
  // EGT over temp
  // Check current temperature against max temperature
  if(find_temp(thermistor_pin) > max_temp && !fault){
    Serial.println("EGT OVRTMP");
    writeFaultToEEPROM ("EGT Over Temp");
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
        writeFaultToEEPROM ("Over Speed");
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
        writeFaultToEEPROM ("Oil Over Temp");
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
        writeFaultToEEPROM ("Engine Fire");
      }
    }
  }
  btn3_prev = btn3_read;

  if (fault == true && millis() - lastFaultTime >= 10000) {
  fault = false;
  }
  
  int btn4_read = digitalRead(btn4_pin);

  if (btn4_read != btn4_prev){
    last_debounce_time4 = millis();
  }

  /* Allows user to switch between RPM display and temperature 
  reading (EGT reading) through pressing button 4.*/
  if (millis() > (last_debounce_time4 + debounce_delay)){
    if (btn4_read != btn4_state) {
      btn4_state = btn4_read;
      if (btn4_state == LOW){
        if(scroll){
          scroll = false;
        }
        else if (!scroll){
          scroll = true;
        }
      }
    }
  }
  btn4_prev = btn4_read;
  /* Instruct 2nd board to display EGT reading and send updated temperature value 
  over Serial every 10 seconds*/
  if (!scroll){
    if((millis() - last_print_time) > 10000){
      float egt = find_temp(thermistor_pin);
      Serial.println("SHOW EGT");
      Serial.println(egt);
      last_print_time = millis();
    }
  }
  else if (scroll){
    if((millis() - last_print_time) > 10000){
      Serial.println("SHOW RPM");
      last_print_time = millis();
    }
  }

  int btn5_read = digitalRead(btn5_pin);

  if (btn5_read != btn5_prev){
    last_debounce_time5 = millis();
  }

  /* Each fault is logged in EEPROM.
  Each fault is logged sequentially.
  Pressing button 5 steps backthrough fault history one fault at a time.
  EEPROM memory layout: 
  bytes 0-1 = pointer to address of last fault stored,
  bytes 2-1023 = Fault_Data struct (34 bytes) for each fault.
  Reference: https://docs.arduino.cc/learn/programming/eeprom-guide/ */
  if (millis() > (last_debounce_time5 + debounce_delay)){
    if (btn5_read != btn5_state) {
      btn5_state = btn5_read;
      if (btn5_state == LOW && button_5_pressed == false){
        button_5_pressed = true;
        // Pointer storing EEPROM address where next fault will be written
        int write_ptr;
        // Create a buffer to store the newly read fault
        FaultData F_data_read;
        // Get the address of the last fault 
        EEPROM.get(EEPROM_write_ptr_address, write_ptr);
        if (last_addr == -1){
          // If it's the first press go to the most recent fault
          last_addr = write_ptr - sizeof(FaultData);
        } 
        else {
          // If not, step backward through each fault 
          last_addr -= sizeof(FaultData);
        }
        // Do not read from where the pointer to the last fault is stored
        if (last_addr >= (int)sizeof(int)){
          // Read fault data from EEPROM and send it over Serial
          EEPROM.get(last_addr, F_data_read);
          Serial.print("FAULT ");
          Serial.print(F_data_read.time);
          Serial.print(" ");
          Serial.println(F_data_read.name);
        }
      }
      else if (btn5_state == HIGH) button_5_pressed = false;
    }
  }
  btn5_prev = btn5_read;
}

float find_temp (int pin){
  // Calculate resistance of thermistor from voltage divider
  float temp_val = analogRead(pin);
  float temp_vol = (temp_val / 1023) * supply_vol;
  float temp_res = (temp_vol * res_1) / (supply_vol - temp_vol);

  /* Use Steinhart's two-point beta equation to find the temperature
  References: https://www.edn.com/ntc-thermistor-beta-steinhart-hart-guide/ */
  
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

// Add fault name and time to FaultData
void createFault (FaultData& f, unsigned long t, const char* label) {
  f.time = t;
  // Copy fault label into name
  snprintf(f.name, sizeof(f.name), "%s", label);
}

void writeFaultToEEPROM (const char* faultname){
  int write_ptr;
  // Buffer to store fault to write to EEPROM
  FaultData F_data;
  createFault(F_data, RTC.getEpoch(), faultname);
  
  EEPROM.get(EEPROM_write_ptr_address, write_ptr);
  // Do not write if EEPROM full
  if (write_ptr + sizeof(F_data) >= EEPROM.length()){
    Serial.println("EEPROM full!");
    return;
  }
  // Store fault
  EEPROM.put(write_ptr, F_data);
  // Update address of last fault
  write_ptr = write_ptr + sizeof(F_data);
  EEPROM.put(EEPROM_write_ptr_address, write_ptr);
  lastFaultTime = millis();
  fault = true;
}