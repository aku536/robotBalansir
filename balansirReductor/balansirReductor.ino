#include "motor.h"
#include "gyro_acsel.h"

//=================================================================

//начальное смещение нуля баланса.
double zeroDeviation = 3;

//уменьшаем частоту ШИМ 
//до 200Гц или 60Гц
//====================================
//8 бит, 244,14 Гц
//для 5 и 6 пин
//TCCR0B = TCCR0B & 0b11111000 | 0x04;
//=====================================
//8 бит, 61,04 Гц
//TCCR0B = TCCR0B & 0b11111000 | 0x05;
//=====================================

void setup()
{

  motor1.setSpeed(255);
  motor2.setSpeed(255);
  _stop();
  giroscop_setup();
  Serial.begin(115200);
  _stop();
}

void loop()
{
  int i = 0;
  uint32_t timer = micros();
  uint32_t timer2;
  double alfa = 0.001;
  uint8_t f, b; f = b = 0;
  int balansir = 0;
  float  AcXsum = 0;
  float  GyYsum = 0;

  while (true)
  {
    if (GyYsum > 1.0)
    {
      backward(); if ((b > 2) && (GyYsum < 5.0)) {
        b = 0;
        _stop();
      }
      else if ((b > 1) && (GyYsum < 2.0)) {
        b = 0;
        _stop();
      }
      f = 0; b++;
      balansir++;
    }
    else if (GyYsum < -1.0)
    {
      forward();
      if ((f > 2) && (GyYsum > -5.0)) {
        f = 0;
        _stop();
      }
      else if ((f > 1) && (GyYsum > -2.0)) {
        f = 0;
        _stop();
      }
      b = 0; f++;
      balansir--;
    }
    else {
      _stop();
    }
    //Запрос данных с MPU-6050
    Data_mpu6050();
    //Расчет угла по показаниям акселерометра
    // с учетом корректировки точки равновесия balancing_zerro.
    AcXsum = (atan2(AcX, AcZ)) * RAD_TO_DEG + zeroDeviation;
    // Измерение наклона по Х.
    // Использование Комплементарного фильтра,
    // alfa - коэффициент фильтра.
    alfa = 0.001;
    timer2 = timer;
    timer = micros();
    GyYsum = (1 - alfa) * (GyYsum - ((double)GyY * (double)(timer - timer2)) / 131000000.0) + alfa * AcXsum;

//    if (balansir <-1) {
//      GyYsum -= 0.08;
//      balancing_zerro -= 0.006;
//      balansir = 0;
//      //balansir++;
//    }
//    if (balansir > 1) {
//      GyYsum += 0.08;
//      balancing_zerro += 0.006;
//      balansir = 0; 
//      //balansir--;
//    }
    
     i++;
    if(i==500){
      Serial.print("GyYsum="); Serial.println( GyYsum ); 
      Serial.print("balancing_zerro="); Serial.println( zeroDeviation ); 
    i=0;}
  }
}
