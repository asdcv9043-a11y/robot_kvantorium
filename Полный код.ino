#include <Servo.h>
#include <Wire.h>
#include <Strela.h>

// Пины
#define trig P4
#define echo P3
#define serv 11
#define sensL P7
#define sensR P5
#define sensC P6

// Пороговые значения датчика
#define GREEN_MIN 50        // Минимальное значение на зелёном поле
#define GREEN_MAX 100       // Максимальное значение на зелёном поле
#define BLACK_THRESHOLD 400 // Порог чёрного (выше = чёрный)

// Временные константы
#define TURN_90_TIME 400    // Время поворота на 90 градусов
#define TURN_180_TIME 800   // Время разворота на 180 градусов
#define OBSTACLE_DIST 20    // Дистанция до препятствия

// Скорости (от -255 до 255)
#define SPEED_NORMAL 127
#define SPEED_SLOW 80
#define SPEED_TURN 100
#define SPEED_SEARCH 60

// Позиции сервы
#define SERVO_LEFT 0
#define SERVO_CENTER 90
#define SERVO_RIGHT 180

Servo servo;

// Переменные для отслеживания состояния
int polygon1Complete = 0;  // 0 = не пройден, 1 = пройден
int polygon2Complete = 0;
int polygon3Complete = 0;


// Измерение дистанции
int getDist() {
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  int dist = pulseIn(echo, HIGH, 25000) / 58.2;
  return dist;
}

// Измерение дистанции в указанном направлении 0 = лево, 1 = прямо, 2 = право
int checkDist(int direction) {
  if (direction == 0) {
    servo.write(SERVO_LEFT);
  } else if (direction == 2) {
    servo.write(SERVO_RIGHT);
  } else {
    servo.write(SERVO_CENTER);
  }
  delay(500);
  int dist = getDist();
  delay(100);
  return dist;
}

// Остановка робота
void stopRobot() {
  drive(0, 0);
}

// Движение вперёд с остановкой
void moveForward(int speedVal, int timeMs) {
  drive(speedVal, speedVal);
  delay(timeMs);
  stopRobot();
}

// Движение назад с остановкой
void moveBackward(int speedVal, int timeMs) {
  drive(-speedVal, -speedVal);
  delay(timeMs);
  stopRobot();
}

// Поворот направо на 90 градусов
void turnRight() {
  drive(SPEED_TURN, -SPEED_TURN);
  delay(TURN_90_TIME);
  stopRobot();
}

// Поворот налево на 90 градусов
void turnLeft() {
  drive(-SPEED_TURN, SPEED_TURN);
  delay(TURN_90_TIME);
  stopRobot();
}

// Разворот на 180 градусов
void turnAround() {
  drive(SPEED_TURN, -SPEED_TURN);
  delay(TURN_180_TIME);
  stopRobot();
}

// Проверка: на зелёный
int isOnGreen() {
  int val = analogRead(sensC);
  if (val >= GREEN_MIN && val <= GREEN_MAX) {
    return 1;  // зелёный
  }
  return 0;  // Нет
}

// Проверка на чёрный
int isOnBlack() {
  if (analogRead(sensC) > BLACK_THRESHOLD) {
    return 1;  // Да, на чёрном
  }
  return 0;  // Нет
}

// Линия

// Объезд препятствия справа
void bypassObstacle() {
  stopRobot();
  delay(200);
  
  // Отъезжаем назад
  moveBackward(SPEED_SLOW, 300);
  
  // Поворачиваем направо
  turnRight();
  moveForward(SPEED_SLOW, 600);
  
  // Поворачиваем налево
  turnLeft();
  moveForward(SPEED_SLOW, 800);
  
  // Поворачиваем налево снова (ищем линию)
  turnLeft();
  
  // Ищем линию
  int lineFound = 0;
  unsigned long startTime = millis();
  while (millis() - startTime < 2000) {
    if (isOnBlack() == 1) {
      lineFound = 1;
      break;
    }
    moveForward(SPEED_SLOW, 50);
  }
  
  // Выравниваемся на линии
  turnRight();
  stopRobot();
}

// Следование по линии с обходом препятствия
void followLineWithObstacle() {
  servo.write(SERVO_CENTER);
  
  int finished = 0;
  
  while (finished == 0) {
    // Проверяем финиш (зелёное поле)
    if (isOnGreen() == 1) {
      stopRobot();
      finished = 1;
      break;
    }
    
    // Проверяем препятствие
    int dist = getDist();
    if (dist > 0 && dist < OBSTACLE_DIST) {
      bypassObstacle();
      continue;
    }
    
    // Читать датчики линии
    int L = uDigitalRead(sensL);
    int R = uDigitalRead(sensR);
    int C = isOnBlack();
    
    // Логика следования по линии
    if (L == 0 && R == 0) {
      // прямо
      drive(SPEED_NORMAL, SPEED_NORMAL);
    } else if (L == 0 && R == 1) {
      // поворачиваем на лево
      drive(SPEED_SLOW, SPEED_NORMAL);
    } else if (L == 1 && R == 0) {
      // поворачиваем на право
      drive(SPEED_NORMAL, SPEED_SLOW);
    } else if (C == 1) {
      // прямо
      drive(SPEED_NORMAL, SPEED_NORMAL);
    } else {
      // Поиск
      drive(-SPEED_TURN, SPEED_TURN);  // Налево
      delay(100);
      
      if (isOnBlack() == 0) {
        drive(SPEED_TURN, -SPEED_TURN);  // На право
        delay(200);
      }
    }
    
    delay(10);
  }
}

// Лабиринт

void mazeRightHand() {
  servo.write(SERVO_CENTER);
  
  int finished = 0;
  
  while (finished == 0) {
    // Проверка на финиш
    if (isOnGreen() == 1) {
      stopRobot();
      finished = 1;
      break;
    }
    
    // Измеряем дистанцию в трёх направлениях
    int distCenter = getDist();
    int distRight = checkDist(2);  // Право
    int distLeft = checkDist(0);   // Лево
    
    // Возвращаем серву в центр
    servo.write(SERVO_CENTER);
    delay(200);
    
    if (distRight > 25) {
      // Справа свободно - направо
      moveForward(SPEED_SLOW, 200);
      turnRight();
    } else if (distCenter > 25) {
      // Прямо свободно
      moveForward(SPEED_NORMAL, 100);
    } else if (distLeft > 25) {
      // Слева свободно
      moveForward(SPEED_SLOW, 200);
      turnLeft();
    } else {
      // Тупик - разворот
      turnAround();
    }
    
    delay(50);
  }
}

// Кегли

// Поиск кегли 0 = не найдено, 1 = справа, 2 = слева, 3 = прямо
int searchSkittle() {
  servo.write(SERVO_CENTER);
  delay(300);
  int distCenter = getDist();
  
  if (distCenter > 0 && distCenter < 80) {
    return 3;  // Прямо
  }
  
  servo.write(SERVO_RIGHT);
  delay(400);
  int distRight = getDist();
  
  servo.write(SERVO_LEFT);
  delay(400);
  int distLeft = getDist();
  
  servo.write(SERVO_CENTER);
  delay(200);
  
  if (distRight > 0 && distRight < 80) {
    return 1;  // Справа
  }
  if (distLeft > 0 && distLeft < 80) {
    return 2;  // Слева
  }
  
  return 0;  // Не найдено
}

// Поворот к кегле
void turnToSkittle(int direction) {
  if (direction == 1) {
    // Справа - поворот вправо
    drive(SPEED_TURN, -SPEED_TURN);
    delay(200);
  } else if (direction == 2) {
    // Слева - поворот влево
    drive(-SPEED_TURN, SPEED_TURN);
    delay(200);
  }
  stopRobot();
}

// Вытеснение кегли
void pushSkittle() {
  drive(SPEED_NORMAL + 30, SPEED_NORMAL + 30);
  delay(800);
  drive(200, 200);
  delay(300);
  stopRobot();
  delay(100);
  
  // назад до линии ринга
  int onBlack = 0;
  while (onBlack == 0) {
    drive(-SPEED_SLOW, -SPEED_SLOW);
    delay(10);
    onBlack = isOnBlack();
  }
  
  // Ещё немного назад, чтобы быть внутри ринга
  drive(-SPEED_SLOW, -SPEED_SLOW);
  delay(300);
  stopRobot();
}

// Основная функция кельгеринга
void skittlesRing() {
  int attempts = 0;       // Счётчик попыток
  int maxAttempts = 30;   // Максимальное количество попыток
  int allCleared = 0;     // Флаг: все кегли вытеснены
  
  servo.write(SERVO_CENTER);
  
  while (attempts < maxAttempts && allCleared == 0) {
    // Ищем кеглю
    int found = searchSkittle();
    
    if (found > 0) {
      // Кегля найдена
      if (found != 3) {
        // Поворот
        turnToSkittle(found);
      }
      
      // Вытолкнуть кеглю
      pushSkittle();
      
      // Увеличиваем счётчик попыток
      attempts = attempts + 1;
      
      // Небольшой разворот для поиска следующей
      drive(SPEED_TURN, -SPEED_TURN);
      delay(300);
      stopRobot();
      
    } else {
      // Кегля не найдена
      drive(SPEED_SEARCH, -SPEED_SEARCH);
      delay(200);
      stopRobot();
      
      // Увеличиваем счётчик попыток
      attempts = attempts + 1;
    }
    
    // Проверяем - все ли кегли вытеснены
    servo.write(SERVO_CENTER);
    delay(300);
    int finalCheck = getDist();
    
    servo.write(SERVO_RIGHT);
    delay(400);
    int checkRight = getDist();
    
    servo.write(SERVO_LEFT);
    delay(400);
    int checkLeft = getDist();
    
    servo.write(SERVO_CENTER);
    
    // Если везде дистанция > 80 см - кегли закончились
    int centerClear = 0;
    int rightClear = 0;
    int leftClear = 0;
    
    if (finalCheck > 80 || finalCheck == 0) {
      centerClear = 1;
    }
    if (checkRight > 80 || checkRight == 0) {
      rightClear = 1;
    }
    if (checkLeft > 80 || checkLeft == 0) {
      leftClear = 1;
    }
    
    if (centerClear == 1 && rightClear == 1 && leftClear == 1) {
      allCleared = 1;
    }
  }
  
  // Остановка в центре ринга
  stopRobot();
}

// Езда между полигонами

void moveToNextPolygon() {
  // Едем вперёд пока не найдём следующий старт
  int greenFound = 0;
  while (greenFound == 0) {
    drive(SPEED_SLOW, SPEED_SLOW);
    delay(50);
    greenFound = isOnGreen();
  }
  stopRobot();
}


void setup() {
  uPinMode(trig, OUTPUT);
  uPinMode(serv, OUTPUT);
  uPinMode(echo, INPUT);
  uPinMode(sensL, INPUT);
  uPinMode(sensR, INPUT);
  uPinMode(sensC, INPUT);
  
  motorConnection(1, 0);
  
  servo.attach(serv);
  
  // Тест сервы
  servo.write(SERVO_LEFT);
  delay(500);
  servo.write(SERVO_CENTER);
  delay(500);
  servo.write(SERVO_RIGHT);
  delay(500);
  servo.write(SERVO_CENTER);
  delay(500);
  
  // Пауза перед стартом
  delay(1000);
}


void loop() {
  
  // Полигон 1: Следование по линии с препятствием
  if (polygon1Complete == 0) {
    followLineWithObstacle();
    polygon1Complete = 1;
    delay(500);
    
    moveToNextPolygon();
  }
  
  // Полигон 2: Лабиринт 
  if (polygon1Complete == 1 && polygon2Complete == 0) {
    mazeRightHand();
    polygon2Complete = 1;
    delay(500);
    
    moveToNextPolygon();
  }
  
  // Полигон 3: Кегельринг
  if (polygon2Complete == 1 && polygon3Complete == 0) {
    skittlesRing();
    polygon3Complete = 1;
  }
  
  // Все полигоны пройдены - остановка
  if (polygon3Complete == 1) {
    stopRobot();
    while (true) {
      delay(1000);
    }
  }
}
