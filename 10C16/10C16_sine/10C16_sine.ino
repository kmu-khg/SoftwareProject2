#include <Servo.h>
#include <math.h>

#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

#define _EMA_ALPHA 0.5

#define MEDIAN_SIZE 10

unsigned long last_sampling_time;

float median_arr[MEDIAN_SIZE];  
int   median_idx = 0;    
int   median_count = 0;

Servo myServo;
unsigned long moveStartTime;
int startAngle = 0;
int stopAngle = 90;

bool gateOpen = false;

void median_push(float v) {
  median_arr[median_idx] = v;
  median_idx = (median_idx + 1) % MEDIAN_SIZE;
  if (median_count < MEDIAN_SIZE) median_count++;
}

float median_get() {
  if (median_count == 0) return 0.0;

  float sorted_arr[MEDIAN_SIZE];
  for (int i = 0; i < median_count; i++) {
    sorted_arr[i] = median_arr[i];
  }

  for (int i = 0; i < median_count - 1; i++) {
    for (int j = 0; j < median_count - i - 1; j++) {
      if (sorted_arr[j] > sorted_arr[j + 1]) {
        float temp = sorted_arr[j];
        sorted_arr[j] = sorted_arr[j + 1];
        sorted_arr[j + 1] = temp;
      }
    }
  }

  int mid = median_count / 2;
  if (median_count % 2 == 1) {
    return sorted_arr[mid];
  } else {
    return (sorted_arr[mid - 1] + sorted_arr[mid]) / 2.0;
  }
}

void setup() {
  Serial.begin(57600);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  myServo.attach(PIN_SERVO);

  myServo.write(startAngle);
  delay(500);
}

void loop() {
  float dist_raw, dist_median;
 
  if (millis() < last_sampling_time + INTERVAL)
    return;

  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);  

  median_push(dist_raw);
  dist_median = median_get();

  if (dist_median < 60 && gateOpen == false)
  {
    gateOpen = true;
    SineMove(startAngle, stopAngle, 2500);
    delay(2000);
  }

  if (dist_median > 80 && gateOpen == true)
  {
    SineMove(stopAngle, startAngle, 2500);
    gateOpen = false;
  }

  last_sampling_time += INTERVAL;
}

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}

void SineMove(int start_Angle, int stop_Angle, int duration) {
  int steps = 100;
  float delayTime = (float)duration / steps;
  float angleChange = stop_Angle - start_Angle;

  for (int i = 0; i <= steps; i++) {
    float input = (float)i / steps;

    float SineOutput = -(cos(M_PI * input) - 1.0) / 2.0;

    int currentAngle = start_Angle + (angleChange * SineOutput);

    myServo.write(currentAngle);
    delay(delayTime);
  }
}
