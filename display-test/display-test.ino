#include <Wire.h>

// デバイスアドレス(スレーブ)
uint8_t DEVICE_ADDRESS = 0x3E;

char line1[] = "\314\337\301\323\335\303";
char line2[] = "petitmonte.com";
char test[] = "1234567890123456"; // テスト用

void setup() {
  // マスタとしてI2Cバスに接続する
  Wire.begin();

  // LCDの初期設定
  init_LCD();

  // カーソルをホームへ戻す
  writeCommand(0x02); 
 
// --- クリア命令のテスト用
//　※この部分は「クリア命令のテスト用」ですので削除しても構いません。
 
  // 1234567890123456の表示
  for (int i = 0; i < sizeof(test); i++) {
    writeData(test[i]);
  }
 
  // LCDの表示をクリアする
  Wire.beginTransmission(DEVICE_ADDRESS);
    // クリア命令
    Wire.write(0x20);
    Wire.write(0x01);
  Wire.endTransmission();  
 
// ---  ココマデ
 
}
 
void loop() {
  // カーソルをホームへ戻す
  writeCommand(0x02); 
 
  // プチモンテの表示
  for (int i = 0; i < sizeof(line1); i++) {
    writeData(line1[i]);
  }
 
  // petitmonte.comの表示
  for (int i = 0; i < sizeof(line2); i++) {
    writeCommand(0x40 + 0x80 + i);
    writeData(line2[i]);
  }
}
 
// データ書き込み
void writeData(byte t_data)
{
  Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x40);
    Wire.write(t_data);
  Wire.endTransmission();
  delay(1);
}
 
// コマンド書き込み
void writeCommand(byte t_command)
{
  Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x00);
    Wire.write(t_command);
  Wire.endTransmission();
  delay(10);
}
 
// 液晶初期化
void init_LCD()
{
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