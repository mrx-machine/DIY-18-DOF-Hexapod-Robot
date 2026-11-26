#include <Arduino.h>
#include "header.h"
#include <ArxContainer.h>

int loopTime;

Vector2 directionInput(0, 0);
float rotationInput = 0;
float groundClearance = 35;
float stepRadius = 50;
float maxSpeed = 400; // с PS2: скорость при полностью отклонённом стике

State HexapodState = State::SITTING;

#ifdef WS2812B_LED
int ledUpdateTimer = 0;
int brightness = 5;
int colorR = 0;
int colorG = 0;
int colorB = 255;
bool ledUpdate = true;
#endif

void setup()
{
  Serial.begin(9600);

#ifdef PS2_CONTROL
  Ps2_init();

  // не продолжаем, пока пульт не отзовётся (Ps2_read сам пробует переподключиться)
  while (!Ps2_connected())
  {
    Ps2_read();
    delay(10);
  }
#endif

#ifdef WS2812B_LED // initialitze LEDs
  Led_init();
#endif

#ifdef SERVO // initialite Servos
  Servo_init();
#endif

#ifdef WS2812B_LED // startup LED animation
  for (size_t i = 0; i < 25; i++)
  {
    Led_update(i);
    FastLED.show();
    delay(25);
  }

#ifdef SERVO
  for (int i = 0; i < 100; i++)
  {
    for (int o = 0; o < 6; o++)
    {
      LED_leg_animation(o, 0, float(i + 1) / 100.0f);
    }
    FastLED.show();

    delay(10);
  }
#endif

#endif

#ifdef SERVO
#ifdef PS2_CONTROL
  // ждём START, потом встаём
  while (!standCommand)
  {
    Ps2_read();
    delay(10);
  }

  standUp();
#else
  standUp();
#endif
#endif

#ifdef SERVO_CALIBRATION
  HexapodState = State::STANDING;
#endif
}

void loop()
{
  int long curTime = millis();

#ifdef PS2_CONTROL
  Ps2_read();
#endif

#ifdef SERVO

#ifdef PS2_CONTROL
  // встать / сесть по кнопке START
  if (!standCommand && (HexapodState == State::STANDING || HexapodState == State::WALKING))
  {
    sitDown();
    curTime = millis(); // sitDown занимает время, иначе loopTime собьётся
  }
  else if (standCommand && HexapodState == State::SITTING)
  {
    standUp();
    curTime = millis();
  }
#endif

  if (HexapodState == State::STANDING || HexapodState == State::WALKING)
  {
    walkCycle();
    Output_update();
  }

#ifdef PS2_CONTROL
  // SELECT: вернуть ноги в центр (только на стоящем роботе, стики в центре)
  if (homeCommand)
  {
    if (HexapodState == State::STANDING)
    {
      homeLegs();
      curTime = millis(); // homeLegs занимает время, иначе loopTime собьётся
    }
    homeCommand = false;
  }

  // TRIANGLE: приветствие (тоже только на стоящем роботе)
  if (waveCommand)
  {
    if (HexapodState == State::STANDING)
    {
      waveHello();
      curTime = millis();
    }
    waveCommand = false;
  }
#endif

#endif

#ifdef WS2812B_LED
  if (ledUpdate || (ledUpdateTimer > updateInterval && continuesUpdate))
  {
    for (size_t i = 0; i < NUM_LEDS; i++)
    {
      Led_update(i);
    }

    FastLED.setBrightness(brightness);
    FastLED.show();

    ledUpdate = false;
    ledUpdateTimer = 0;

    curTime = millis();
  }

  ledUpdateTimer += loopTime;
#endif

  loopTime = millis() - curTime;
  // Serial.println(loopTime); // для отладки
}