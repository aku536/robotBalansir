#include <AFMotor.h>

AF_DCMotor motor1(1);
AF_DCMotor motor2(3);

//===============================================

// Направление движения условно и зависит от расположения проводов на двинателе.
// Движение вперед
void forward()
{
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
}

// Движение назад.
void backward()
{
  motor1.run(FORWARD);
  motor2.run(FORWARD);
}

// Остановка движения двигателей
void _stop()
{
  motor1.run(RELEASE);
  motor2.run(RELEASE);
}
