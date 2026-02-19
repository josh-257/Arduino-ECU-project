#include <LiquidCrystal.h>

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
    // Set Baud rate and GPIO pins
    Serial.begin(9600);
    pinMode(buzzer_pin, OUTPUT);
    pinMode(red_led_pin, OUTPUT);
    pinMode(orange_led_pin, OUTPUT);
    pinMode(eng_pin, OUTPUT);
    // Initialise LCD object
    lcd.begin(16, 2);
    // Wipe display
    lcd.clear();
    // Start up fan/motor
    eng_start();
}

void loop() {
    // Listen for engine faults 
    if (Serial.available()) {
        // Read fault 
        incomingString = Serial.readStringUntil('\n');
        incomingString.trim();
        // React to each engine fault
        if (incomingString == "EGT OVRTMP") {
            lcd.clear();
            // Centre text on the display
            lcd.setCursor(1, 0);
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

        else if (incomingString == "OVR SPEED") {
            analogWrite(eng_pin, over_speed);
            lcd.clear();
            // Center text across 2 lines
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

        else if (incomingString == "OIL TMP") {
            lcd.clear();
            lcd.setCursor(4, 0);
            lcd.print("Oil Temp");
            lcd.setCursor(5, 1);
            lcd.print("High!");
            buzzer_sound();
            org_led_flash();
            eng_shutdown();
            delay(5000);
            eng_start();
        }

        else if (incomingString == "ENG FIRE") {
            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("Engine Fire!");
            buzzer_sound();
            red_led_flash();
            delay(3000);
            eng_shutdown();
            delay(5000);
            eng_start();
        }
    }
}

// Gradually ramps up fan/motor PWM to normal_speed then displays RPM
void eng_start() {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Starting...");

    for (int i = 0; i <= normal_speed; i += 10) {
        delay(100);
        analogWrite(eng_pin, i);
    }

    analogWrite(eng_pin, normal_speed);

    int rpm = 100;
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(rpm);
    lcd.print("% RPM");
}

// Displays shutdown then gradually ramps down motor speed to 0
void eng_shutdown() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Shutting down...");

    for (int i = normal_speed; i > 0; i -= 10) {
        delay(100);
        analogWrite(eng_pin, i);
    }
}

// Flash red LED 5 times
void red_led_flash() {
    for (int i = 0; i < 5; i++) {
        digitalWrite(red_led_pin, HIGH);
        delay(300);
        digitalWrite(red_led_pin, LOW);
        delay(300);
    }
}

// Flash orange LED 5 times
void org_led_flash() {
    for (int i = 0; i < 5; i++) {
        digitalWrite(orange_led_pin, HIGH);
        delay(300);
        digitalWrite(orange_led_pin, LOW);
        delay(300);
    }
}

// Sound buzzer 3 times
void buzzer_sound() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(buzzer_pin, HIGH);
        delay(500);
        digitalWrite(buzzer_pin, LOW);
        delay(500);
    }
}
