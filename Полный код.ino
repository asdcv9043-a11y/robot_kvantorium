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



int driveLine(int Time, int sped){ // Езда по линии с определенной скоростью, определенное время
  unsigned long currentMillis = millis();
  while(!(analogRead(sensC) > Black) && uDigitalRead(sensR) && uDigitalRead(sensL) && (millis() - currentMillis < Time)){
    drive(sped, sped);
  }
  while((analogRead(sensC) > Black) && !uDigitalRead(sensR) && uDigitalRead(sensL)){
    drive(0, sped / 2);
  }
  while((analogRead(sensC) > Black) && uDigitalRead(sensR) && !uDigitalRead(sensL)){
    drive(sped / 2, 0);
  }
  
  drive(0 , 0);
  
  if (!uDigitalRead(sensR) && !(analogRead(sensC) > Black) && uDigitalRead(sensL)){
    return 1; // Есть возможность повернуть направо
  }
  else if (uDigitalRead(sensR) && !(analogRead(sensC) > Black) && !uDigitalRead(sensL)){
    return 2; // Есть возможность повернуть налево
  }
  else{
    return 0; // Никуда не повернуть
  }
}



int checkDist(int Direction){  // Смотри дистанцию, поворачивая серву: 0 - Прямо, 1 - Право, 2 - Лево
  if (Direction == 2){
    servo.write(0);
    delay(1000);
    return getDist();
  }
  else if (Direction == 1){
    servo.write(180);
    delay(1000);
    return getDist();
  }
  else{
    servo.write(90);
    delay(1000);
    return getDist();
  }
}


void tyrn(int Direction){  //Повернуть: 1 - Право, 2 - Лево, 0 - пропуск
  if (Direction == 1){
    drive(50, 50);
    delay(500);
    drive(127, 0);
    delay(500);
    drive(0, 0);
  }
  else if (Direction == 2){
    drive(50, 50);
    delay(500);
    drive(0, 127);
    delay(500);
    drive(0, 0);
  }
}



void skittle(){ // Сбить все кегли
  unsigned long currentMillis = millis();
  int Timer = 0;
  while (millis() - currentMillis < TimeTyrn - Timer){
    
    while (getDist() > 200){
      drive(50, -50);
    }
    Timer = millis() - currentMillis;
    while (analogRead(sensC) > Black){
      drive(70, 70);
    }
    drive(-70, -70);
    delay(500);
    while (analogRead(sensC) > Green){
      drive(-70, -70);
    }

    currentMillis = millis()
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
