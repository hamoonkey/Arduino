//電卓

#include <Arduino.h>
#include <U8x8lib.h>

U8X8_SSD1306_128X64_ALT0_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

const int rotaryPin = A0;
const int buttonPin = 6;
const int buzzerPin = 5;
const int ledPin = 4;

int buttonPreState = 0;
const char key[] = "0123456789+-*/=C";
int input = 0;
double output = 0;
int mark = 0;
const int note0 = 262;
const int note1 = 523;

void setup(void) {
 u8x8.begin();
 u8x8.setFlipMode(1);
 u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
 
 pinMode(rotaryPin, INPUT);
 pinMode(buttonPin, INPUT);
 pinMode(buzzerPin, OUTPUT);
 pinMode(ledPin, OUTPUT);

 //開始音
 analogWrite(buzzerPin, 300);
 delay(100);
 analogWrite(buzzerPin, 0);
 
 Serial.begin(9600);
 
}

void loop(void) {
 int rotaryValue = map(analogRead(rotaryPin), 0,1023, 0,15);

 //選択
 ///矢印
 u8x8.setCursor(rotaryValue, 2);
 u8x8.print("\x8e");
 ///key
 u8x8.setCursor(0, 3);//長辺方向(0~15), 短辺方向(0~3) 
 for (int i=0;i<=15;i++){
  u8x8.print(key[i]);
 }

 //決定
  int buttonState = digitalRead(buttonPin);
  
  if (buttonPreState == 1 && buttonState == 0){//押して離した瞬間、決定
    delay(200);
    u8x8.clearLine(0);
    u8x8.clearLine(1);
    
    if(rotaryValue>=10 && rotaryValue<=14){//記号が選択されてたら
      switch(mark){//前回選択によって場合分け
        case 0://初回
          output = input;
          input = 0;
          break;
        case 10://+
          output = output + input;
          input = 0;
          break;
        case 11://-
          output = output - input;
          input = 0;
          break;
        case 12://*
          output = output * input;
          input = 0;
          break;
        case 13://÷
          output = output / input;
          input = 0;
          break;
        case 14://等号
          output = output;
          input = 0; 
      }
      
      if(rotaryValue == 14){//等号が選択されてたら、ぴっぴっぴ
        //Serial.println("equal");
        mark = 14;
        u8x8.draw1x2String(1,0,".");
        analogWrite(buzzerPin, 131);
        delay(100);
        analogWrite(buzzerPin, 0);
        delay(800);
        u8x8.draw1x2String(2,0,".");
        analogWrite(buzzerPin, 147);
        delay(100);
        analogWrite(buzzerPin, 0);
        delay(800);
        u8x8.draw1x2String(3,0,".");
        analogWrite(buzzerPin, 165);
        delay(100);
        analogWrite(buzzerPin, 0);
        delay(800);
        u8x8.draw1x2String(4,0,"!!!");
        digitalWrite(ledPin, HIGH);
        analogWrite(buzzerPin, 175);
        delay(700);
        digitalWrite(ledPin, LOW);
        analogWrite(buzzerPin, 0);
                
        u8x8.clearLine(0);
        u8x8.clearLine(1);
        u8x8.setCursor(0,0);
        u8x8.print("The answer is");
        u8x8.setCursor(0,1);
        u8x8.print(output);
      }else{
        //Serial.println("mark");
        mark = rotaryValue;     
        u8x8.setCursor(0,0);
        u8x8.print(output);
      }
    }else if(rotaryValue==15){
      //Serial.println("Clear");
      mark = 0;
      input = 0;
      output = 0; 
    }else{
      //Serial.println("Number");
      input = input*10 + int(key[rotaryValue]) -48;
      u8x8.setCursor(0,0);
      u8x8.print(input);
    }
  }
  buttonPreState = buttonState;

 u8x8.clearLine(2);
}
