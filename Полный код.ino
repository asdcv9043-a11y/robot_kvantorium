#include <Servo.h>
#include <Wire.h>
#include <Strela.h>

#define trig P4
#define echo P3
#define serv 13
#define sensL P5
#define sensR P6
#define sensC P7
#define Green 70 // Записать сюда значение датчика на зеленом поле
#define Black 50 // Записать сюда значение датчика не черном поле
#define TimeTyrn 5000 // Записать сюда время 1 полного разворота на месте
#define TURN_TIME 450 // Записать сюда время 1 оворота на 90 градусов

Servo servo;



int getDist(){  // Замер дистанции
  digitalWrite(trig, 0);
  delayMicroseconds(5);
  digitalWrite(trig, 1);
  delayMicroseconds(10);
  digitalWrite(trig, 0);
  
  int dist;
  dist = pulseIn(echo, HIGH) / 58.2;
  return dist;
}



int driveLine(int Time, int speed){ // Езда по линии с определенной скоростью, определенное время
  unsigned long t0 = millis();
  
  while (millis() - t0 < Time){
    int C = analogRead(sensC);
    bool L = uDigitalRead(sensL);
    bool R = uDigitalRead(sensR);

    if (C < Black){
      drive(speed, speed);
    }
    else if (!R && L){
      drive(speed / 2, 0);
    }
    else if (R && !L){
      drive(0, speed / 2);
    }
    else{
      break; // перекрёсток или тупик
    }
  }

  drive(0, 0);

  if (!uDigitalRead(sensR) && uDigitalRead(sensL)) return 1;
  if (uDigitalRead(sensR) && !uDigitalRead(sensL)) return 2;
  return 0;
}



int checkDist(int Direction){  // Смотрит дистанцию, поворачивая серву: 0 - Прямо, 1 - Право, 2 - Лево
  if (Direction == 2){
    servo.write(0);
  }
  else if (Direction == 1){
    servo.write(180);
  }
  else{
    servo.write(90);
  }
  delay(1000);
  return getDist();
}



void tyrn(int Direction){ //Повернуть: 1 - Право, 2 - Лево, 0 - пропуск
  drive(60, 60);
  delay(200);

  if (Direction == 1){        // вправо
    drive(100, -100);
  }
  else if (Direction == 2){   // влево
    drive(-100, 100);
  }
  else {
    return;
  }
  delay(TURN_TIME);
  drive(0, 0);
}



void skittle(){ // Сбить все кегли
  unsigned long currentMillis = millis();
  int Timer = 0;
  while (millis() - currentMillis < TimeTyrn - Timer){
    
    while ((getDist() > 200) && (millis() - currentMillis < TimeTyrn - Timer)){
      drive(50, -50);
    }
    Timer = millis() - currentMillis;
    while (analogRead(sensC) > Black){
      drive(70, 70);
    }
    drive(-70, -70);
    delay(500);
    while (analogRead(sensC) > Black){
      drive(-70, -70);
    }

    currentMillis = millis();
  }
}



void goToSkittle(){ // Доехать от точки старта до кеглей (Остановиться на зеленом поле)
  while(analogRead(sensC) <= (Green + 10)){
    drive(100, 100);
  }
  delay(200);
  while(analogRead(sensC) > Green){
  }
  drive(0, 0);
}



void labyrinth(){ // Пройти лабиринт
  while (getDist() < 200){
    while(getDist() > 15){
      drive(100, 100);
    }
    drive(0, 0);
    tyrn(dawn());
  }
}



int dawn(){ // пусто 1 - справа, 2 - слева
  int L = checkDist(2);
  int R = checkDist(1);
  if ((L > R) && (L > checkDist(0))){
    return 2;
  }
  else{
    return 1;
  }
}



void setup() {
  uPinMode(trig, OUTPUT); // Инициализация пинов, двигателей, сервы
  uPinMode(serv, OUTPUT);
  uPinMode(echo, INPUT);
  uPinMode(sensL, INPUT);
  uPinMode(sensR, INPUT);
  uPinMode(sensC, INPUT);
  motorConnection(1, 1);
  servo.attach(serv);
  
  servo.write(0); // Покрутить сервой в начале (для отладки (Должна начать смотреть прямо))
  delay(1000);
  servo.write(180);
  delay(1000);
  servo.write(90);
  delay(1000);
}



void loop() {

}
