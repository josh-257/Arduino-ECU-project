# Jet Engine Control Unit Model 

A project using two Arduino boards to mimic some of the basic functions of a engine control unit. Controlling an LCD, fan/motor, LEDs, and a buzzer. The system reacts to button inputs and input from a temperature sensor.

![Tinker CAD Diagram](image/ECU_Project_diagram2.png)

## The Setup

**Board 1 (Controller Board)**
- Reads the analog voltage via the voltage divider made with a 10k Ohm resistor connected in series to a 10k Ohm NTC thermistor (displayed as a photoresistor in TinkerCAD).
- Converts voltage drop into temperature through Steinhart's two-point beta equation.
- Continuously monitors three fault-inducing button inputs that are programmed to cause a simulated engine fire, oil overtemp, and overspeed condition.
- Implemented button debouncing to filter out mechanical noise and prevent repeated signals for one press.
- Determines when a fault is present and begins a 10-second time-out.
- When a fault is triggered, the type of fault and exact time are recorded in EEPROM.
- Sends error messages and fault history over Serial communication to board 2.

**Board 2 (Display Board)**
- Runs a listening loop to interpret specific error messages sent from board 1.
- Displays the current RPM or temperature and each fault as they occur.
- LEDs and a buzzer provide visual and audio warnings.
- Controls a motor via PWM (Pulse Width Modulation) and safely shutsdown the motor in the event of a fault.
- Displays the fault history recorded on board 1.

## Hardware Required

- 2 Arduino boards (with enough GPIO pins)
- LCD and potentiometer
- LEDs and Buzzer
- Switches (to trigger faults and display fault history etc)
- Thermistor (for EGT reading)
- Capacitors, electrolytic and ceramic (to reduce noise that could affect LCD)
- Motor
- 5V motor power supply (I've used a 9v wall adapter connected to an Elegoo power supply module)
- DS1307 RTC Module (I've used ELEGOO's VO3 module, with SDA and SCL pins connected to the arduino SDA and SCL pins)
- 5V power supply for each board
- 2N2222 transistor (MOSFET for a higher power motor)
- Diode
- Wires and resistors (330 Ohm, 1k Ohm, 10k Ohm) 

## Installation

1. Clone repo 'git clone https://github.com/josh-257/Arduino-ECU-project.git'
2. Open both sketches in Arduino IDE:
  - 'Controller_Board.ino'
  - 'Display_Board.ino'
3. Upload sketch to respective board (Controller board = board 1, display board = board 2)
4. Make sure to check each GPIO pin matches the one specified in the code for each board, and connect the serial TX and RX pins to each board.
4. Apply power to the power supply for the motor first, then the display board, and finally apply power to the controller board.
5. Trigger each fault with the first three buttons, switch between RPM and temperature display with the 4th button, and display the fault history with the 5th button.
6. Shut down the circuit in the reverse order of starting it.

