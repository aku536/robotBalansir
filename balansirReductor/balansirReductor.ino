#include "motor.h"
#include "mpu6050.h"

// Начальное смещение нуля баланса
double zeroDeviation = 3;

// Уменьшаем частоту ШИМ 
//до 200Гц или 60Гц

// 8 бит, 244,14 Гц
// для 5 и 6 пин
//TCCR0B = TCCR0B & 0b11111000 | 0x04;

// 8 бит, 61,04 Гц
//TCCR0B = TCCR0B & 0b11111000 | 0x05;

int i = 0;
uint32_t timer = micros();
uint32_t timer2;
double alfa = 0.001; // Коэффициент смешивания комплементарного фильтра
uint8_t f, b = 0; 
int balansir = 0;
float accelAngle = 0; // Угол наклона по акселерометру
float gyroAngle = 0; // Угол наклона по гироскопу


void setup() {
  motor1.setSpeed(255);
  motor2.setSpeed(255);
  setupGyro();
  Serial.begin(115200);
  stopEngine();
}

void loop() {
  
  // Логика балансировки
  // Если угол больше 1 - движение вперед, если меньше -1 - назад.
  if (gyroAngle > 1.0) {
    forward();
    balansir++;
    
    // ШИМ регуляция
    if ((b > 2) && (gyroAngle < 5.0)) {
      b = 0;
      stopEngine();
    } else if ((b > 1) && (gyroAngle < 2.0)) {
      b = 0;
      stopEngine();
    }
    f = 0; 
    b++;
    
  } else if (gyroAngle < -1.0) {
    backward();
    balansir--;
    
    // ШИМ регуляция
    if ((f > 2) && (gyroAngle > -5.0)) 
    {
      f = 0;
      stopEngine();
    } else if ((f > 1) && (gyroAngle > -2.0)) {
      f = 0;
      stopEngine();
    }
    b = 0; f++;
    
  }
  else {
    stopEngine();
  }
  
  // Запрос данных с MPU-6050
  getData();
  
  // Расчет угла по показаниям акселерометра
  // с учетом корректировки точки равновесия balancing_zerro.
  accelAngle = (atan2(AcX, AcZ)) * RAD_TO_DEG + zeroDeviation;
  
  // Измерение наклона по Х.
  // Использование Комплементарного фильтра,
  alfa = 0.001;
  timer2 = timer;
  timer = micros();
  gyroAngle = (1 - alfa) * (gyroAngle - ((double)GyY * (double)(timer - timer2)) / 131000000.0) + alfa * accelAngle;
  
  // Автокоррекция нуля баланса
  if (balansir < -1) {
    gyroAngle -= 0.08; // Быстро изменяем угол наклона
    zeroDeviation -= 0.006; // Коррекция нуля баланса с течением времени
    balansir = 0;
  }
  if (balansir > 1) {
    gyroAngle += 0.08;
    zeroDeviation += 0.006;
    balansir = 0; 
  }
  
  // Логирование в консоль
  i++;
  if(i == 500) {
    Serial.print("Угол наклона = "); Serial.println(gyroAngle); 
    Serial.print("Нуль баланса = "); Serial.println(zeroDeviation); 
    i=0;
  }
}
