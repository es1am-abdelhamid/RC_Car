#include <Servo.h>

// ================= SERVO =================
Servo SERVO_1;
Servo SERVO_2;

// ================= PID / SPEED SETTINGS =================
float Kp = 0.8;            // Increased Proportional (Higher accuracy)
float Kd = 1.2;            // Added Derivative (Reduces wobbling/overshoot)
int baseSpeed = 130;       // Decreased speed (was 170) for better control

int lastError = 0;         // Stores previous error for the D term

// ================= IR Sensors =================
int IR_FOLLOW_RIGHT_PIN = A5;
int IR_FOLLOW_LEFT_PIN  = A0;

// ================= Motor Driver =================
int H_BRIDGE_PIN_ENA = 5;
int H_BRIDGE_PIN_ENB = 6;
int H_BRIDGE_PIN_IN1 = 7;
int H_BRIDGE_PIN_IN2 = 8;
int H_BRIDGE_PIN_IN3 = 11;
int H_BRIDGE_PIN_IN4 = 12;

// ================= Ultrasonic =================
const int TRIG = 9;
const int ECHO = 10;

// ================= Variables =================
int DISTANCE;
long DURATION;
int POS_1 = 0;
int POS_2 = 0;

void setup() {
    Serial.begin(9600);
    pinMode(H_BRIDGE_PIN_ENA, OUTPUT);
    pinMode(H_BRIDGE_PIN_ENB, OUTPUT);
    pinMode(H_BRIDGE_PIN_IN1, OUTPUT);
    pinMode(H_BRIDGE_PIN_IN2, OUTPUT);
    pinMode(H_BRIDGE_PIN_IN3, OUTPUT);
    pinMode(H_BRIDGE_PIN_IN4, OUTPUT);
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    SERVO_1.attach(2);
    SERVO_2.attach(3);
    SERVO_1.write(0);
    SERVO_2.write(0);
}

void loop() {
    // 1. MEASURE DISTANCE
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    DURATION = pulseIn(ECHO, HIGH);
    DISTANCE = DURATION * 0.034 / 2;

    // 2. OBSTACLE AVOIDANCE
    if (DISTANCE > 0 && DISTANCE <= 14) {
        stopMotors();
        for (POS_1 = 0; POS_1 <= 70; POS_1 += 2) { SERVO_1.write(POS_1); delay(40); }
        for (POS_2 = 0; POS_2 <= 180; POS_2 += 4) { SERVO_2.write(POS_2); delay(20); }
        for (POS_2 = 180; POS_2 >= 0; POS_2 -= 4) { SERVO_2.write(POS_2); delay(20); }
        for (POS_1 = 70; POS_1 >= 0; POS_1 -= 2) { SERVO_1.write(POS_1); delay(40); }
    } 
    // 3. LINE FOLLOWER (PD CONTROL)
    else {
        int leftValue = analogRead(IR_FOLLOW_LEFT_PIN);
        int rightValue = analogRead(IR_FOLLOW_RIGHT_PIN);

        // Current Error
        int error = leftValue - rightValue;

        // Calculate Derivative (rate of change)
        int derivative = error - lastError;

        // PD Formula
        int correction = (error * Kp) + (derivative * Kd);

        // Save error for next loop
        lastError = error;

        // Adjust Speeds
        int leftSpeed  = baseSpeed - correction;
        int rightSpeed = baseSpeed + correction;

        leftSpeed  = constrain(leftSpeed, 0, 255);
        rightSpeed = constrain(rightSpeed, 0, 255);

        // Direction Forward
        digitalWrite(H_BRIDGE_PIN_IN1, LOW);
        digitalWrite(H_BRIDGE_PIN_IN2, HIGH);
        digitalWrite(H_BRIDGE_PIN_IN3, HIGH);
        digitalWrite(H_BRIDGE_PIN_IN4, LOW);

        analogWrite(H_BRIDGE_PIN_ENA, leftSpeed);
        analogWrite(H_BRIDGE_PIN_ENB, rightSpeed);
    }
    delay(5); // Faster loop for higher accuracy
}

void stopMotors() {
    analogWrite(H_BRIDGE_PIN_ENA, 0);
    analogWrite(H_BRIDGE_PIN_ENB, 0);
}