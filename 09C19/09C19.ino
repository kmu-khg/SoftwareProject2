#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define _DIST_MIN 100
#define _DIST_MAX 300

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

#define _EMA_ALPHA 0.5

#define MEDIAN_SIZE 10

unsigned long last_sampling_time;
float dist_prev = _DIST_MAX;
float dist_ema;

float median_arr[MEDIAN_SIZE];  
int   median_idx = 0;    
int   median_count = 0;    

void median_add(float x) {
  median_arr[median_idx] = x;
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
        float a = sorted_arr[j];
        sorted_arr[j] = sorted_arr[j + 1];
        sorted_arr[j + 1] = a;
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
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);


}

void loop() {
  float dist_raw, dist_filtered, dist_median;

  if (millis() < last_sampling_time + INTERVAL)
    return;

  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);

  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX)) {
      dist_filtered = dist_prev;
  } else {
      dist_filtered = dist_raw;
      dist_prev = dist_raw;
  }

  median_add(dist_raw);
  dist_median = median_get();

  dist_ema = (_EMA_ALPHA * dist_filtered) + ((1 - _EMA_ALPHA) * dist_ema);

  Serial.print("Min:");     Serial.print(_DIST_MIN);
  Serial.print(",raw:");    Serial.print(dist_raw);
  Serial.print(",ema:");    Serial.print(dist_ema);
  Serial.print(",median:"); Serial.print(dist_median);
  Serial.print(",Max:");    Serial.print(_DIST_MAX);
  Serial.println("");


  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, 1);
  else
    digitalWrite(PIN_LED, 0);

  last_sampling_time += INTERVAL;
}

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
