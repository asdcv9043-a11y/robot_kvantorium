#include <Servo.h>
#include <Wire.h>
#include <Strela.h>

// ==================== НАСТРОЙКИ ====================

#define trig P4
#define echo P3
#define serv 11
#define sensL P7
#define sensR P5
#define sensC P6

// Пороговые значения (НАСТРОИТЬ!)
#define GREEN_MIN 50
#define GREEN_MAX 100
#define BLACK_THRESHOLD 400

// Время поворотов (НАСТРОИТЬ!)
#define TURN_90_TIME 400
#define TURN_180_TIME 800
#define OBSTACLE_DIST 20

// Скорости
#define SPEED_NORMAL 127
#define SPEED_SLOW 80
#define SPEED_TURN 100
#define SPEED_SEARCH 60

// Позиции сервы
#define SERVO_LEFT 0
#define SERVO_CENTER 90
#define SERVO_RIGHT 180

Servo servo;

// ==================== СОСТОЯНИЯ ====================

// Какой полигон выполняем (1, 2 или 3)
int currentPolygon = 1;

// --- Полигон 1: Линия ---
int p1_state = 0;        // Состояние внутри полигона
int p1_bypassStep = 0;   // Шаг объезда препятствия
unsigned long p1_timer = 0;

// --- Полигон 2: Лабиринт ---
int p2_state = 0;
unsigned long p2_timer = 0;

// --- Полигон 3: Кегельринг ---
int p3_state = 0;
int p3_attempts = 0;
int p3_scanDir = 0;      // Направление сканирования
unsigned long p3_timer = 0;

// ==================== БАЗОВЫЕ ФУНКЦИИ ====================

int getDist() {
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  int dist = pulseIn(echo, HIGH, 25000) / 58.2;
  return dist;
}

int checkDist(int direction) {
  if (direction == 0) {
    servo.write(SERVO_LEFT);
  } else if (direction == 2) {
    servo.write(SERVO_RIGHT);
  } else {
    servo.write(SERVO_CENTER);
  }
  delay(400);
  int dist = getDist();
  return dist;
}

void stopRobot() {
  drive(0, 0);
}

int isOnGreen() {
  int val = analogRead(sensC);
  if (val >= GREEN_MIN && val <= GREEN_MAX) {
    return 1;
  }
  return 0;
}

int isOnBlack() {
  if (analogRead(sensC) > BLACK_THRESHOLD) {
    return 1;
  }
  return 0;
}

// ==================== ПОЛИГОН 1: ЛИНИЯ ====================

void polygon1_line() {
  
  // === СОСТОЯНИЕ 0: Едем по линии ===
  if (p1_state == 0) {
    
    // Проверка финиша
    if (isOnGreen() == 1) {
      stopRobot();
      p1_state = 100;  // Полигон завершён
      return;
    }
    
    // Проверка препятствия
    int dist = getDist();
    if (dist > 0 && dist < OBSTACLE_DIST) {
      stopRobot();
      p1_state = 1;    // Начинаем объезд
      p1_bypassStep = 1;
      p1_timer = millis();
      return;
    }
    
    // Следование по линии
    int L = uDigitalRead(sensL);
    int R = uDigitalRead(sensR);
    int C = isOnBlack();
    
    if (L == 0 && R == 0) {
      drive(SPEED_NORMAL, SPEED_NORMAL);
    } else if (L == 0 && R == 1) {
      drive(SPEED_SLOW, SPEED_NORMAL);
    } else if (L == 1 && R == 0) {
      drive(SPEED_NORMAL, SPEED_SLOW);
    } else if (C == 1) {
      drive(SPEED_NORMAL, SPEED_NORMAL);
    } else {
      drive(-SPEED_TURN, SPEED_TURN);
    }
    return;
  }
  
  // === СОСТОЯНИЕ 1: Объезд препятствия ===
  if (p1_state == 1) {
    
    // Шаг 1: Отъезд назад
    if (p1_bypassStep == 1) {
      drive(-SPEED_SLOW, -SPEED_SLOW);
      if (millis() - p1_timer > 300) {
        stopRobot();
        p1_bypassStep = 2;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 2: Поворот направо
    if (p1_bypassStep == 2) {
      drive(SPEED_TURN, -SPEED_TURN);
      if (millis() - p1_timer > TURN_90_TIME) {
        stopRobot();
        p1_bypassStep = 3;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 3: Едем вправо
    if (p1_bypassStep == 3) {
      drive(SPEED_SLOW, SPEED_SLOW);
      if (millis() - p1_timer > 600) {
        stopRobot();
        p1_bypassStep = 4;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 4: Поворот налево
    if (p1_bypassStep == 4) {
      drive(-SPEED_TURN, SPEED_TURN);
      if (millis() - p1_timer > TURN_90_TIME) {
        stopRobot();
        p1_bypassStep = 5;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 5: Едем вперёд
    if (p1_bypassStep == 5) {
      drive(SPEED_SLOW, SPEED_SLOW);
      if (millis() - p1_timer > 800) {
        stopRobot();
        p1_bypassStep = 6;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 6: Поворот налево (ищем линию)
    if (p1_bypassStep == 6) {
      drive(-SPEED_TURN, SPEED_TURN);
      if (millis() - p1_timer > TURN_90_TIME) {
        stopRobot();
        p1_bypassStep = 7;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 7: Ищем линию
    if (p1_bypassStep == 7) {
      if (isOnBlack() == 1) {
        stopRobot();
        p1_bypassStep = 8;
        p1_timer = millis();
        return;
      }
      drive(SPEED_SLOW, SPEED_SLOW);
      if (millis() - p1_timer > 2000) {
        stopRobot();
        p1_bypassStep = 8;
        p1_timer = millis();
      }
      return;
    }
    
    // Шаг 8: Выравнивание
    if (p1_bypassStep == 8) {
      drive(SPEED_TURN, -SPEED_TURN);
      if (millis() - p1_timer > TURN_90_TIME) {
        stopRobot();
        p1_state = 0;  // Возвращаемся к езде по линии
        servo.write(SERVO_CENTER);
      }
      return;
    }
  }
}

// ==================== ПОЛИГОН 2: ЛАБИРИНТ ====================

void polygon2_maze() {
  
  // === СОСТОЯНИЕ 0: Анализ и движение ===
  if (p2_state == 0) {
    
    // Проверка финиша
    if (isOnGreen() == 1) {
      stopRobot();
      p2_state = 100;  // Полигон завершён
      return;
    }
    
    // Измеряем дистанции
    int distCenter = getDist();
    
    servo.write(SERVO_RIGHT);
    delay(400);
    int distRight = getDist();
    
    servo.write(SERVO_LEFT);
    delay(400);
    int distLeft = getDist();
    
    servo.write(SERVO_CENTER);
    
    // Алгоритм правой руки
    if (distRight > 25) {
      // Справа свободно
      p2_state = 1;  // Поворот направо
      p2_timer = millis();
    } else if (distCenter > 25) {
      // Прямо свободно
      p2_state = 2;  // Едем прямо
      p2_timer = millis();
    } else if (distLeft > 25) {
      // Слева свободно
      p2_state = 3;  // Поворот налево
      p2_timer = millis();
    } else {
      // Тупик
      p2_state = 4;  // Разворот
      p2_timer = millis();
    }
    return;
  }
  
  // === СОСТОЯНИЕ 1: Поворот направо ===
  if (p2_state == 1) {
    drive(SPEED_TURN, -SPEED_TURN);
    if (millis() - p2_timer > TURN_90_TIME) {
      stopRobot();
      p2_state = 0;
    }
    return;
  }
  
  // === СОСТОЯНИЕ 2: Едем прямо ===
  if (p2_state == 2) {
    drive(SPEED_NORMAL, SPEED_NORMAL);
    if (millis() - p2_timer > 100) {
      stopRobot();
      p2_state = 0;
    }
    return;
  }
  
  // === СОСТОЯНИЕ 3: Поворот налево ===
  if (p2_state == 3) {
    drive(-SPEED_TURN, SPEED_TURN);
    if (millis() - p2_timer > TURN_90_TIME) {
      stopRobot();
      p2_state = 0;
    }
    return;
  }
  
  // === СОСТОЯНИЕ 4: Разворот ===
  if (p2_state == 4) {
    drive(SPEED_TURN, -SPEED_TURN);
    if (millis() - p2_timer > TURN_180_TIME) {
      stopRobot();
      p2_state = 0;
    }
    return;
  }
}

// ==================== ПОЛИГОН 3: КЕГЕЛЬРИНГ ====================

void polygon3_skittles() {
  
  // === СОСТОЯНИЕ 0: Сканирование центра ===
  if (p3_state == 0) {
    servo.write(SERVO_CENTER);
    delay(300);
    int dist = getDist();
    
    if (dist > 0 && dist < 80) {
      // Кегля прямо - атака
      p3_state = 10;
      p3_timer = millis();
    } else {
      // Смотрим вправо
      p3_state = 1;
    }
    return;
  }
  
  // === СОСТОЯНИЕ 1: Сканирование вправо ===
  if (p3_state == 1) {
    servo.write(SERVO_RIGHT);
    delay(400);
    int dist = getDist();
    
    if (dist > 0 && dist < 80) {
      // Кегля справа - поворот
      p3_state = 5;
      p3_timer = millis();
    } else {
      // Смотрим влево
      p3_state = 2;
    }
    return;
  }
  
  // === СОСТОЯНИЕ 2: Сканирование влево ===
  if (p3_state == 2) {
    servo.write(SERVO_LEFT);
    delay(400);
    int dist = getDist();
    
    servo.write(SERVO_CENTER);
    
    if (dist > 0 && dist < 80) {
      // Кегля слева - поворот
      p3_state = 6;
      p3_timer = millis();
    } else {
      // Кеглей нет - вращаемся
      p3_state = 7;
      p3_timer = millis();
    }
    return;
  }
  
  // === СОСТОЯНИЕ 5: Поворот вправо к кегле ===
  if (p3_state == 5) {
    drive(SPEED_TURN, -SPEED_TURN);
    if (millis() - p3_timer > 200) {
      stopRobot();
      servo.write(SERVO_CENTER);
      p3_state = 10;  // Атака
      p3_timer = millis();
    }
    return;
  }
  
  // === СОСТОЯНИЕ 6: Поворот влево к кегле ===
  if (p3_state == 6) {
    drive(-SPEED_TURN, SPEED_TURN);
    if (millis() - p3_timer > 200) {
      stopRobot();
      servo.write(SERVO_CENTER);
      p3_state = 10;  // Атака
      p3_timer = millis();
    }
    return;
  }
  
  // === СОСТОЯНИЕ 7: Вращение для поиска ===
  if (p3_state == 7) {
    drive(SPEED_SEARCH, -SPEED_SEARCH);
    if (millis() - p3_timer > 200) {
      stopRobot();
      p3_attempts = p3_attempts + 1;
      
      // Проверка: слишком много попыток?
      if (p3_attempts > 30) {
        p3_state = 100;  // Завершаем
      } else {
        p3_state = 0;    // Ищем снова
      }
    }
    return;
  }
  
  // === СОСТОЯНИЕ 10: Атака - разгон ===
  if (p3_state == 10) {
    drive(SPEED_NORMAL + 30, SPEED_NORMAL + 30);
    if (millis() - p3_timer > 800) {
      p3_state = 11;
      p3_timer = millis();
    }
    return;
  }
  
  // === СОСТОЯНИЕ 11: Атака - толчок ===
  if (p3_state == 11) {
    drive(200, 200);
    if (millis() - p3_timer > 300) {
      stopRobot();
      p3_state = 12;
      p3_timer = millis();
    }
    return;
  }
  
  // === СОСТОЯНИЕ 12: Возврат назад до линии ===
  if (p3_state == 12) {
    if (isOnBlack() == 1) {
      stopRobot();
      p3_state = 13;
      p3_timer = millis();
      return;
    }
    drive(-SPEED_SLOW, -SPEED_SLOW);
    return;
  }
  
  // === СОСТОЯНИЕ 13: Отъезд от линии ===
  if (p3_state == 13) {
    drive(-SPEED_SLOW, -SPEED_SLOW);
    if (millis() - p3_timer > 300) {
      stopRobot();
      p3_attempts = p3_attempts + 1;
      
      // Проверка: все кегли вытеснены?
      p3_state = 20;  // Финальная проверка
    }
    return;
  }
  
  // === СОСТОЯНИЕ 20: Проверка завершения ===
  if (p3_state == 20) {
    servo.write(SERVO_CENTER);
    delay(300);
    int d1 = getDist();
    
    servo.write(SERVO_RIGHT);
    delay(400);
    int d2 = getDist();
    
    servo.write(SERVO_LEFT);
    delay(400);
    int d3 = getDist();
    
    servo.write(SERVO_CENTER);
    
    // Если везде далеко - кеглей нет
    int clear1 = 0;
    int clear2 = 0;
    int clear3 = 0;
    
    if (d1 > 80 || d1 == 0) clear1 = 1;
    if (d2 > 80 || d2 == 0) clear2 = 1;
    if (d3 > 80 || d3 == 0) clear3 = 1;
    
    if (clear1 == 1 && clear2 == 1 && clear3 == 1) {
      p3_state = 100;  // Всё чисто - финиш
    } else {
      p3_state = 0;    // Ищем ещё кегли
    }
    return;
  }
}

// ==================== ПЕРЕХОД МЕЖДУ ПОЛИГОНАМИ ====================

void moveToNextPolygon() {
  // Просто едем вперёд пока не увидим зелёное
  if (isOnGreen() == 0) {
    drive(SPEED_SLOW, SPEED_SLOW);
  } else {
    stopRobot();
    currentPolygon = currentPolygon + 1;
  }
}

// ==================== SETUP ====================

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
  
  delay(1000);
}

// ==================== LOOP ====================

void loop() {
  
  // Полигон 1: Линия
  if (currentPolygon == 1) {
    polygon1_line();
    
    if (p1_state == 100) {
      // Полигон 1 завершён
      delay(500);
      currentPolygon = 10;  // Переход
    }
    return;
  }
  
  // Переход к полигону 2
  if (currentPolygon == 10) {
    moveToNextPolygon();
    return;
  }
  
  // Полигон 2: Лабиринт
  if (currentPolygon == 2) {
    polygon2_maze();
    
    if (p2_state == 100) {
      delay(500);
      currentPolygon = 20;  // Переход
    }
    return;
  }
  
  // Переход к полигону 3
  if (currentPolygon == 20) {
    moveToNextPolygon();
    return;
  }
  
  // Полигон 3: Кегельринг
  if (currentPolygon == 3) {
    polygon3_skittles();
    
    if (p3_state == 100) {
      currentPolygon = 100;  // Всё завершено
    }
    return;
  }
  
  // Всё пройдено - остановка
  if (currentPolygon == 100) {
    stopRobot();
    delay(1000);
  }
}
