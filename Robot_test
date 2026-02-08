#include <Strela.h>

#define sensL P5
#define sensR P6
#define sensC P7




void waitForConfirmation() {
  while (true) {
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == '+') {
        Serial.println("Подтверждено");
        delay(500);
        break;
      }
    }
  }
}




void setup() {
  Serial.begin(9600);
  uPinMode(sensL, INPUT);
  uPinMode(sensR, INPUT);
  uPinMode(sensC, INPUT);
  motorConnection(1, 1);
  
  delay(2000);
  Serial.println("=== КАЛИБРОВКА ДАТЧИКОВ ===");
  
  // ===== ЧЕРНАЯ ПОВЕРХНОСТЬ =====
  Serial.println("\n1. Поставьте робота на ЧЕРНУЮ поверхность");
  Serial.println("Введите '+' для продолжения:");
  waitForConfirmation();
  
  delay(500);
  bool L_black = uDigitalRead(sensL);
  bool R_black = uDigitalRead(sensR);
  int C_black = analogRead(sensC);
  
  // Проверка правого датчика
  if (R_black == 1) {
    Serial.println("ОШИБКА: Правый датчик показывает 1 на черном!");
    Serial.println("Пожалуйста, откалибруйте правый датчик.");
    while(1); // Остановка программы
  }
  
  // Проверка левого датчика
  if (L_black == 1) {
    Serial.println("ОШИБКА: Левый датчик показывает 1 на черном!");
    Serial.println("Пожалуйста, откалибруйте левый датчик.");
    while(1); // Остановка программы
  }
  
  Serial.print("Центральный датчик на черном: ");
  Serial.println(C_black);
  
  // ===== БЕЛАЯ ПОВЕРХНОСТЬ =====
  Serial.println("\n2. Поставьте робота на белую поверхность");
  Serial.println("Введите '+' для продолжения:");
  waitForConfirmation();
  
  delay(500);
  bool L_white = uDigitalRead(sensL);
  bool R_white = uDigitalRead(sensR);
  int C_white = analogRead(sensC);
  
  // Проверка правого датчика
  if (R_white == 0) {
    Serial.println("ОШИБКА: Правый датчик показывает 0 на белом!");
    Serial.println("Пожалуйста, откалибруйте правый датчик.");
    while(1); // Остановка программы
  }
  
  // Проверка левого датчика
  if (L_white == 0) {
    Serial.println("ОШИБКА: Левый датчик показывает 0 на белом!");
    Serial.println("Пожалуйста, откалибруйте левый датчик.");
    while(1); // Остановка программы
  }
  
  Serial.print("Центральный датчик на белом: ");
  Serial.println(C_white);
  
  // ===== ЗЕЛЕНАЯ ПОВЕРХНОСТЬ =====
  Serial.println("\n3. Поставьте робота на зеленую поверхность");
  Serial.println("Введите '+' для продолжения:");
  waitForConfirmation();
  
  delay(500);
  int C_green = analogRead(sensC);
  Serial.print("Центральный датчик на зеленом: ");
  Serial.println(C_green);
  
  // ===== ТЕСТ ВРАЩЕНИЯ 1 (медленное) =====
  Serial.println("\n=== ТЕСТ ВРАЩЕНИЯ ===");
  Serial.println("Сейчас робот начнет крутиться на 360 градусов (медленно)");
  delay(3000);
  
  Serial.println("Вращение началось...");
  drive(50, -50);
  delay(5000);
  drive(0, 0);
  Serial.println("Первое вращение завершено!");
  
  // ===== ТЕСТ ВРАЩЕНИЯ 2 (быстрое) =====
  Serial.println("\nСейчас робот начнет крутиться на 90 градусов (быстро)");
  delay(3000);
  
  Serial.println("Вращение началось...");
  drive(100, -100);
  delay(3000);
  drive(0, 0);
  Serial.println("Второе вращение завершено!");
  
  // ===== ЗАВЕРШЕНИЕ =====
  Serial.println("\n=== КАЛИБРОВКА ЗАВЕРШЕНА ===");
  Serial.println("Программа завершена.");
}




void loop() {

}
