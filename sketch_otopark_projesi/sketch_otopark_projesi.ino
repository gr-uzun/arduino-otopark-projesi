/*
   Akilli Otopark Projesi

   Kaynaklar :

  Ultrasonic sensor SR04
  https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/

  LCD(Liquid crystal display) LCM1602 IIC
  https://arduino-info.wikispaces.com/LCD-Blue-I2C

   @author : Ugur Enes UZUN,Gurkan UZUN
   @since : 15.04.2018
*/

#include <LiquidCrystal.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Library initialization with 16x2 size settings
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27

// defines pins numbers,ultrasonic sensors SR04
const int trigPins[2] = {2, 4};
const int echoPins[2] = {3, 5};

// 2 servo motors
Servo servoMotors[2];

// defines constants
const unsigned int KAPI_ARAC_GIRIS = 0;
const unsigned int KAPI_ARAC_CIKIS = 1;
const unsigned int ARAC_YOK = 0;
const unsigned int ARAC_VAR = 1;
const unsigned int T1 = 100; //bekleme suresi 1(dongu sonu bekleme)
const unsigned int T2 = 50; //bekleme suresi 2 (giris/cikis arasinda bekleme)
const unsigned int ARAC_MESAFE_MIN = 3;
const unsigned int ARAC_MESAFE_MAX = 15;
const unsigned int ARAC_ADET_MAX = 5;
const unsigned long KAPI_ACIK_KALMA_SURESI_MIN = 500;
const unsigned int KAPI_KAPALI = 0;
const unsigned int KAPI_ACIK = 1;

// defines variables
int aracAdet;
int bosYerAdedi;
int aracDurums[2] = {ARAC_YOK, ARAC_YOK};
int oncekiAracDurums[2] = {ARAC_YOK, ARAC_YOK};
long kapiAcikKalmaSuresi[2] = {0, 0};
int kapiDurums[2] = {KAPI_KAPALI, KAPI_KAPALI};

void setup() {
  Serial.begin(9600); // Starts the serial communication

  lcd.init();
  lcd.backlight();

  int i = 0;

  //ultrasonic sensors SR04
  for (i = 0; i < 2; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  // servo motor pinlerini tanimla.
  servoMotors[KAPI_ARAC_GIRIS].attach(8);
  servoMotors[KAPI_ARAC_CIKIS].attach(9);

  //her 2 kapi kapali.
  for (i = 0; i < 2; i++) {
    servoMotors[i].write(0);
  }

  aracAdet = 0;
}

void loop()
{
  lcd.clear();
  lcd.setCursor(0, 0);

  // KAPI_ARAC_GIRIS ve KAPI_ARAC_CIKIS icin yeni dongu(2 tur)
  int i = 0;
  for (i = 0; i < 2; i++) {
    aracDurums[i] = aracBul(i);

    bosYerAdedi = ARAC_ADET_MAX - aracAdet;
    lcd.setCursor(0, 0);
    if (bosYerAdedi > 0) {
      lcd.print("BOS YER : ");
      lcd.print(bosYerAdedi);
    } else {
      lcd.print("* OTOPARK DOLU *");
    }

    lcd.setCursor(0, 1);
    if (aracDurums[i] == ARAC_VAR) {
      kapiAc(i);
      // arac yok ise bir sure bekledikten sonra kapiyi kapat.
    } else if (aracDurums[i] == ARAC_YOK && kapiAcikKalmaSuresi[i] > KAPI_ACIK_KALMA_SURESI_MIN) {
      kapiKapa(i);
    }

    if (aracDurums[i] == ARAC_VAR && oncekiAracDurums[i] == ARAC_YOK) {
      if (i == KAPI_ARAC_GIRIS) {
        aracAdet++;
      } else if (i == KAPI_ARAC_CIKIS) {
        aracAdet--;
      }
    }

    if (aracAdet < 0) {
      aracAdet = 0;
    } else if (aracAdet > ARAC_ADET_MAX) {
      aracAdet = ARAC_ADET_MAX;
    }

    oncekiAracDurums[i] = aracDurums[i];
    if (i < 1) {
      delay(T2);
    }

  }
  delay(T1);
  // her 2 kapi acik kalma suresine T1 zamanini ekle.
  int j = 0;
  for (j = 0; j < 2; j++) {
    if (kapiDurums[j] == KAPI_ACIK) {
      kapiAcikKalmaSuresi[j] = kapiAcikKalmaSuresi[j] + T1 + T2;
    }
  }

}

void kapiAc(int kapiNo) {
  if (kapiNo == KAPI_ARAC_GIRIS) {
    lcd.print("ARAC GIRIS");
    //bos yer var ise giris kapisini ac.
    if (bosYerAdedi > 0) {
      // kapi acilinca kapi acik kalma suresini sifirla.
      kapiDurums[kapiNo] = KAPI_ACIK;
      kapiAcikKalmaSuresi[kapiNo] = 0;
      servoMotors[kapiNo].write(90);
    }
    //cikis kapisini ac.
  } else if (kapiNo == KAPI_ARAC_CIKIS) {
    lcd.print("ARAC CIKIS");
    // kapi acilinca kapi acik kalma suresini sifirla.
    kapiDurums[kapiNo] = KAPI_ACIK;
    kapiAcikKalmaSuresi[kapiNo] = 0;
    servoMotors[kapiNo].write(90);
  }
}

void kapiKapa(int kapiNo) {
  // kapi kapaninca kapi acik kalma suresini sifirla.
  kapiDurums[kapiNo] = KAPI_KAPALI;
  kapiAcikKalmaSuresi[kapiNo] = 0;
  servoMotors[kapiNo].write(0);
}

int aracBul(int kapiNo) {
  // arac yok ise 0 var ise 1 dondurur.
  int donenDeger = ARAC_YOK;

  // Clears the trigPin
  digitalWrite(trigPins[kapiNo], LOW);

  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPins[kapiNo], HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPins[kapiNo], LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPins[kapiNo], HIGH);

  // Calculating the distance
  int distance = duration * 0.034 / 2;

  Serial.print(kapiNo);
  Serial.print(".kapi[cm]: ");
  Serial.println(distance);

  if (ARAC_MESAFE_MIN < distance && distance < ARAC_MESAFE_MAX) {
    donenDeger = ARAC_VAR;
  } else {
    donenDeger = ARAC_YOK;
  }

  return donenDeger;
}

