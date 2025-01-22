/***********************************WIRING***************************************************************************

              MFRC522      Arduino      
              Reader/PCD   Uno/101      
 * Signal      Pin          Pin           
 * --------------------------------------
 * RST/Reset   RST          9             
 * SPI SS      SDA(SS)      10            
 * SPI MOSI    MOSI         11 / ICSP-4   
 * SPI MISO    MISO         12 / ICSP-1   
 * SPI SCK     SCK          13 / ICSP-3   
 * 
 * 
 * 
 * lcd 20x4     Arduino UNO
 * ---------------------------------------
 * SDA          A4
 * SCL          A5
 * VCC          5 volt
 * GND          GND
  */
/********************************************************************************************************************/
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); 

#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
 
//========================================================
//pin 
byte rowPins[ROWS] = {A0,8,7,6};
byte colPins[COLS] = {5, 4,3,2};
 
 
//Variabel lcd dan menu
#define lebarTextLCD 20 + 1
#define menuLevel 2
//========================================================

//variable rfid tambah saldo
bool notif = true;
bool isiSaldo = false;
String isiSldo ="";
String input;
long saldo;
int digit;

long OLDsaldo;
int OLDdigit;
//end variable

String batal="";//pembatalan keluar menu

//variable lihat saldo
bool tapkartu=false;
//end

//isi saldo
long nilaiBaru;
bool tapisisaldo=false;
//

//merchan
bool bayarmerchant=false;
//end

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
 
enum mode 
{
  UInt8,
  UInt16,
  textDropDown,
  subMenu
};
 
struct Menu
{
  char text[lebarTextLCD];
  byte tipe;
  void *variabel;
  uint16_t nilaiMin;
  uint16_t nilaiMax;
  void *subMenu;
  byte jumlahBaris;
};
 
struct MenuIndex
{
  byte index;
  Menu *menu;
  byte menuLength;
  char *dropDown;
  byte dropDownLength;
};
 
//========================================================
//variabel
byte cs;
byte ts;
uint16_t mrc;

const Menu menuUtama[] = 
{  
  //lebarTextLCD,      tipe,        variabel, nilaiMin,   nilaiMax,  subMenu,jumlahBaris
  {"1.Cek Saldo     ", textDropDown,&cs,      28,         36,        0,      0 },
  {"2.Topup Saldo   ", UInt8,       &ts,      50,         100,       0,      0 }, 
  {"3.Transfer         ", UInt16,      &mrc,     5,          300,       0,      0 },
};

//========================================================
 
MenuIndex menuIndex[menuLevel];
 
long millismenuText;
String menuEntriNilai;
int8_t levelMenu = -1;
bool entriNilai;
byte menuTextIndex;
char *judulMenu;
byte judulMenuTampil;
byte lcdEntriPos;
 
 
void setup() 
{
  Serial.begin(9600);
  SPI.begin();      
  Wire.begin();  

  mfrc522.PCD_Init(); // Init MFRC522 card

  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
    
  lcd.begin(20, 4);
  lcd.backlight();
  menuIdle();
 
  millismenuText = millis();
}
 
void menuIdle()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tekan * untuk");
  lcd.setCursor(0, 1);
  lcd.print("masuk ke menu");
}
 
void loop() 
{
  cekMenu();   
}
 
void cekMenu()
{
  char keyp = keypad.getKey();
  if (keyp){
    if(keyp == '*'){
      if(entriNilai){
        nilaiBaru = menuEntriNilai.toInt(); 
        if((nilaiBaru >=0)){
          switch(menuIndex[levelMenu].menu[menuIndex[levelMenu].index].tipe){
            case UInt8:                                          
              tapisisaldo = false;
              isiSldo="";
              batal="";
              while (tapisisaldo!=true){
                tambah_saldo();
                if (batal=="ya"){
                  break;       
                }
              }
              if (isiSldo=="sukses"){
                  lcd.clear();
                  lcd.print(judulMenu);
                  lcd.setCursor(0, 1);
                  lcd.print(nilaiBaru);
                  lcd.print(" disimpan");     
              }
              break;
            case UInt16:
              bayarmerchant=false;
              batal="";
              while (bayarmerchant!=true){
                merchant();
                if (batal=="ya"){
                  break;       
                }
              }
              break;
            case textDropDown:              
              break;
          }          
          delay(1000);
        }
        entriNilai = false;
      }else if(levelMenu == -1){
        levelMenu = 0;
      }else{
        levelMenu--;
      }
      displayMenu();
    }else if(keyp == '#'){
      if(entriNilai){
        entriNilai = false;
        displayMenu();
      }else if(levelMenu >= 0){
        levelMenu--;
        displayMenu();
      }
    }else if((keyp >= '0') || (keyp <= '3') ){
      if(entriNilai){
          menuEntriNilai += keyp;
          lcd.setCursor(lcdEntriPos++, 1);
          lcd.print(keyp);
      }else if((keyp != '0') && (keyp - '0' <= menuIndex[levelMenu].menuLength)){//pilihan menu      
          menuIndex[levelMenu].index = keyp - '1';
          judulMenu = menuIndex[levelMenu].menu[menuIndex[levelMenu].index].text;
          lcd.clear();
          lcd.print(judulMenu);
          lcd.setCursor(0, 1);
          lcdEntriPos = 9;
          judulMenuTampil = 0;
   
          uint16_t nilaiUInt8;
          uint16_t nilaiUInt16;
          uint16_t nilaiUInt32;
          int16_t nilaiFloat;
   
          menuEntriNilai = "";
          entriNilai = true;
 
          switch(menuIndex[levelMenu].menu[menuIndex[levelMenu].index].tipe){
            case UInt8:            
              lcd.print("Topup Rp.");
              break;
            case UInt16:            
              lcd.print("Transfer Rp.");
              break;
            case textDropDown:            
              tapkartu=false;
              batal="";
              while (tapkartu==false){
                Serial.println("while "+batal);
                lihatsaldo();       
                if (batal=="ya"){
                  break;       
                }
              }       
              levelMenu=0;                    
              entriNilai = false;              
              displayMenu();
              break;          
          }
      }
    }
  }
  
  if(millis() - millismenuText > 2000){
    millismenuText = millis();     
    if(menuIndex[levelMenu].dropDownLength != 0){
      if(menuTextIndex >= menuIndex[levelMenu].dropDownLength){
        menuTextIndex = 0;
      }
      lcd.setCursor(0, 1);
      lcd.print(menuIndex[levelMenu].dropDown + (menuTextIndex++ * (lebarTextLCD)));
    }
    
    if(entriNilai){      
      lcd.setCursor(0, 0);
      if((judulMenuTampil % 3) == 0){
        lcd.print(judulMenu);
      }else if((judulMenuTampil % 3) == 1){
        lcd.print("* untuk lanjut  ");
      }else{
        lcd.print("# untuk batal   ");
      }
      judulMenuTampil++;
    }else if(levelMenu != -1){
      if(menuTextIndex >= menuIndex[levelMenu].menuLength){
        menuTextIndex = 0;
      }
      lcd.setCursor(0, 1);
      lcd.print(menuIndex[levelMenu].menu[menuTextIndex++].text);
    }
  }
}
 
void displayMenu()
{
  if(levelMenu == -1){
    menuIdle();
    menuIndex[levelMenu].dropDownLength = 0;
  }else if(levelMenu == 0){
    menuIndex[levelMenu].index = 0;
    menuIndex[levelMenu].menu = menuUtama;
    menuIndex[levelMenu].menuLength = sizeof(menuUtama)/sizeof(menuUtama[0]);
    menuIndex[levelMenu].dropDownLength = 0;
    menuTextIndex = 0;
 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pilih [1..");
    lcd.print(sizeof(menuUtama)/sizeof(menuUtama[0]));
    lcd.print("]");
  }else{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menuIndex[levelMenu-1].menu[menuIndex[levelMenu-1].index].text);
    menuIndex[levelMenu].dropDownLength = 0;
  }
}
