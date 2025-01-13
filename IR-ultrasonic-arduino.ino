#include <Arduino.h>

// PIN VARIABLES
int buttonPin = 7;
int buttonState = 0;
int LED_PIN = 13;

/***IRremote variables*************************************************************************/
#include <IRremote.h>

#define IR_RECEIVE_PIN 11

// IRremote Codes
#define IR_BUTTON_1 12
#define IR_BUTTON_2 24
#define IR_BUTTON_3 94
#define IR_BUTTON_PLAY_PAUSE 64
#define IR_BUTTON_POWER 69        // Power button
#define IR_BUTTON_REWIND 68       // Rewind button
#define IR_BUTTON_FAST_FORWARD 67 // Fast-forward button
#define IR_BUTTON_VOLUME_UP 70    // Volume Up
#define IR_BUTTON_VOLUME_DOWN 21  // Volume Down

/***Motor Driver variables*************************************************************************/
// TB6612FNG Motor Driver
// the right motor will be controlled by the motor A pins on the motor driver
const int PWMA = 5; // speed control pin on the motor driver for the right motor
const int AIN1 = 2; // control pin 1 on the motor driver for the right motor
const int AIN2 = 3; // control pin 2 on the motor driver for the right motor

// the left motor will be controlled by the motor B pins on the motor driver
const int PWMB = 6; // speed control pin on the motor driver for the left motor
const int BIN1 = 8; // control pin 1 on the motor driver for the left motor
const int BIN2 = 9; // control pin 2 on the motor driver for the left motor

// L293D mapping
const int IN1_M1 = AIN1; // Control pin 1 for the right motor (Motor A)
const int IN2_M1 = AIN2; // Control pin 2 for the right motor (Motor A)
const int PWM_M1 = PWMA; // Speed control for the right motor (Motor A)

const int IN1_M2 = BIN1; // Control pin 1 for the left motor (Motor B)
const int IN2_M2 = BIN2; // Control pin 2 for the left motor (Motor B)
const int PWM_M2 = PWMB; // Speed control for the left motor (Motor B)
int speed = 150;

// VARIABLES
int motorSpeed = 0; // starting speed for the motor

/***Servo and Ultrasonic*************************************************************************/
#include <Servo.h>
#define SERVO_PIN 4            // Pin to which the servo motor is attached
#define ULTRASONIC_TRIG_PIN 12 // Pin for triggering the ultrasonic pulse
#define ULTRASONIC_ECHO_PIN 10 // Pin for receiving the ultrasonic echo
// Define variables for time measurement and calculated distance
#define SOUND_SPEED 0.034 // Speed of sound in cm per microsecond (340 m/s)
long timeInterval;        // Variable to store pulse duration
int measuredDistance;     // Variable to store calculated distance
Servo motorControl;       // Servo object to control motor movement

unsigned long previousMillis = 0; // Stores the last time a servo step was taken
// const int sweepInterval = 80;         // Interval between servo movements (in milliseconds)
const int sweepInterval = 30;
int sweepDirection = 1; // 1 for increasing angle, -1 for decreasing
// int servoAngle = 15;    // Start angle of the servo
int servoAngle = 100; // Start angle of the servo

// Function declarations
void forward();
void backward();
void turnLeft();
void turnRight();
void stop();
void speedUp();
void speedDown();
void rightMotor(int speed);
void leftMotor(int speed);
int getDistance();
void getSonar();

void setup()
{
    // set the motor control pins as outputs
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMA, OUTPUT);

    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);

    Serial.begin(115200);                      // begin serial communication with the computer
    Serial.println("To infinity and beyond!"); // test the serial connection
    Serial.print("Motor Speed: ");             // print the speed that the motor is set to run at
    Serial.println(speed);

    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    pinMode(buttonPin, INPUT_PULLUP); // set this as a pullup to sense whether the switch is flipped

    // Servo and Ultrasonic Sensor
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT); // Configure the trigger pin as an output
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);  // Configure the echo pin as an input
    motorControl.attach(SERVO_PIN);       // Attach the servo object to its designated pin
    motorControl.write(servoAngle);
    measuredDistance = getDistance(); // Get distance again
                                      //    Serial.print(angle);        // Print angle while moving back
    Serial.print(",");                // Print separator
    Serial.print(measuredDistance);   // Print measured distance
    Serial.print(".");                // Print separator
}

void loop()
{
    if (IrReceiver.decode())
    {
        int command = IrReceiver.decodedIRData.command;
        Serial.println(command);

        // Handle the received IR command
        switch (command)
        {
        case IR_BUTTON_1:
            Serial.println("Pressed on button 1");
            speedDown();
            break;
        case IR_BUTTON_2:
            Serial.println("Pressed on button 2");
            speedUp();
            break;
        case IR_BUTTON_3:
            Serial.println("Pressed on button 3");
            break;
        case IR_BUTTON_PLAY_PAUSE:
            Serial.println("Pressed on button Play/Pause");
            stop();
            break;
        case IR_BUTTON_POWER:
            Serial.println("Pressed on button Power");
            break;
        case IR_BUTTON_REWIND:
            Serial.println("Pressed on button Rewind");
            turnLeft();
            break;
        case IR_BUTTON_FAST_FORWARD:
            Serial.println("Pressed on button Fast Forward");
            turnRight();
            break;
        case IR_BUTTON_VOLUME_UP:
            Serial.println("Pressed on button Volume Up");
            forward();
            break;
        case IR_BUTTON_VOLUME_DOWN:
            Serial.println("Pressed on button Volume Down");
            backward();
            break;
        default:
            Serial.print("Unknown button code received: ");
            Serial.println(command);
            break;
        }

        IrReceiver.resume(); // Prepare for the next IR signal
    }

    // "E-stop" button
    const int debounceDelay = 50; // 50ms debounce
    static unsigned long lastDebounceTime = 0;
    static int lastButtonState = HIGH;
    int currentState = digitalRead(buttonPin);
    if (currentState != lastButtonState)
    {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (currentState == LOW)
        {
            stop();
            digitalWrite(LED_PIN, HIGH);
        }
    }
    lastButtonState = currentState;

    // Servo and Ultrasonic
    if (measuredDistance < 1)
    { // Stop if an obstacle is detected
        stop();
        Serial.println("Obstacle detected! Stopping...");
    }
    // Sweep servo non-blocking
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= sweepInterval)
    {
        previousMillis = currentMillis;
        motorControl.write(servoAngle); // Move servo to the current angle
        getSonar();
        // Update the servo angle for the next step
        servoAngle += sweepDirection * 1; // Increment angle by 5 degrees
        if (servoAngle <= 15 || servoAngle >= 165)
        {
            sweepDirection *= -1; // Reverse direction at boundaries
        }
    }
}

// Motor control functions
void forward()
{
    Serial.println("forward");
    // analogWrite(PWM_M1, speed);
    // analogWrite(PWM_M2, speed);
    // digitalWrite(IN1_M1, HIGH);
    // digitalWrite(IN2_M1, LOW);
    // digitalWrite(IN1_M2, HIGH);
    // digitalWrite(IN2_M2, LOW);
    leftMotor(speed);
    rightMotor(speed);
}

void backward()
{
    Serial.println("backward");
    analogWrite(PWM_M1, speed);
    analogWrite(PWM_M2, speed);
    digitalWrite(IN1_M1, LOW);
    digitalWrite(IN2_M1, HIGH);
    digitalWrite(IN1_M2, LOW);
    digitalWrite(IN2_M2, HIGH);

    // back up
    // rightMotor(-255);
    // leftMotor(-255);
}

void turnLeft()
{
    Serial.println("turnLeft");
    // analogWrite(PWM_M1, 0);
    // analogWrite(PWM_M2, speed);
    // digitalWrite(IN1_M1, HIGH);
    // digitalWrite(IN2_M1, LOW);
    // digitalWrite(IN1_M2, HIGH);
    // digitalWrite(IN2_M2, LOW);

    // Temp workaround
    leftMotor(-speed);
    rightMotor(speed);
}

void turnRight()
{
    Serial.println("turnRight");
    // analogWrite(PWM_M1, speed);
    // analogWrite(PWM_M2, 0);
    // digitalWrite(IN1_M1, HIGH);
    // digitalWrite(IN2_M1, LOW);
    // digitalWrite(IN1_M2, HIGH);
    // digitalWrite(IN2_M2, LOW);

    // Temp workaround
    leftMotor(speed);
    rightMotor(-speed);
}

void stop()
{
    Serial.println("stop");
    //    digitalWrite(IN1_M1, LOW);
    //    digitalWrite(IN2_M1, LOW);
    //    digitalWrite(IN1_M2, LOW);
    //    digitalWrite(IN2_M2, LOW);

    rightMotor(0);
    leftMotor(0);
}

void speedUp()
{
    Serial.println("speedUp");
    speed += 10;
    if (speed > 255)
        speed = 255;
    analogWrite(PWM_M1, speed);
    analogWrite(PWM_M2, speed);
}

void speedDown()
{
    Serial.println("speedDown");
    speed -= 10;
    if (speed < 0)
        speed = 0;
    analogWrite(PWM_M1, speed);
    analogWrite(PWM_M2, speed);
}

/********************************************************************************/
void leftMotor(int motorSpeed) // function for driving the left motor
{
    if (motorSpeed > 0) // if the motor should drive forward (positive speed)
    {
        digitalWrite(BIN1, HIGH); // set pin 1 to high
        digitalWrite(BIN2, LOW);  // set pin 2 to low
    }
    else if (motorSpeed < 0) // if the motor should drive backward (negative speed)
    {
        digitalWrite(BIN1, LOW);  // set pin 1 to low
        digitalWrite(BIN2, HIGH); // set pin 2 to high
    }
    else // if the motor should stop
    {
        digitalWrite(BIN1, LOW); // set pin 1 to low
        digitalWrite(BIN2, LOW); // set pin 2 to low
    }
    analogWrite(PWMB, abs(motorSpeed)); // now that the motor direction is set, drive it at the entered speed
}

/********************************************************************************/
void rightMotor(int motorSpeed) // function for driving the right motor
{
    if (motorSpeed > 0) // if the motor should drive forward (positive speed)
    {
        digitalWrite(AIN1, HIGH); // set pin 1 to high
        digitalWrite(AIN2, LOW);  // set pin 2 to low
    }
    else if (motorSpeed < 0) // if the motor should drive backward (negative speed)
    {
        digitalWrite(AIN1, LOW);  // set pin 1 to low
        digitalWrite(AIN2, HIGH); // set pin 2 to high
    }
    else // if the motor should stop
    {
        digitalWrite(AIN1, LOW); // set pin 1 to low
        digitalWrite(AIN2, LOW); // set pin 2 to low
    }
    analogWrite(PWMA, abs(motorSpeed)); // now that the motor direction is set, drive it at the entered speed
}

/********************************************************************************/
// Function to calculate the distance from ultrasonic sensor
int getDistance()
{
    // Start trigger pulse
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW); // Set trigger pin to LOW to reset
    delayMicroseconds(2);                   // Short delay to ensure clean pulse

    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH); // Start a pulse with trigger HIGH
    delayMicroseconds(10);                   // Pulse duration of 10 microseconds
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);  // End pulse by setting LOW

    timeInterval = pulseIn(ULTRASONIC_ECHO_PIN, HIGH); // Measure time of pulse

    // Calculate distance (in cm) based on time and speed of sound
    measuredDistance = timeInterval * SOUND_SPEED / 2;
    return measuredDistance; // Return calculated distance
}

void getSonar()
{
    measuredDistance = getDistance(); // Get distance again
    Serial.print(servoAngle);         // Print angle while moving back
    Serial.print(",");                // Print separator
    Serial.print(measuredDistance);   // Print measured distance
    Serial.print(".");                // Print separator
}