// ---------------- PIN DEFINITIONS ----------------
#define IR_LEFT A1     
#define IR_RIGHT A0    
#define IR_WALL A2      

#define RGB_R 12
#define RGB_G 11
#define RGB_B 10

#define BUZZER 8       

// MOTOR DRIVER
#define MOTOR_IN1 3     
#define MOTOR_IN2 4     
#define MOTOR_IN3 5    
#define MOTOR_IN4 6   
#define MOTOR_ENA 2     
#define MOTOR_ENB 7    

// ---------------- VARIABLES ----------------

// STEP CONTROL
unsigned long lastStepTime = 0;
const unsigned long stepInterval = 180;
int stepSpeed = 140;

// TURN CONTROL (🔥 FIX)
int lastTurn = 0; // -1 = left, 1 = right
unsigned long turnLockTime = 0;
const unsigned long turnLockDuration = 500; // longer hold

// OBSTACLE STOP
bool obstacleStop = false;
unsigned long stopStartTime = 0;
int stopCount = 0;

// RGB
unsigned long lastRGBTime = 0;
int rgbState = 0;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);

  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(IR_WALL, INPUT);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(MOTOR_ENB, OUTPUT);

  stopMotors();
  digitalWrite(BUZZER, LOW);
}

// ---------------- LOOP ----------------
void loop() {

  unsigned long now = millis();

  int leftIR = digitalRead(IR_LEFT);   
  int rightIR = digitalRead(IR_RIGHT);
  int wallIR = digitalRead(IR_WALL);   // HIGH = obstacle (inverted)

  // -------- OBSTACLE DETECTION --------
  if (wallIR == HIGH && !obstacleStop) {

    obstacleStop = true;
    stopStartTime = now;
    stopCount++;

    Serial.print("STOP #: ");
    Serial.println(stopCount);

    stopMotors();
  }

  // -------- STOP STATE --------
  if (obstacleStop) {

    stopMotors();

    if (now - lastRGBTime >= 100) {
      lastRGBTime = now;
      rgbState = (rgbState + 1) % 3;
    }

    setRGB(0,0,0);

    if (rgbState == 0) digitalWrite(RGB_R, HIGH);
    else if (rgbState == 1) digitalWrite(RGB_G, HIGH);
    else digitalWrite(RGB_B, HIGH);

    digitalWrite(BUZZER, LOW); // OFF

    if (now - stopStartTime >= 5000) {
      obstacleStop = false;
      setRGB(0,0,0);
      Serial.println("Resuming...");
    }

    return;
  }

  // -------- NORMAL MODE --------
  digitalWrite(BUZZER, LOW);

  if (now - lastStepTime >= stepInterval) {
    lastStepTime = now;
    stepLineFollow(leftIR, rightIR, now);
  }
}

// ---------------- LINE FOLLOW (FIXED TURN LOGIC) ----------------
void stepLineFollow(int leftIR, int rightIR, unsigned long now) {

  bool lockActive = (now - turnLockTime < turnLockDuration);

  // 🔥 CASE 1: BOTH WHITE
  if (leftIR == HIGH && rightIR == HIGH) {

    // Continue previous turn instead of switching
    if (lastTurn == -1) pulseLeft();
    else if (lastTurn == 1) pulseRight();
    else pulseForward();
  }

  // 🔥 LEFT detects white → turn LEFT
  else if (leftIR == HIGH) {

    if (!lockActive) { // only update if not locked
      lastTurn = -1;
      turnLockTime = now;
    }

    pulseLeft();
  }

  // 🔥 RIGHT detects white → turn RIGHT
  else if (rightIR == HIGH) {

    if (!lockActive) {
      lastTurn = 1;
      turnLockTime = now;
    }

    pulseRight();
  }

  // 🔥 BOTH BLACK → go forward
  else {
    lastTurn = 0;
    pulseForward();
  }
}

// ---------------- STEP MOVEMENT ----------------
void pulseForward() {
  motorForward(stepSpeed);
  delay(25);
  stopMotors();
}

void pulseLeft() {
  motorLeft(stepSpeed);
  delay(35);
  stopMotors();
}

void pulseRight() {
  motorRight(stepSpeed);
  delay(35);
  stopMotors();
}

// ---------------- MOTOR CONTROL ----------------
void motorForward(int speed) {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  analogWrite(MOTOR_ENA, speed);

  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
  analogWrite(MOTOR_ENB, speed);
}

void motorLeft(int speed) {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, speed);

  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
  analogWrite(MOTOR_ENB, speed);
}

void motorRight(int speed) {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  analogWrite(MOTOR_ENA, speed);

  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_ENB, speed);
}

void stopMotors() {
  analogWrite(MOTOR_ENA, 0);
  analogWrite(MOTOR_ENB, 0);
}

// ---------------- RGB ----------------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(RGB_R, r);
  digitalWrite(RGB_G, g);
  digitalWrite(RGB_B, b);
}
