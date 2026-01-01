#include <Servo.h> //サーボのためのライブラリ

Servo penguin; //penguinという名前をつけたの
void setup() {
  penguin.attach(5); //PWMにつなぐ。[~]マークの3,5,6,9,10,11。
}

void loop() {
  penguin.write(0); //0度の位置まで回転
  delay(400); //回転のため0.4秒待つ
  penguin.write(120); //120度の位置まで回転...以下略
  delay(300);
  penguin.write(60);
  delay(200);
  penguin.write(180);
  delay(300);
}
