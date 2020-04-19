#include <Wire.h>

//===============================================

//Коррекция склонения для акселерометра по Y
#define ACYSUMCORRECTION -0.95

//Коррекция склонения для акселерометра по X
#define ACXSUMCORRECTION -2.17

int PROG;

// упрощеный I2C адрес нашего гироскопа/акселерометра MPU-6050.
const int MPU_addr = 0x68;

// переменные для хранения данных возвращаемых прибором.
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float AcYsum, AcXsum, GyXsum, GyYsum;

// Угол поворота вокруг оси Z.
float Ang_;

// Ускорения по осям  - для анализа наклона
float AcZtec, AcYtec, AcXtec;
float CompensatorZ, CompensatorX, CompensatorY,  CompensatorAcX;

unsigned long timer = 0;

//===============================================

// Считывание данных с mpu6050
void Data_mpu6050()
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  //Готовим для чтения регистры  с адреса 0x3B.
  Wire.endTransmission(false);
  // Запрос 14 регистров.
  Wire.requestFrom(MPU_addr, 14, true);
  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcX = Wire.read() << 8 | Wire.read();
  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcY = Wire.read() << 8 | Wire.read();
  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  AcZ = Wire.read() << 8 | Wire.read();
  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  Tmp = Wire.read() << 8 | Wire.read();
  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyX = Wire.read() << 8 | Wire.read();
  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyY = Wire.read() << 8 | Wire.read();
  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  GyZ = Wire.read() << 8 | Wire.read();
}

// Запуск гироскопа
void giroscop_setup()
{
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // Производим запись в регистр энергосбережения MPU-6050
  Wire.write(0);     // устанавливем его в ноль
  Wire.endTransmission(true);
  Ang_ = 0;
}

// Задержка, но с расчетом изменения Z угла
void time_gyro(float mill_sec)
{
  unsigned long ms = mill_sec;
  unsigned long timer2;
  float m_t_d131;
  if (timer == 0) timer = micros();
  unsigned long endtime = timer + long(ms * 1000.0);
  do {
    Data_mpu6050();
    timer2 = timer;
    timer = micros();
    m_t_d131 = (float)(timer - timer2) / 131000000.0;
    Ang_ = Ang_ + ((float)(GyZ) - CompensatorZ) * m_t_d131;
    AcYsum = (atan2(AcY, AcZ)) * RAD_TO_DEG + ACYSUMCORRECTION; //balancing_zerro;
    // Измерение наклона по Х.
    // Использование Комплементарного фильтра,
    // alfa - коэффициент фильтра.
    float alfa = 0.005;
    AcXtec = (1.0 - alfa) * (AcXtec + ((float)GyX - CompensatorX) * m_t_d131) + alfa * AcYsum;
    AcXsum = - (atan2(AcX, AcZ)) * RAD_TO_DEG + ACXSUMCORRECTION; //+balancing_zerro;
    // Измерение наклона по Y.
    // Использование Комплементарного фильтра,
    // alfa - коэффициент фильтра.
    AcYtec = (1.0 - alfa) * (AcYtec + ((float)GyY - CompensatorY) * m_t_d131) + alfa * AcXsum;
  } while (endtime > timer);
}

// Компенсация нуля гироскопа
void Calc_CompensatorZ(float mill_sec)
{
  long ms = mill_sec;
  float i = 0;
  CompensatorZ = 0;
  CompensatorX = 0;
  CompensatorY = 0;
  timer = micros();
  unsigned long endtime = micros() + long(ms * 1000.0);
  while (endtime > timer) {
    CompensatorZ += (float)(GyZ);
    CompensatorX += (float)(GyX);
    CompensatorY += (float)(GyY);
    timer = micros();
    Data_mpu6050();
    delayMicroseconds(100);
    i++;
  }
  CompensatorZ /= i;
  CompensatorX /= i;
  CompensatorY /= i;
}
