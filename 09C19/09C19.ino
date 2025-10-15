#include <Servo.h>
#include <math.h>

// --- 핀 설정 ---
#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

// --- 상수 ---
#define SOUND_VELOCITY 346.0   // m/s
#define INTERVAL 25            // 주기 (ms)
#define PULSE_DURATION 10
#define DIST_THRESHOLD 15.0    // 차량 감지 거리(cm)
#define SERVO_UP_ANGLE 90      // 차단기 열림 각도
#define SERVO_DOWN_ANGLE 0     // 차단기 닫힘 각도

Servo servo;
unsigned long last_sampling_time = 0;

// --- 함수 선택 (0: sigmoid / 1: sin) ---
int controlMode = 0;

// --- 초음파 거리 측정 함수 ---
float getDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout
  float distance = duration * 0.000001 * (SOUND_VELOCITY / 2.0) * 100.0; // cm
  return distance;
}

// --- 시그모이드 함수 기반 서보 제어 ---
void moveServoSigmoid(int startAngle, int endAngle) {
  float step = 0.1;
  float a = 6.0;  // 시그모이드 기울기
  for (float t = 0.0; t <= 1.0; t += step) {
    float sigmoid = 1.0 / (1.0 + exp(-a * (t - 0.5)));
    int angle = startAngle + (endAngle - startAngle) * sigmoid;
    servo.write(angle);
    delay(25);
  }
}

// --- 삼각함수 기반 서보 제어 ---
void moveServoSin(int startAngle, int endAngle) {
  float step = 0.05;
  for (float t = 0.0; t <= 1.0; t += step) {
    float smooth = (sin(PI * t - PI / 2) + 1.0) / 2.0;  // 0~1 부드러운 곡선
    int angle = startAngle + (endAngle - startAngle) * smooth;
    servo.write(angle);
    delay(25);
  }
}

// --- setup ---
void setup() {
  Serial.begin(9600);
  servo.attach(PIN_SERVO);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  servo.write(SERVO_DOWN_ANGLE);
  Serial.println("Parking Gate System Ready!");
  Serial.println("Control Mode: 0=Sigmoid, 1=Sin");
}

// --- loop ---
void loop() {
  float distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);

  // 모드 전환 (시리얼 입력으로 변경 가능)
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == '0') controlMode = 0;
    else if (ch == '1') controlMode = 1;
  }

  if (distance < DIST_THRESHOLD) {
    Serial.println("Car detected! Opening gate...");
    if (controlMode == 0) moveServoSigmoid(SERVO_DOWN_ANGLE, SERVO_UP_ANGLE);
    else moveServoSin(SERVO_DOWN_ANGLE, SERVO_UP_ANGLE);
    delay(2000);
  } 
  else {
    Serial.println("No car detected. Closing gate...");
    if (controlMode == 0) moveServoSigmoid(SERVO_UP_ANGLE, SERVO_DOWN_ANGLE);
    else moveServoSin(SERVO_UP_ANGLE, SERVO_DOWN_ANGLE);
  }

  delay(500);
}
