#include <Arduino.h>
#include "header.h"

// ================================================================
// ===                          wave                            ===
// ================================================================
// Приветствие: одна нога поднимает фемур и тибию, потом тибия несколько раз
// опускается и поднимается (фемур стоит неподвижно), после чего нога
// возвращается в обычное положение.
//
// Суставы двигаются напрямую по углам (без IK), относительно текущих углов ноги.
// Запускается на стоящем роботе. Функция блокирующая: пока она работает,
// пульт не читается.

#ifdef SERVO

// ===== настройки приветствия: меняй под себя =====
#define WAVE_LEG 5          // какой ногой машем (0-5)
#define WAVE_FEMUR_UP 150    // на сколько градусов поднять фемур
#define WAVE_TIBIA_UP -70    // на сколько градусов поднять тибию вместе с фемуром
#define WAVE_TIBIA_SWING -50 // на сколько градусов тибия опускается и потом поднимается
#define WAVE_PAUSE_START_MS 300 // пауза после подъёма, перед маханием
#define WAVE_PAUSE_END_MS 300   // пауза после махания, перед опусканием ноги
#define WAVE_COUNT 3        // сколько раз махнуть
#define WAVE_STEP_MS 20     // пауза между шагами, мс (меньше = быстрее)
#define WAVE_STEPS_LIFT 20  // шагов на подъём и на возврат
#define WAVE_STEPS_SWING 15 // шагов на один взмах вниз или вверх

// если сустав идёт в обратную сторону, поменяй знак на -1
// (для зеркальных ног направление переворачивается автоматически)
#define WAVE_FEMUR_SIGN 1
#define WAVE_TIBIA_SIGN 1

// плавно ведёт фемур и тибию к заданным углам
static void moveJoints(int legId, float femur, float tibia, int steps)
{
    Leg_Struct &leg = Leg[legId];

    float f0 = leg.Servo[1].targetAngle;
    float t0 = leg.Servo[2].targetAngle;

    for (int s = 1; s <= steps; s++)
    {
        float k = float(s) / float(steps);

        leg.Servo[1].targetAngle = f0 + (femur - f0) * k;
        leg.Servo[2].targetAngle = t0 + (tibia - t0) * k;

        Servo_update(leg.Servo[1]);
        Servo_update(leg.Servo[2]);

        delay(WAVE_STEP_MS);
    }
}

void waveHello()
{
    Leg_Struct &leg = Leg[WAVE_LEG];

    // у зеркальных ног направление углов обратное
    float dir = leg.mirrored ? -1.0f : 1.0f;
    float dirF = dir * WAVE_FEMUR_SIGN;
    float dirT = dir * WAVE_TIBIA_SIGN;

    // текущее (обычное) положение ноги
    float baseF = leg.Servo[1].targetAngle;
    float baseT = leg.Servo[2].targetAngle;

    float upF = baseF + dirF * WAVE_FEMUR_UP;
    float upT = baseT + dirT * WAVE_TIBIA_UP;
    float downT = upT - dirT * WAVE_TIBIA_SWING;

    // поднять фемур и тибию
    moveJoints(WAVE_LEG, upF, upT, WAVE_STEPS_LIFT);

    delay(WAVE_PAUSE_START_MS); // пауза перед маханием

    // фемур неподвижен, тибия вниз-вверх
    for (int n = 0; n < WAVE_COUNT; n++)
    {
        moveJoints(WAVE_LEG, upF, downT, WAVE_STEPS_SWING);
        moveJoints(WAVE_LEG, upF, upT, WAVE_STEPS_SWING);
    }

    delay(WAVE_PAUSE_END_MS); // пауза перед опусканием ноги

    // вернуть ногу в обычное положение
    moveJoints(WAVE_LEG, baseF, baseT, WAVE_STEPS_LIFT);

    // синхронизировать с расчётом IK
    Leg_update(WAVE_LEG);
}

#endif