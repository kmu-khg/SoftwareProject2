const int LED_PIN = 7;
int PWM_period = 0;
long high_time = 0;
long low_time = 0;

void set_period(int period) {
  PWM_period = period;
}

void set_duty(int duty) {
  high_time = (long)PWM_period * duty / 100;
  low_time = PWM_period - high_time;
}

void PWM() {
  digitalWrite(LED_PIN, HIGH);
  delayMicroseconds(high_time);
  digitalWrite(LED_PIN, LOW);
  delayMicroseconds(low_time);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);

  Serial.println("주기를 입력하세요. (100 ~ 10000)(unit: us): ");
  while(Serial.available() == 0) {}
  set_period(Serial.parseInt());

  Serial.print("주기가 ");
  Serial.print(PWM_period);
  Serial.print(" us 로 설정되었습니다.\n");
  
}

void loop() {
  long repeat_count = 500000 / PWM_period;
  int PWM_duty = 0;
  for(long i=0; i < repeat_count; i++) {
    PWM_duty = (int)((i*100.0) / repeat_count + 0.5);
    set_duty(PWM_duty);
    PWM();
  }
  for(long i = 0; i < repeat_count; i++) {
    PWM_duty = (int)(100 - ((i*100.0) / repeat_count - 0.5));
    set_duty(PWM_duty);
    PWM();
  }
}
