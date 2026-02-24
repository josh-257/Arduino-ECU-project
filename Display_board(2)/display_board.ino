#include <LiquidCrystal.h>
#include <TimeLib.h>

// Pins
const int eng_pin = 6;
const int red_led_pin = 7;
const int orange_led_pin = 8;
const int buzzer_pin = 9;


// Engine speed values
const int normal_speed = 230;
const int over_speed = 255;

// Set LCD display to 16x2
const int lcd_width = 16; 

// LCD object containing GPIO pins
LiquidCrystal lcd(12, 11, 2, 3, 4, 5);

String incomingString;

void setup() {
  // Set baud rate and GPIO pins
  Serial.begin(9600);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  pinMode(orange_led_pin, OUTPUT);
  pinMode(eng_pin, OUTPUT);
  // Initialise LCD object
  lcd.begin(16, 2);
  // Wipe display
  lcd.clear();
  Serial.setTimeout(2000);
  // Start up fan/motor
  eng_start();
}

void loop() {
  // Listen for engine faults
  if(Serial.available()){
    // Read fault
    incomingString = Serial.readStringUntil('\n');
    incomingString.trim();
    // React to fault
    if (incomingString.equals("EGT OVRTMP")){
      lcd.clear();
      // Centre text on display
      lcd.setCursor(1,0);
      // Display fault
      lcd.print("EGT Overtemp!");
      // Sound warning buzzer and illuminate red warning light
      buzzer_sound();
      digitalWrite(red_led_pin, HIGH);
      delay(3000);
      // Shutdown and restart engine
      eng_shutdown();
      digitalWrite(red_led_pin, LOW);
      delay(5000);
      eng_start();
    }
    else if (incomingString.equals("OVR SPEED")){
      Serial.print("received:");
      Serial.println("OVR SPEED");
      analogWrite(eng_pin, over_speed);
      lcd.clear();
      // Centre text across 2 lines
      lcd.setCursor(3, 0);
      lcd.print("Engine Over");
      lcd.setCursor(5, 1);
      lcd.print("Speed!");
      buzzer_sound();
      digitalWrite(red_led_pin, HIGH);
      delay(3000);
      eng_shutdown();
      digitalWrite(red_led_pin, LOW);
      delay(5000);
      eng_start(); 
    }
    else if (incomingString.equals("OIL TMP")){
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Oil Temp");
      lcd.setCursor(5, 1);
      lcd.print("High!");
      buzzer_sound();
      digitalWrite(orange_led_pin, HIGH);
      delay(3000);
      digitalWrite(orange_led_pin, LOW);
      eng_shutdown();
      delay(5000);
      eng_start();
    }
    else if (incomingString.equals("ENG FIRE")){
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Engine Fire!");
      buzzer_sound();
      led_flash();
      delay(3000);
      eng_shutdown();
      delay(5000);
      eng_start();
    }
    // Calculate RPM and display it
    else if(incomingString.equals("SHOW RPM")){
      int rpm = float(normal_speed) / float(normal_speed)  * 100.0;
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print(rpm);
      lcd.print("\% RPM");
    }
    // Receive EGT temp and display it
    else if (incomingString.equals("SHOW EGT")){
      float egt = Serial.parseFloat();
      if (egt != 0){
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("EGT: ");
        lcd.print(egt);
        lcd.print("C");
      }
    }
    // Read a message like "FAULT 1708732800 Engine Fire",
    // use the number as the time, and show the fault name with that date/time on the LCD.
    else if (incomingString.startsWith("FAULT ")){
      int firstSpace = incomingString.indexOf(' ');
      int secondSpace = incomingString.indexOf(' ', firstSpace + 1);
      unsigned long epochTime = incomingString.substring(firstSpace + 1, secondSpace).toInt();
      String name = incomingString.substring(secondSpace + 1);
      setTime(epochTime);
      // Print fault name
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(name);
      // Print fault time (hh:mm) + day
      lcd.setCursor(0, 1);
      if(hour() < 10) lcd.print('0');
      lcd.print(hour());
      lcd.print(":");
      if(minute() < 10) lcd.print('0');
      lcd.print(minute());
      lcd.print(" ");
      if(day() < 10) lcd.print('0');
      lcd.print(day());
      lcd.print("/");
      if(month() < 10) lcd.print('0');
      lcd.print(month());
      lcd.print("/");
      lcd.print(year());
    }
  }
}

// Gradually ramp up fan/motor PWM to normal_speed then display RPM
void eng_start() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Starting...");
  for(int i = 0; i <= normal_speed; i += 10){
    delay(100);
    analogWrite(eng_pin, i); 
  }
  analogWrite(eng_pin, normal_speed);
}  

// Display shutdown then gradually ramp down motor speed to 0
void eng_shutdown() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Shutting down...");
  for(int i = normal_speed; i > 0; i -= 10){
    delay(100);
    analogWrite(eng_pin, i); 
  }
}

// Flash red LED 5 times
void led_flash() {
  for(int i = 0; i < 5; i++){
    digitalWrite(red_led_pin, HIGH);
    delay(300);
    digitalWrite(red_led_pin, LOW);
    delay(300);
  }
}

// Sound buzzer 3 times
void buzzer_sound() {
  for(int i = 0; i < 3; i++){
    digitalWrite(buzzer_pin, HIGH);
    delay(500);
    digitalWrite(buzzer_pin, LOW);
    delay(500);
  }
}
