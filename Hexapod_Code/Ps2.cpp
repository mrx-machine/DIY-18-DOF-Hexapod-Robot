#include <Arduino.h>
#include "header.h"

// ================================================================
// ===                           PS2                            ===
// ================================================================
// Заменяет bluetooth.cpp. Заполняет те же переменные, что раньше
// заполнял Bluetooth_map(): directionInput, rotationInput и команду
// "встать / сесть" (standCommand).
//
// Управление:
//   левый стик   - ходьба (вперёд/назад и вбок)
//   правый стик  - поворот (по горизонтали)
//   START        - встать / сесть
//
// Если пульт пропал, робот останавливается и ждёт переподключения.

#ifdef PS2_CONTROL

#include <PS2X_lib.h>

// пины приёмника (как распаяно)
#define PS2_CLK 8
#define PS2_CMD 11
#define PS2_ATT 10
#define PS2_DAT 12

// мёртвая зона стика (доля полного хода, 0..1)
#define PS2_DEADZONE 0.12f

// направление осей: если робот идёт/поворачивает не туда, поменяй знак на -1
#define PS2_FWD_SIGN 1
#define PS2_SIDE_SIGN -1
#define PS2_ROT_SIGN -1

// высота корпуса (groundClearance), мм
#define BODY_MIN 10.0f
#define BODY_MAX 70.0f
#define BODY_RATE 30.0f // скорость изменения, мм/с

// кривая отклика: 0 = линейно, 1 = очень мягко у центра
#define PS2_EXPO 0.4f

// плавность: ускорение и торможение, мм/с²
#define PS2_ACCEL 600.0f
#define PS2_DECEL 1200.0f

// множители скорости на курках
#define PS2_SLOW_MULT 0.4f // L2
#define PS2_FAST_MULT 1.6f // R2

// сколько пульт может молчать, прежде чем робот сядет
#define PS2_LOST_SIT_MS 5000


PS2X ps2;

bool standCommand = false; // true = стоять, false = сидеть
bool homeCommand = false;  // true = вернуть ноги в центр
bool waveCommand = false;

static bool ps2Connected = false;
static bool needCenter = true;            // пока стики не в центре, движение запрещено
static unsigned long disconnectedSince = 0; // когда пропала связь (0 = связь есть)

bool Ps2_connected()
{
    return ps2Connected;
}

static void Ps2_connect()
{
    ps2Connected = (ps2.config_gamepad(PS2_CLK, PS2_CMD, PS2_ATT, PS2_DAT, false, false) == 0);

    if (ps2Connected)
    {
        needCenter = true; // после (пере)подключения ждём, пока стики вернутся в центр
        disconnectedSince = 0;
    }
}

void Ps2_init()
{
    Ps2_connect();
}

// сырое значение стика 0..255 -> -1..1: мёртвая зона + кривая отклика
static float stickValue(byte raw)
{
    float v = (float(raw) - 127.5f) / 127.5f;
    float a = fabs(v);

    if (a < PS2_DEADZONE)
        return 0.0f;

    a = (a - PS2_DEADZONE) / (1.0f - PS2_DEADZONE);
    a = (1.0f - PS2_EXPO) * a + PS2_EXPO * a * a * a; // мягче у центра, полный ход в конце

    return (v < 0) ? -a : a;
}

// плавно подтягиваем текущие directionInput/rotationInput к целевым
static void applySlew(const Vector2 &targetDir, float targetRot, float dt)
{
    // направление
    Vector2 delta = targetDir - directionInput;
    float d = delta.magnitude();
    float rate = (targetDir.magnitude() < directionInput.magnitude()) ? PS2_DECEL : PS2_ACCEL;
    float maxStep = rate * dt;

    if (d <= maxStep)
        directionInput = targetDir;
    else
        directionInput = directionInput + delta * (maxStep / d);

    // поворот
    float dr = targetRot - rotationInput;
    float rateR = (fabs(targetRot) < fabs(rotationInput)) ? PS2_DECEL : PS2_ACCEL;
    float maxStepR = rateR * dt;

    if (fabs(dr) <= maxStepR)
        rotationInput = targetRot;
    else
        rotationInput += (dr > 0 ? maxStepR : -maxStepR);
}

void Ps2_read()
{
    static unsigned long lastTry = 0;
    static unsigned long lastCall = 0;

    unsigned long now = millis();
    float dt = float(now - lastCall) / 1000.0f;
    lastCall = now;
    if (dt > 0.05f)
        dt = 0.05f; // после блокирующих функций (sitDown, homeLegs) не даём скачка

    Vector2 targetDir = Vector2::zero;
    float targetRot = 0.0f;

    // ---------- пульта нет ----------
    if (!ps2Connected)
    {
        if (disconnectedSince == 0)
            disconnectedSince = now;

        // долго нет связи: садимся, чтобы сервы не грелись
        if (now - disconnectedSince > PS2_LOST_SIT_MS)
            standCommand = false;

        applySlew(targetDir, targetRot, dt); // плавно тормозим

        if (now - lastTry > 1000)
        {
            lastTry = now;
            Ps2_connect();
        }
        return;
    }

    // ---------- связь пропала ----------
    if (!ps2.read_gamepad(false, 0))
    {
        ps2Connected = false;
        disconnectedSince = now;
        needCenter = true;
        applySlew(targetDir, targetRot, dt);
        return;
    }

    // ---------- стики ----------
    float fwd = -stickValue(ps2.Analog(PSS_LY)) * PS2_FWD_SIGN;
    float side = stickValue(ps2.Analog(PSS_LX)) * PS2_SIDE_SIGN;
    float rot = stickValue(ps2.Analog(PSS_RX)) * PS2_ROT_SIGN;

    // запрет движения, пока стики не побывали в центре
    if (needCenter)
    {
        if (fwd == 0 && side == 0 && rot == 0)
            needCenter = false;
        fwd = side = rot = 0;
    }

    // множитель скорости: L2 медленно, R2 быстро (обе сразу = обычная)
    float mult = 1.0f;
    if (ps2.Button(PSB_L2) && !ps2.Button(PSB_R2))
        mult = PS2_SLOW_MULT;
    else if (ps2.Button(PSB_R2) && !ps2.Button(PSB_L2))
        mult = PS2_FAST_MULT;

    float limit = maxSpeed * mult;

    targetDir = Vector2(fwd * limit, side * limit);
    float mag = targetDir.magnitude();
    if (mag > limit)
        targetDir = targetDir * (limit / mag); // по диагонали не быстрее лимита

    targetRot = rot * limit;

    applySlew(targetDir, targetRot, dt);

    // ---------- кнопки ----------
    // аварийная посадка: L1 + R1 + КРЕСТ
    if (ps2.Button(PSB_L1) && ps2.Button(PSB_R1) && ps2.Button(PSB_CROSS))
        standCommand = false;

    // START: встать / сесть
    if (ps2.ButtonPressed(PSB_START))
        standCommand = !standCommand;

    // SELECT: вернуть ноги в центр
    if (ps2.ButtonPressed(PSB_SELECT))
        homeCommand = true;

    // крестовина вверх/вниз: высота корпуса
    if (HexapodState != State::SITTING)
    {
        if (ps2.Button(PSB_PAD_UP))
            groundClearance = fmin(groundClearance + BODY_RATE * dt, BODY_MAX);
        if (ps2.Button(PSB_PAD_DOWN))
            groundClearance = fmax(groundClearance - BODY_RATE * dt, BODY_MIN);
    
    }
    // TRIANGLE: приветствие
    if (ps2.ButtonPressed(PSB_TRIANGLE))
        waveCommand = true;

}

#endif