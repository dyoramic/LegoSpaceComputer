// Required libraries
#include <SPIFFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPIFFS_ImageReader.h>
#include <LinkedList.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ezTime.h>

char ssid[] = "";       // your network SSID (name)
char password[] = "";  // your network key
WiFiClientSecure client;

#define BLUE            0x001F
#define GREEN           0x07E0
#define BLACK           0x0000
#define WHITE           0xFFFF

// Display interface configuration
#define TFT_CS_PIN    26
#define TFT_DC_PIN    33
#define TFT_MOSI_PIN  23
#define TFT_SCLK_PIN  18
#define TFT_RST_PIN   32
Adafruit_SSD1351 tft = Adafruit_SSD1351(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_SCLK_PIN, TFT_RST_PIN);

// Image reader
SPIFFS_ImageReader reader;

// Fact variables
int numSpaceFacts = 15;
char *spaceFacts[] = {"Space is completely \nsilent",
                      "The hottest planet in our solar system is 450C",
                      "The temperature in \nthe void of space is about -270C",
                      "If you could fly a \nplane to Pluto, it \nwould take more than 800 years",
                      "A NASA space suit \ncosts $12,000,000",
                      "One million earths \ncan fit inside the \nsun",
                      "There are more trees on earth than stars \nin the milk way",
                      "One day on venus is \nlonger than one year",
                      "The sunset on Mars \nappears blue",
                      "Footprints on the \nmoon will stay for \nmillions of years",
                      "Some planets have no surface to land on \n(Jupiter, Saturn, \nUranus and Neptune)",
                      "Saturn would flaot in water",
                      "It would take 9.5 \nyears to walk to the moon",
                      "The sun makes up \n99.8% on the solar \nsystems mass",
                      "Almost 6,000 \nsatellites orbit the Earth"};



// Button variables
#define button1Pin 21
#define button2Pin 22
#define modeButtonPin 15
int button1State = 1;
int button1LastState = 1;
int button2State = 1;
int button2LastState = 1;
int modeButtonState = 1;
int modeButtonLastState = 1;

// Game variables and class
class Block {
  public:
    int x;
    int y;
};

int spaceShipX = 0;
int oldSpaceShipX = 0;
unsigned long lastUpdateTime;
unsigned long updateInterval = 50;
LinkedList<Block*> blocks;
int blockAdded = 0;
bool shipHit = false; 
int blockInterval = 25;

// Time date variables
Timezone myTZ;
int oldMin;
char location[] = "Asia/Muscat";

// Progam vars
int progMode = 0;
int numProgStates = 6;
bool modeChanged = false;

// Radar
int radarState1;
int radarState2;

// Like
int yTextPos;

void setup() {
  Serial.begin(115200);

  // initialize SPIFFS
  if(!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1);
  }

  startWiFi();
  initalizeTimeDate();
  
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(modeButtonPin, INPUT_PULLUP);
  tft.begin();
  showIntro();
  progMode = 0;
  modeChanged = true;
}

void loop() {
  modeButtonState = digitalRead(modeButtonPin);
  if(modeButtonState != modeButtonLastState) {
    if(modeButtonState == HIGH) {
      progMode = (progMode + 1) % numProgStates;
      modeChanged = true; 
      Serial.println(progMode);
    }
    delay(25);
  }
  
  modeButtonLastState = modeButtonState;
  
  if(progMode == 0) {
    if(modeChanged == true) {
      //reader.drawBMP("/cross_9.bmp", tft, 0, 0);
      tft.fillScreen(GREEN);
      tft.fillRoundRect(128/2-6, 6, 12, 128-12, 5, WHITE);
      tft.fillRoundRect(6, 128/2-6, 128-12, 12, 7, WHITE);
      modeChanged = false;
    }
  } else if(progMode == 1) {
    if(modeChanged == true) {
      initRadar();
      modeChanged = false;
    }
    updateRadar();
  } else if(progMode == 2) {
    if(modeChanged == true) {
      initSpaceFacts();
      modeChanged = false;
    }
    updateSpaceFacts();
  } else if(progMode == 3) {
    if(modeChanged == true) {
      gameSetup();
      modeChanged = false;
    }
    processGame();
  } else if(progMode == 4) {
    if(modeChanged == true) {
      initTimeDate();
      modeChanged = false;
    }
    updateTimeDate();
  } else if(progMode == 5) {
    if(modeChanged == true) {
      initLike();
      modeChanged = false;
    }
    updateLike();
  }
  
}

void startWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
}

void initalizeTimeDate() {
  waitForSync();
  myTZ.setLocation(F(location));  
}



void gameSetup() {
  tft.fillScreen(BLACK);
  shipHit = false; 
  blocks.clear();
  Block *blockToAdd = new Block();
  blockToAdd->x = 10;
  blockToAdd->y = 0;
  blocks.add(blockToAdd);
  lastUpdateTime = millis();
  spaceShipX = 64;
  blockAdded = blockInterval;
  updateInterval = 50;
}

void processGame() {
  if(!shipHit) {
    button1State = digitalRead(button1Pin);
    button2State = digitalRead(button2Pin);
    if(button1State != button1LastState) {
      if(button1State == HIGH) {
        spaceShipX += 10; 
      }
      delay(25);
      //updateScreen();
    }
    if(button2State != button2LastState) {
      if(button2State == HIGH) {
        spaceShipX -= 10; 
      }
      delay(25);
    } 
    button1LastState = button1State;
    button2LastState = button2State;
  
    if(lastUpdateTime + updateInterval < millis()){
      updateScreen();
    }
  }
}

void updateScreen() {
  drawSpaceShip();
  updateBlocks();
  lastUpdateTime = millis();
}

void updateBlocks() {
  Block *block;
  int i = 0;
  while(i < blocks.size() && !shipHit) {
    block = blocks.get(i);
    block->y = block->y + 2;
    if(block->y > 128) {
      blocks.remove(i);
    }
    drawBrick(block->x, block->y);
    calculateShipImpact(block->x, block->y);
    i++;
  }

  if(blockAdded-- == 0) {
    addNewBlock();
    blockAdded = blockInterval;
    if(blockInterval > 15) {
      blockInterval--;
    }
  }
}

void addNewBlock() {
  Block *blockToAdd = new Block();
  blockToAdd->x = random(102);
  blockToAdd->y = 0;
  blocks.add(blockToAdd);
}

void calculateShipImpact(int x, int y) {
  if(y > 92) {
    if(x < spaceShipX + 14 && x + 25 > spaceShipX) {
      shipHit = true;
      updateInterval = 100000;
      printGameOver();
    }
  }
  // missed
}

void printGameOver() {
  tft.fillRect(20, 10, 100, 65, BLACK);
  tft.setTextSize(4);
  tft.setTextColor(GREEN);
  tft.setCursor(20, 10);
  tft.print("GAME");
  tft.setCursor(20, 45);
  tft.print("OVER");
}

void drawSpaceShip() {
  if(spaceShipX != oldSpaceShipX) {
    int y = 112;
    tft.fillRect(oldSpaceShipX, y, 15, 14, BLACK);
    int x = spaceShipX;
    if(x > 114) {
      x = 114;
    } else if (x < 0) {
      x = 0;
    }
    drawShipShape(x, y);
    oldSpaceShipX = x;
  }
}

void drawShipShape(int x, int y) {
  tft.drawLine(x+7, y, x, y+13, GREEN);
  tft.drawLine(x+7, y, x+14, y+13, GREEN);
  tft.drawLine(x, y+13, x+7, y+9, GREEN);
  tft.drawLine(x+14, y+13, x+7, y+9, GREEN);
}

void drawBrick(int x, int y) {
  tft.fillRect(x, y-2, 25, 20, BLACK);
  tft.drawRect(x, y+4, 25, 15, GREEN);
  tft.drawRect(x+3, y, 7, 5, GREEN);
  tft.drawRect(x+15, y, 7, 5, GREEN);
}

void drawSingleBrick(int x, int y) {
  tft.drawRect(x, y+4, 13, 15, GREEN);
  tft.drawRect(x+3, y, 7, 5, GREEN);
}

void showIntro() {
  reader.drawBMP("/space_0.bmp", tft, 0, 0);
  delay(250);
  reader.drawBMP("/space_1.bmp", tft, 0, 0);
  delay(50);
  reader.drawBMP("/space_2.bmp", tft, 0, 0);
  delay(50);
  reader.drawBMP("/space_3.bmp", tft, 0, 0);
  delay(50);
  reader.drawBMP("/space_4.bmp", tft, 0, 0);
  delay(50);
  reader.drawBMP("/space_5.bmp", tft, 0, 0);
  delay(50);
  reader.drawBMP("/space_6.bmp", tft, 0, 0);
  delay(1000);
}

void showSpaceFacts() {
  for(int i = 0; i < numSpaceFacts; i++) {
    displaySpaceFact(i);
    delay(4000);
  }
}

void initSpaceFacts() {
  tft.fillScreen(BLACK);
  displaySpaceFact(random(numSpaceFacts));
  updateInterval = 4000;
  lastUpdateTime = millis();
}

void updateSpaceFacts() {
  if(lastUpdateTime + updateInterval < millis()){
    displaySpaceFact(random(numSpaceFacts));
    lastUpdateTime = millis();
  }
}

void displaySpaceFact(int i) {
  tft.fillScreen(BLACK);
  tft.setTextSize(1);
  tft.setCursor(0,0);
  tft.setTextColor(GREEN);
  tft.print(spaceFacts[i]);
}

void initRadar() {
  tft.fillScreen(BLACK);
  tft.fillRoundRect(128/2-6, 6, 12, 128-12, 5, GREEN);
  tft.fillRoundRect(6, 128/2-6, 128-12, 12, 7, GREEN);
  lastUpdateTime = millis();
  updateInterval = 1000;
  radarState1 = 0;
  radarState2 = 0;
}

void updateRadar() {
  bool changeState = false;
  if(lastUpdateTime + updateInterval < millis()){
    if(radarState1 == 0) {
      changeState = processRadarShip(20, 20);
    }else if(radarState1 == 1) {
      changeState = processRadarShip(10, 100);
    }else if(radarState1 == 2) {
      changeState = processRadarShip(90, 10);
    }else if(radarState1 == 3) {
      changeState = processRadarShip(90, 110);
    }else if(radarState1 == 4) {
      changeState = processRadarShip(30, 90);
    }

    if(changeState){
      radarState1 = (radarState1 + 1) % 5;
      Serial.println(radarState1);
    }
  }
 
}

bool processRadarShip(int x, int y) {
  
  if(radarState2 == 0) {
    drawShipShape(x, y);
  }else if(radarState2 == 1) {
    tft.drawCircle(x+7, y+7, 10, GREEN);
  }else if(radarState2 == 2) {
    tft.drawCircle(x+7, y+7, 10, BLACK);
    tft.drawCircle(x+7, y+7, 15, GREEN);
  }else if(radarState2 == 3) {
    tft.drawCircle(x+7, y+7, 15, BLACK);
    tft.drawCircle(x+7, y+7, 20, GREEN);
  }else if(radarState2 == 4) {
    tft.drawCircle(x+7, y+7, 20, BLACK);
    tft.drawCircle(x+7, y+7, 15, GREEN);
  }else if(radarState2 == 5) {
    tft.drawCircle(x+7, y+7, 15, BLACK);
  }else if(radarState2 == 6) {
    tft.fillRect(x, y, 15, 15, BLACK);
    radarState2 = 0;
    lastUpdateTime = millis();
    updateInterval = random(500, 2000);
    return true;
  }
  radarState2++;
  lastUpdateTime = millis();
  updateInterval = 200;
  return false;
}

void drawRadarShip(int x, int y) {
  drawShipShape(x, y);
  delay(200);
  for(int i = 0; i < 3; i++) {
    tft.drawCircle(x+7, y+7, 10, GREEN);
    delay(200);
    tft.drawCircle(x+7, y+7, 10, BLACK);
    tft.drawCircle(x+7, y+7, 15, GREEN);
    delay(200);
    tft.drawCircle(x+7, y+7, 15, BLACK);
    tft.drawCircle(x+7, y+7, 20, GREEN);
    delay(200);
    tft.drawCircle(x+7, y+7, 20, BLACK);
    tft.drawCircle(x+7, y+7, 15, GREEN);
    delay(200);
    tft.drawCircle(x+7, y+7, 15, BLACK);
    delay(200);
  }
  tft.fillRect(x, y, 15, 15, BLACK);
}

void initTimeDate() {
  tft.fillScreen(BLACK);
  drawBrick(0, 10);
  drawBrick(25, 10);
  drawBrick(50, 10);
  drawBrick(75, 10);
  drawBrick(100, 10);
  drawBrick(0, 100);
  drawBrick(25, 100);
  drawBrick(50, 100);
  drawBrick(75, 100);
  drawBrick(100, 100);
  tft.setTextColor(GREEN);
  oldMin = -1;
}

void updateTimeDate() {
  if(myTZ.minute() != oldMin) {
    tft.fillRect(0, 40, 128, 50, BLACK);
    displayTime();
    displayDate();
    oldMin = myTZ.minute();
  }
}

void displayTime() {
  tft.setTextSize(4);
  tft.setCursor(0, 40);
  int hour = myTZ.hour();
  zeroPad(myTZ.hour());
  tft.print(myTZ.hour());
  tft.print(":");
  zeroPad(myTZ.minute());
  tft.print(myTZ.minute());
}

void displayDate() {
  tft.setTextSize(2);
  tft.setCursor(0, 75);
  zeroPad(myTZ.day());
  tft.print(myTZ.day());
  tft.print("/");
  zeroPad(myTZ.month());
  tft.print(myTZ.month());
  tft.print("/");
  tft.print(myTZ.year());
}

void zeroPad(int num) {
  if(num < 10) {
    tft.print("0");
  }
}

void initLike() {
  tft.fillScreen(BLACK);
  tft.setTextColor(GREEN);
  tft.setTextSize(4);
  lastUpdateTime = millis();
  updateInterval = 300;
  yTextPos = -40;
}

void updateLike() {
  if(lastUpdateTime + updateInterval < millis()){
    lastUpdateTime = millis();
    tft.fillRect(20, yTextPos, 100, 40, BLACK);
    if(yTextPos > 128) {
      yTextPos = -30;
    } else {
      yTextPos = yTextPos + 5;
    }
    tft.setCursor(20, yTextPos);
    tft.print("LIKE");
  }
  
}
