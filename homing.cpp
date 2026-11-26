#include <Arduino.h>
#include "header.h"

// ================================================================
// ===                          homing                          ===
// ================================================================
// Возвращает все ноги в центр (0, 0) на землю. Запускается кнопкой SELECT
// на стоящем роботе. Ноги идут по пути: подъём -> перенос в центр -> опускание.
//
// HOMING_TRIPOD включён: две группы по три ноги (0,2,4 и 1,3,5), как в ходьбе.
// Выключен: ноги по одной, медленнее, но пять ног всегда на земле.
//
// Функция блокирующая (как sitDown): пока она работает, пульт не читается.

#ifdef SERVO

#define HOMING_TRIPOD     // закомментируй, чтобы ноги возвращались по одной
#define HOMING_STEPS 30   // шагов на путь ноги (больше = плавнее и медленнее)
#define HOMING_STEP_MS 15 // пауза между шагами, мс

static float groundZ; // высота кончика ноги на земле
static float liftZ;   // высота кончика ноги в воздухе

// точка на пути ноги: t = 0..1 по длине пути (подъём, перенос в центр, опускание)
static Vector3 homePoint(const Vector3 &s, float t)
{
    float l1 = fabs(liftZ - s.z);          // подъём
    float l2 = s.xyPlane().magnitude();    // перенос в центр
    float l3 = fabs(liftZ - groundZ);      // опускание
    float d = t * (l1 + l2 + l3);

    if (d <= l1)
    {
        float f = (l1 > 0) ? d / l1 : 1.0f;
        return Vector3(s.x, s.y, s.z + (liftZ - s.z) * f);
    }
    d -= l1;

    if (d <= l2)
    {
        float f = (l2 > 0) ? d / l2 : 1.0f;
        return Vector3(s.x * (1.0f - f), s.y * (1.0f - f), liftZ);
    }
    d -= l2;

    float f = (l3 > 0) ? d / l3 : 1.0f;
    if (f > 1.0f)
        f = 1.0f;
    return Vector3(0.0f, 0.0f, liftZ + (groundZ - liftZ) * f);
}

// ведёт группу ног в центр одновременно; все ноги группы приходят в конец пути вместе
static void moveGroup(const int *legs, int count)
{
    Vector3 start[3];
    bool needs[3];
    bool any = false;

    for (int k = 0; k < count; k++)
    {
        start[k] = Leg[legs[k]].curPosition;
        // нога уже в центре на земле: не двигаем
        needs[k] = !(start[k].xyPlane().magnitude() < 3.0f && fabs(start[k].z - groundZ) < 3.0f);
        if (needs[k])
            any = true;
    }

    if (!any)
        return;

    for (int s = 1; s <= HOMING_STEPS; s++)
    {
        float t = float(s) / float(HOMING_STEPS);

        for (int k = 0; k < count; k++)
        {
            if (!needs[k])
                continue;

            Leg[legs[k]].targetPosition = homePoint(start[k], t);
            Leg_update(legs[k]);
        }
        delay(HOMING_STEP_MS);
    }
}

void homeLegs()
{
    groundZ = LENGTH_TIBIA - (groundClearance + 40);
    liftZ = groundZ + legLiftDistance;

#ifdef HOMING_TRIPOD
    const int groupA[3] = {0, 2, 4};
    const int groupB[3] = {1, 3, 5};
    moveGroup(groupA, 3);
    moveGroup(groupB, 3);
#else
    const int order[6] = {0, 3, 1, 4, 2, 5}; // чередуем стороны
    for (int n = 0; n < 6; n++)
    {
        moveGroup(&order[n], 1);
    }
#endif

    // сбрасываем состояние шага, чтобы следующая ходьба начиналась с чистого листа
    for (int i = 0; i < 6; i++)
    {
        Leg[i].lifted = false;
    }
}

#endif