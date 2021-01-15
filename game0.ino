#include <Arduino.h>
#include <U8x8lib.h>

U8X8_SSD1306_128X64_ALT0_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

const int rotaryPin = A0;
const int buttonPin = 6;
const int buzzerPin = 5;

int buttonPreState = 0;
int buttonState = 0;


int bullet[] = {-1,-1};
int enemy[] =
{
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 //0が死亡、1が生存
};

void setup(void) {
 u8x8.begin();
 u8x8.setFlipMode(1);
 pinMode(rotaryPin, INPUT);
 pinMode(buttonPin, INPUT);
 pinMode(buzzerPin, OUTPUT);
 analogWrite(buzzerPin, 300);
 delay(100);
 analogWrite(buzzerPin, 0);
 Serial.begin(9600);
 
}

void loop(void) {
 int rotaryValue = map(analogRead(rotaryPin), 0,1023, 0,15); 
 //Serial.println(rotaryValue);
 
 u8x8.setFont(u8x8_font_chroma48medium8_r);
 
 //自分の動き
 u8x8.setCursor(rotaryValue, 3);//長辺方向(0~15), 短辺方向(0~3)
 u8x8.print("O");

 //bulletの動き
 ///ボタン押したら発射
  buttonPreState = buttonState;
  delay(10);
  buttonState = digitalRead(buttonPin);
  
  if (buttonPreState == 0 && buttonState == 1){//押した瞬間、発射
    bullet[0] = rotaryValue;
    bullet[1] = 3;
  }
  ///bullet描画
  if (bullet[1] >= 0 && bullet[1] <= 3){
    u8x8.setCursor(bullet[0], bullet[1]);
    u8x8.print("'");
    bullet[1]--;//更新
  }

  //enemyの動き
  for (int i=0; i<16; i++){
    if(enemy[i]==1){
      u8x8.setCursor(i,0);
      if (bullet[0] == i && bullet[1] == 0){//bulletに当たってたら
        u8x8.print("*");
        enemy[i]=0;
      }else{//bulletに当たってなかったら
        u8x8.print("&");
      }
    }    
  }
 

 delay(100); 
 u8x8.clearDisplay();
}
