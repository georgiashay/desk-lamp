// This #include statement was automatically added by the Particle IDE.
#include "structures.h"

// This #include statement was automatically added by the Particle IDE.
#pragma SPARK_NO_PREPROCESSOR

#include <FastLED.h>

//include the other files

//the main object types I am using
#include "structures.h"
//all of the different patterns
#include "patterns.h"

#include <string>
#include <vector>

//pin definitions
#define VOLUME_POT A0
#define SWIPE_STRIPE A1
#define CS A2
#define BRIGHTNESS_POT A4
#define SPEAKER_ON DAC
#define SPEC_LEVEL WKP
#define LED3 D0
#define LED1 D1
#define LED2 D2
#define LED4 D3
#define AUD_SELECT D4
#define MUTE_MUX D5
#define SPEC_STROBE D6
#define SPEC_RESET D7
#define PIXEL_TYPE WS2812B
#define STRIP_LENGTH 60
#define COLOR_ORDER GRB

#define POT_DELTA 100

//for dfplayer
#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

//rates
#define MONITOR_RATE 20
#define CONNECTION_TEST 5000
#define SWITCH_PATTERN_RATE 10000

//for debugging purposes
unsigned int localPort = 8888;
UDP udp;

//led strips
CRGB leds1[STRIP_LENGTH];
CRGB leds2[STRIP_LENGTH];
CRGB leds3[STRIP_LENGTH];
CRGB leds4[STRIP_LENGTH];

//potentiometer readings
uint16_t vPotValue;
uint16_t bPotValue;
uint16_t sPotValue;

uint8_t brightness = 100;
uint8_t volume = 128;

//whether or not to monitor these values
bool monitorVPot = true;
bool monitorBPot = true;
bool monitorSPot = true;
bool monitorSerial = true;
bool monitorSpectrum = false;

//whether or not to use these values as normal
bool normalVPot = true;
bool normalBPot = true;
bool normalSPot = true;

//spectrum analyzer
int bandIndex = 0;
uint16_t bands[7];
uint16_t finalBands[7] = {0, 0, 0, 0, 0, 0, 0};

void setPlayer(std::string);
void debugUDP(String message);

CardMusicPlayer cardPlayer;
BluetoothMusicPlayer bluetoothPlayer;
MusicPlayer* player = &cardPlayer;

SwipeDetector swipeStripeDetector;

//default pattern set if the last one is not correctly downloaded from the cloud
std::vector<patternEngine*> patternList;
std::vector<std::string> codeList = {"ST", "RN"};
std::vector<std::vector<CRGB>> colorList = {{CRGB::Red, CRGB::Blue, CRGB::White},{CRGB::Blue}};
std::vector<uint16_t> speedList = {100,100};
std::vector<std::string> argList = {"",""};
uint8_t patternIndex = 0;

String audioMode = "NONE"; // NONE - no sound, CARD - only sound for patterns with sound, DUAL - card if applicable, otherwise bluetooh, BLUE - only bluetooth
String patternMode = "AUT"; //AUT = automatically advance, MAN - manually advance (with swipe stripe)

void patternLoop();
void connectionTest();
void updateLoop();
void monitorLoop();
void receivedDefaults(const char *event, const char *data);

int passData(String);

std::vector<uint8_t> DFPlayerData;
uint16_t incomingInfo;
std::string serialString = "";
std::string connectionStatus = "";


Timer patternTimer(SWITCH_PATTERN_RATE, patternLoop);
Timer monitorTimer(MONITOR_RATE, monitorLoop);
Timer updatePatternTimer(DEFAULT_SPEED, updateLoop);

bool firstRun;

void setup() {

    firstRun = true;

    //set up variables exposed to the cloud
    Particle.function("passdata",passData);
    Particle.variable("brightness",brightness);
    Particle.variable("volume",volume);
    Particle.variable("audioMode",audioMode);
    Particle.variable("patternState",patternMode);
    Particle.publish("pattern_defaults", "", PRIVATE);
    Particle.subscribe("hook-response/pattern_defaults", receivedDefaults, MY_DEVICES);

    //set up random number generators
    uint32_t seed = millis();
    srand(seed);

    //sets up debugging
    udp.begin(localPort);

    //sets up each pin
    pinMode(VOLUME_POT, INPUT);
    pinMode(SWIPE_STRIPE, INPUT);
    pinMode(SPEAKER_ON, OUTPUT);
    pinMode(SPEC_LEVEL, INPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
    pinMode(AUD_SELECT, OUTPUT);
    pinMode(MUTE_MUX, OUTPUT);
    pinMode(SPEC_STROBE, OUTPUT);
    pinMode(SPEC_RESET, OUTPUT);
    pinMode (CS, OUTPUT);

    //set up SPI connection for digital potentiometer
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV8);
    //SPI.begin(SPI_MODE_MASTER,A2);

    pinMode(BRIGHTNESS_POT, INPUT);

    //debugUDP("set SPI\n");
    Serial1.begin(9600);
    setPlayer("card");
    execute_CMD(0x3F,0,0);
    delay(3000);
    while(Serial1.available()<10) {
        delay(30);
    }
    execute_CMD(0x06,0,30);
    execute_CMD(0x0F,1 , 2);
    //debugUDP("serialed the dfplayer\n");

    //execute_CMD(0x03,0,0);

    //sets up serial connection with the bluetooth module
    setPlayer("bluetooth");
    Serial1.print("RESTORE\r");
    Serial1.print("WRITE\r");
    Serial1.print("RESET\r");
    Serial1.print("DISCOVERABLE ON\r");
    Serial1.print("WRITE\r");
    delay(500);
    Serial1.print("SET NAME=GeorgiaDeskLamp\r");
    debugUDP("blue;");
    Serial1.print("WRITE\r");
    Serial1.print("RESET\r");
    delay(500);
    Serial1.print("SET BAUD=9600\r");
    Serial1.print("WRITE\r");
    Serial1.print("RESET\r");
    delay(500);
    player->changeVolume(128);
    volume = 128;
    debugUDP("serialed the bluetooth\n");
    //sets up serial connection with the DFPlayer


    //turn on speaker
    digitalWrite(SPEAKER_ON, HIGH);
    digitalWrite(MUTE_MUX, LOW);

    //set up timers
    monitorTimer.start();
    patternTimer.start();
    updatePatternTimer.start();
   // quickTimer.start();

    //set up spectrum analyzer to start analyzing sound
    digitalWrite(SPEC_STROBE, HIGH);
    digitalWrite(SPEC_RESET, HIGH);
    delayMicroseconds(30);
    digitalWrite(SPEC_RESET, LOW);

    //sets up the LED strips
    FastLED.addLeds<PIXEL_TYPE, LED1, COLOR_ORDER>(leds1, STRIP_LENGTH);
    FastLED.addLeds<PIXEL_TYPE, LED2, COLOR_ORDER>(leds2, STRIP_LENGTH);
    FastLED.addLeds<PIXEL_TYPE, LED3, COLOR_ORDER>(leds3, STRIP_LENGTH);
    FastLED.addLeds<PIXEL_TYPE, LED4, COLOR_ORDER>(leds4, STRIP_LENGTH);

    //sets up brightness and volume
    player->changeVolume(analogRead(VOLUME_POT)/16);
    player->state = false;
    //FastLED.setBrightness(analogRead(BRIGHTNESS_POT)/16);
    FastLED.setBrightness(100);
    brightness = 100;

        debugUDP("YO IM IN THE SETUP\n");

    debugUDP("about to make the pattern list\n");
    patternList = makePatternList(codeList, colorList, speedList, argList);
    debugUDP("finished setup\n");
    debugUDP("volume: "+String(volume)+"\n");
    debugUDP("brightness: "+String(brightness)+"\n");

}

void loop() {
    //read Serial as fast as possible
    if(player->type=="bluetooth") {
        while(Serial1.available()>0) {
            char c = Serial1.read();
            serialString += c;
        }

    }
}

int passData(String command) {
    firstRun = true;
    //FRZ pauses all loops
    //PAU pauses pattern loop
    //CLR clears pattern list
    //PAT 3 chars pattern num 2 chars pattern code
    //TPC total pattern count
    //COL 3 chars pattern num 3 chars color number 6 chars hex color
    //DUR duration of each pattern in seconds
    //SPD 3 chars pattern num, ms between updaates
    //AUD 4 chars audio mode - NONE, BLUE, CARD, DUAL
    //MOD 3 chars pattern mode - AUT, MAN
    //RES resume loop
    //BRT 3 chars brightness value (0-255)
    //VOL 3 chars volume value (0-255)
    //NXT next pattern
    //PRE previous pattern
    //ARG 3 chars patternNum arg list (parsed by individual patterns)
    //APA audio pause
    //APR audio previous
    //ANX audio next

    debugUDP("CMD: "+command+"\n");
    if(command.length() < 3) {
        return -1;
    }

    std::string inputCommand = command.toUpperCase().c_str();
    std::string mainCommand = inputCommand.substr(0, 3);
    std::string subCommand = inputCommand.substr(3);

    if(mainCommand=="FRZ") {
        monitorTimer.stop();
        patternTimer.stop();
        updatePatternTimer.stop();
        player->pause();
        patternMode = "MAN";
        debugUDP("frozen\n");
        return 0;
    } else if(mainCommand=="PAU") {
        patternTimer.stop();
        patternMode = "MAN";
        debugUDP("paused\n");
        return 0;
    } else if(mainCommand=="CLR") {
        //may need to destroy some pointers here
        patternList.clear();
        colorList.clear();
        codeList.clear();
        argList.clear();
        speedList.clear();
        debugUDP("cleared\n");
        return 0;
    } else if(mainCommand=="PAT") {
        uint16_t patternNum = atoi(subCommand.substr(0,3).c_str());
        std::string code = subCommand.substr(3);
        if(patternNum>=codeList.size()) {
            codeList.resize(patternNum+1,"");
        }
        codeList.at(patternNum) = code;
        debugUDP("added pattern " + String(code.c_str())+"\n");
        return 0;
    } else if(mainCommand=="COL") {
        uint16_t patternNum = atoi(subCommand.substr(0,3).c_str());
        uint16_t colorNum = atoi(subCommand.substr(3,3).c_str());
        uint32_t colorInt = strtol(subCommand.substr(6,6).c_str(),NULL,16); //base 16 number corresponding to color value
        CRGB colorToAdd = CRGB(colorInt);
        std::vector<CRGB> newVector = {};
        //if the current pattern number is higher than the size of the colorlist, resize the colorlist
        if(patternNum>=colorList.size()) {
            colorList.resize(patternNum+1, newVector);
        }
        //if the current color number is higher than the size of the list of colors for this pattern, resize that list
        if(colorNum>=colorList[patternNum].size()) {
            colorList[patternNum].resize(colorNum+1,CRGB::Black);
        }
        colorList[patternNum].at(colorNum) = colorToAdd;
        return 0;
    } else if(mainCommand=="DUR") {
        uint32_t duration = atoi(subCommand.c_str());
        patternTimer.changePeriod(duration * 1000);
        return 0;
    } else if(mainCommand=="SPD") {
        uint16_t patternNum = atoi(subCommand.substr(0,3).c_str());
        uint16_t speed = atoi(subCommand.substr(3).c_str());
        std::vector<uint16_t>::iterator speedBegin = speedList.begin();
        speedList.insert(speedBegin+patternNum, speed);
        return 0;
    } else if(mainCommand=="AUD") {
        audioMode = String(subCommand.c_str());
        if(audioMode=="NONE") {
            player->pause();
            player->mute();
        } else if(audioMode=="CARD"||audioMode=="DUAL") {
            patternEngine* pat = patternList[patternIndex];
            song songToPlay = pat->getSong();
            if(songToPlay.title.length()>0) {
                cardPlayer.play(songToPlay);
            } else if(audioMode=="DUAL") {
                bluetoothPlayer.play();
            }
        } else if(audioMode=="BLUE") {
            bluetoothPlayer.play();
        }
        return 0;
    } else if(mainCommand=="MOD") {
        patternMode = String(subCommand.c_str());
        return 0;
    } else if(mainCommand=="RES") {
        patternMode = "AUT";
        //make sure if there isn't a color list for a pattern, a default (blank) is put in
        uint8_t minSize = codeList.size();
        std::vector<CRGB> newVector = {};
        colorList.resize(minSize, newVector);
        argList.resize(minSize,"");
        speedList.resize(minSize, 100);
        patternList = makePatternList(codeList, colorList, speedList, argList);
        monitorTimer.start();
        if(patternList.size()>1) {
            patternTimer.start();
        }
        patternLoop();
        updatePatternTimer.start();
        return 0;
    } else if(mainCommand=="BRT") {
        brightness = atoi(subCommand.substr().c_str());
        FastLED.setBrightness(brightness);
        return 0;
    } else if(mainCommand=="VOL") {
        volume = atoi(subCommand.substr().c_str());
        player->changeVolume(volume);
        return 0;
    } else if(mainCommand=="NXT") {
        //assumes this command is only available in manual mode
        patternIndex++;
        patternIndex%= patternList.size();
        patternLoop();
        return 0;
    } else if(mainCommand=="PRE") {
        //assumes this command is only available in manual mode
        patternIndex += patternList.size() - 1;
        patternIndex  = patternIndex%patternList.size();
        patternLoop();
        return 0;
    } else if(mainCommand=="ARG") {
        uint16_t patternNum = atoi(subCommand.substr(0,3).c_str());
        std::string arguments = subCommand.substr(3);
        //if the argument list isn't big enough to hold the argument at that pattern number, resize it
        if(patternNum>=argList.size()) {
            argList.resize(patternNum+1, "");
        }
        argList.at(patternNum) = arguments;
        return 0;
    } else if(mainCommand=="APA") {
        player->pause();
        return 0;
    } else if(mainCommand=="APR") {
        if(player->type=="bluetooth") {
            player->previous();
        }
        return 0;
    } else if(mainCommand=="ANX") {
        if(player->type=="bluetooth") {
            player->next();
        }
    } else if(mainCommand=="APL") {
        player->play();
        return 0;
    }
}

void connectionTest() {
    debugUDP("asking for status\n");
    Serial1.write("STATUS\r");
    Serial1.write("WRITE\r");
}

void monitorLoop() {
    //get the value of the volume potentiometer
    if(monitorVPot) {
        if(normalVPot) {
            uint16_t newValue = analogRead(VOLUME_POT);
            if(abs(newValue-vPotValue)>POT_DELTA) {
                volume = newValue/16;
                player->changeVolume(volume);
            }
            vPotValue = newValue;
        } else {
            vPotValue = analogRead(VOLUME_POT);
        }

    }
    //get the value of the brightness potentiometer
    if(monitorBPot) {
        if(normalBPot) {
            uint16_t newValue = analogRead(BRIGHTNESS_POT);
            if(abs(newValue-bPotValue)>POT_DELTA) {
                brightness = newValue/16;
                debugUDP("new brightness: "+String(brightness)+"\n");
                FastLED.setBrightness(brightness);
                FastLED.show();
            }
            bPotValue = newValue;
        } else {
            bPotValue = analogRead(BRIGHTNESS_POT);
        }
    }
    if(monitorSPot) {
        if(normalSPot) {
            sPotValue = analogRead(SWIPE_STRIPE);
            swipeStripeDetector.add(sPotValue);
            std::string eventType = swipeStripeDetector.detect();
            if(eventType=="decreasing") {
                //swipe right
                //go to next pattern
                if(patternMode=="MAN") {
                    patternIndex++;
                    patternIndex %= patternList.size();
                    patternLoop();
                }
                debugUDP("decreasing\n");
            } else if(eventType=="increasing") {
                //swipe left
                //go to previous pattern
                if(patternMode=="MAN") {
                    patternIndex += patternList.size() - 1;
                    patternIndex = patternIndex % patternList.size();
                    patternLoop();
                }
                debugUDP("increasing\n");
            } else if(eventType=="tap") {
                //change pattern mode
                if(patternMode=="AUT") {
                    patternMode = "MAN";
                    patternTimer.stop();
                } else if(patternMode=="MAN") {
                    patternMode = "AUT";
                    patternTimer.start();
                }
                debugUDP("tap\n");
            }
        } else {
            sPotValue = analogRead(SWIPE_STRIPE);
        }

    }
    if(monitorSerial) {
        if(player->type=="card") {
            while(Serial1.available()>0) {
                incomingInfo = Serial1.read();
                if(incomingInfo==126) {
                    //code for first byte of data
                    DFPlayerData.clear();
                }
                DFPlayerData.push_back(incomingInfo);
                if(incomingInfo == 239) {
                    //code for last byte of data
                    cardPlayer.sendSerial(DFPlayerData);
                }
            }
        } else if(player->type=="bluetooth") {
            //serial data is monitored in loop because of its extremely quick occurence and saved to the serialString variable
            //bluetooth serial data ends with a \r character
            std::size_t rIdx = serialString.find("\r",0);
            while(rIdx!=std::string::npos) {
                bluetoothPlayer.sendSerial(serialString.substr(0,rIdx+1));
                serialString = serialString.substr(rIdx+1);
                rIdx = serialString.find("\r",0);
            }

        }
    }
    //gets values from the spectrum analyzer
    //there are 7 bands of frequencies analyzed by it, cycle through each to get their values
    if(monitorSpectrum) {
        digitalWrite(SPEC_STROBE, LOW);
        delayMicroseconds(30);
        bands[bandIndex] = analogRead(SPEC_LEVEL);
        if(bandIndex==6) {
            //last band
           for(int i = 0; i < 7; i++) {
               finalBands[i] = bands[i];
           }
        }
        //cycle to next band for next iteration of monitor loop (set to 7 times faster than the desired rate of a pattern that uses it)
        bandIndex++;
        bandIndex%=7;
        digitalWrite(SPEC_STROBE, HIGH);
    }
}




void patternLoop() {

    updatePatternTimer.stop();
    //reset pattern specific variables
    normalBPot = true;
    normalVPot = true;
    normalSPot = true;
    monitorVPot = true;
    monitorBPot = true;
    monitorSPot = true;
    monitorSerial = true;
    monitorSpectrum = false;


    //reset things, increment index
    if(patternMode=="AUT") {
        //only increment in automatic mode
        patternIndex++;
        patternIndex%=patternList.size();
    }
    patternEngine* pat = patternList[patternIndex];
    pat->reset();
    pat->setup();
    monitorTimer.changePeriod(pat->monitorRate());
    if(player->type=="card") {
        player->pause();
    }

    //reset LED strip for new pattern
    FastLED.clear();
    FastLED.show();

    if(audioMode=="CARD") {
        if(player->type!="card") {
            setPlayer("card");
        }
        patternEngine* pat = patternList[patternIndex];
        song songToPlay = pat->getSong();
        if(songToPlay.title.length()>0) {
            //required 30ms delay between any data sent to DFPlayer
            delay(30);
            cardPlayer.play(songToPlay);
        } else {
            player->mute();
        }
    } else if(audioMode=="DUAL") {
        patternEngine* pat = patternList[patternIndex];
        song songToPlay = pat->getSong();
        //if there's a song corresponding to a pattern, play it
        if(songToPlay.title.length()>0) {
            setPlayer("card");
            if(patternList.size()>1|| (!player->state)) {
                cardPlayer.repeatPlay(songToPlay);
            }
        } else {
            //if connected (using bluetooth variable) (todo)
            if(player->type=="bluetooth") {
                if(!player->state) {
                    player->play();
                }
                //player->next();
               // player->play();
            } else {
                delay(20);
                setPlayer("bluetooth");
                //player->next();
                player->play();
            }
        }
    } else if(audioMode=="BLUE") {
        if(player->type!="bluetooth") {
            //player->pause();
            //delay(20);
            setPlayer("bluetooth");
            player->play();
        }
        // if(!player->state) {
        //     player->play();
        // }
    } else if(audioMode=="NONE") {
        if(player->type!="bluetooth") {
            debugUDP("setting to bluetooth because it isn't already\n");
            setPlayer("bluetooth");
        }
        if(player->state) {
            debugUDP("it wasn't muted so I'm doing that now\n");
            player->mute();
            Serial1.print("DISCOVERABLE ON\r");
            Serial1.print("WRITE\r");
        }

    }
    updatePatternTimer.start();
}

void updateLoop() {
    patternEngine* pat = patternList[patternIndex];
    pat->update();
    FastLED.show();
    uint16_t delay = pat->nextUpdate();
    updatePatternTimer.changePeriod(delay);
}

void setPlayer(std::string playerType) {
    if(playerType=="card") {
        digitalWrite(AUD_SELECT, LOW);
        player = &cardPlayer;
        player->type = "card";
    } else if(playerType=="bluetooth") {
        digitalWrite(AUD_SELECT, HIGH);
        player = &bluetoothPlayer;
        player->type = "bluetooth";
        //query status of bluetooth
        connectionTest();
    }
    player->state = false;
}

void receivedDefaults(const char *event, const char *data) {
    std::string dataString = std::string(data);
    debugUDP("received webhook data\n\n");
    debugUDP(String(dataString.c_str()));
    std::vector<std::string> codes = extraParse(dataString.substr(0, dataString.length()-1),"~");
    for(int i = 0; i < codes.size(); i++) {
        passData(String(codes[i].c_str()));
    }
}
