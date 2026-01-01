#include <Wire.h>
#include <Servo.h>

// デバイスアドレス(スレーブ)
uint8_t DEVICE_ADDRESS = 0x3E;

Servo myServo;

// 文字列定義 (半角カタカナ)
// サギョウチュウ...
char workLabel[] = "\273\267\336\256\263\301\255\263..."; 
// キュウケイチュウ...
char breakLabel[] = "\267\255\263\271\262\301\255\263...";
// イチジテイシチュウ... (ｲﾁｼﾞﾃｲｼﾁｭｳ...)
char pauseLabel[] = "\262\301\274\336\303\262\274\301\255\263...";

// ピン定義
const int PIN_BUTTON_6 = 6;
const int PIN_BUTTON_7 = 7;
const int PIN_SERVO = 5;

// 状態管理
enum TimerMode {
  MODE_IDLE,
  MODE_WORK,
  MODE_BREAK
};

TimerMode currentMode = MODE_IDLE;
bool isPaused = false;
long remainingSeconds = 0;
unsigned long lastTickTime = 0;

// ボタン管理
int lastBtn6State = HIGH;
int lastBtn7State = HIGH;
unsigned long btn7PressStartTime = 0;
bool btn7LongPressHandled = false;
unsigned long btn6PressStartTime = 0;
bool btn6LongPressHandled = false;

void setup() {
  // マスタとしてI2Cバスに接続する
  Wire.begin();

  // LCDの初期設定
  init_LCD();
  
  // ボタン設定 (プルアップ)
  pinMode(PIN_BUTTON_6, INPUT_PULLUP);
  pinMode(PIN_BUTTON_7, INPUT_PULLUP);

  // サーボ設定
  myServo.attach(PIN_SERVO);
  myServo.write(0); // 初期位置は0度
  
  // 初期画面
  enterIdleMode();
  // 起動時は案内を表示したい場合は以下を有効化
  writeCommand(0x0C); // Display ON
  setCursor(0, 0);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // --- ボタン読み取り ---
  int btn6State = digitalRead(PIN_BUTTON_6);
  int btn7State = digitalRead(PIN_BUTTON_7);

  // --- ピン6の処理 (開始 / 一時停止 / 再開 / 長押しで終了) ---
  if (lastBtn6State == HIGH && btn6State == LOW) {
    // 押し始め
    btn6PressStartTime = currentMillis;
    btn6LongPressHandled = false;

    if (currentMode == MODE_IDLE) {
      startWork();
      btn6LongPressHandled = true; // 開始は即座に実行
    }
  }

  // 長押し判定 (押されている間)
  if (btn6State == LOW && currentMode != MODE_IDLE && !btn6LongPressHandled) {
    if (currentMillis - btn6PressStartTime > 2000) {
      // 2秒以上長押し -> IDLEモードへ (終了・非表示)
      enterIdleMode();
      btn6LongPressHandled = true;
    }
  }

  // 離した判定 (短押し)
  if (lastBtn6State == LOW && btn6State == HIGH) {
    if (currentMode != MODE_IDLE && !btn6LongPressHandled) {
      // 短押し -> 一時停止/再開
      isPaused = !isPaused;
      
      // 画面更新
      setCursor(0, 0);
      if (isPaused) {
        printString(pauseLabel);
      } else {
        if (currentMode == MODE_WORK) {
          printString(workLabel);
        } else {
          printString(breakLabel);
        }
        // 残った文字を消すためにスペースを表示
        printString("  "); 
      }
    }
  }
  lastBtn6State = btn6State;

  // --- ピン7の処理 (開始 / リセット / スキップ) ---
  if (lastBtn7State == HIGH && btn7State == LOW) {
    // 押し始め
    btn7PressStartTime = currentMillis;
    btn7LongPressHandled = false;
    
    if (currentMode == MODE_IDLE) {
      startWork();
      btn7LongPressHandled = true; // 開始アクションとして処理済みとする
    }
  }
  
  // 長押し判定 (押されている間)
  if (btn7State == LOW && currentMode != MODE_IDLE && !btn7LongPressHandled) {
    if (currentMillis - btn7PressStartTime > 2000) {
      // 2秒以上長押し -> モード切り替え (スキップ)
      toggleMode();
      btn7LongPressHandled = true; // 処理済みフラグ
    }
  }
  
  // 離した判定 (短押し)
  if (lastBtn7State == LOW && btn7State == HIGH) {
    if (currentMode != MODE_IDLE && !btn7LongPressHandled) {
      // 短押し -> リセット
      resetTimer();
    }
  }
  lastBtn7State = btn7State;

  // --- タイマー処理 ---
  if (currentMode != MODE_IDLE && !isPaused) {
    if (currentMillis - lastTickTime >= 1000) {
      lastTickTime = currentMillis;
      remainingSeconds--;
      
      if (remainingSeconds < 0) {
        toggleMode(); // 時間切れでモード切り替え
      } else {
        updateTimeDisplay();
      }
    }
  }
}

// 作業モード開始
void startWork() {
  writeCommand(0x0C); // Display ON (念のため)
  currentMode = MODE_WORK;
  remainingSeconds = 25 * 60;
  isPaused = false;
  lastTickTime = millis();
  
  myServo.write(180); // サーボを180度へ

  clearLCD();
  setCursor(0, 0);
  printString(workLabel);
  updateTimeDisplay();
}

// 休憩モード開始
void startBreak() {
  writeCommand(0x0C); // Display ON (念のため)
  currentMode = MODE_BREAK;
  remainingSeconds = 5 * 60;
  isPaused = false;
  lastTickTime = millis();
  
  myServo.write(0); // サーボを0度へ

  clearLCD();
  setCursor(0, 0);
  printString(breakLabel);
  updateTimeDisplay();
}

// IDLEモードへ移行 (終了・非表示)
void enterIdleMode() {
  currentMode = MODE_IDLE;
  isPaused = false;
  myServo.write(0); // サーボを0度へ
  clearLCD();
  writeCommand(0x08); // Display OFF
}

// モード切り替え (作業 <-> 休憩)
void toggleMode() {
  if (currentMode == MODE_WORK) {
    startBreak();
  } else {
    startWork();
  }
}

// タイマーリセット
void resetTimer() {
  if (currentMode == MODE_WORK) {
    remainingSeconds = 25 * 60;
  } else if (currentMode == MODE_BREAK) {
    remainingSeconds = 5 * 60;
  }
  updateTimeDisplay();
}

// 時間表示更新
void updateTimeDisplay() {
  int m = remainingSeconds / 60;
  int s = remainingSeconds % 60;
  
  setCursor(0, 1);
  if (m < 10) writeData('0');
  printNumber(m);
  writeData(':');
  if (s < 10) writeData('0');
  printNumber(s);
}

// 文字列を表示する関数
void printString(char *str) {
  while (*str) {
    writeData(*str);
    str++;
  }
}

// 数字を表示する関数
void printNumber(int num) {
  String str = String(num);
  for (int i = 0; i < str.length(); i++) {
    writeData(str[i]);
  }
}

// カーソル位置を指定する関数 (col: 0-7, row: 0-1)
void setCursor(int col, int row) {
  int row_offsets[] = { 0x00, 0x40 };
  writeCommand(0x80 | (col + row_offsets[row]));
}

// 画面をクリアする関数
void clearLCD() {
  writeCommand(0x01);
  delay(20); // Clear Displayは時間がかかる
}

// データ書き込み
void writeData(byte t_data) {
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0x40);
  Wire.write(t_data);
  Wire.endTransmission();
  delay(1);
}

// コマンド書き込み
void writeCommand(byte t_command) {
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0x00);
  Wire.write(t_command);
  Wire.endTransmission();
  delay(10);
}

// 液晶初期化
void init_LCD() {
  delay(100);
  writeCommand(0x38); // FUNCTION SET
  delay(20);
  writeCommand(0x39); // IS=1
  delay(20);
  writeCommand(0x14); // INT OSC FREQUENCY
  delay(20);
  writeCommand(0x7A); // CONTRAST SET 0,1,2,3
  delay(20);
  writeCommand(0x54); // CONTRAST SET 4,5
  delay(20);
  writeCommand(0x6C); // F0LLOWER CONTROL
  delay(20);
  writeCommand(0x38); // IS=0
  delay(20);
  writeCommand(0x0C); // Display ON
  delay(20);
  writeCommand(0x01); // Clear Display
  delay(20);
  writeCommand(0x06); // Entry Mode
  delay(20);
}
