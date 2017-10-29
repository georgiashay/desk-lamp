
#pragma SPARK_NO_PREPROCESSOR

#include "structures.h"

#include <FastLED.h>
#include <string>
#include <vector>

FASTLED_USING_NAMESPACE

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
#define DEFAULT_SPEED 100

#define Start_Byte 0x7E
#define Version_Byte 0xFF
#define Command_Length 0x06
#define End_Byte 0xEF
#define Acknowledge 0x00

extern UDP udp;
extern unsigned int localPort;

void debugUDP(String message) {

    char buffer[255] = "";
    IPAddress remoteIP(192,168,1,235);
    int remotePort = 8888;
    message.toCharArray(buffer,sizeof(buffer));
    if (udp.sendPacket(buffer, sizeof(buffer), remoteIP, remotePort) < 0) {
        Particle.publish("Error");
     }
}

song::song(std::string name, uint8_t fol, uint16_t num) {
    title = name;
    number = num;
    folder = fol;
}
song::song() {
    title = "";
    number = 0;
    folder = 0;
}

int digitalPotWrite(int value) {
  digitalWrite(CS, LOW);
  SPI.transfer(0x13);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}

//from http://educ8s.tv/arduino-mp3-player/
void execute_CMD(byte CMD, byte Par1, byte Par2) { // Excecute the command and parameters
    // Calculate the checksum (2 bytes)
    int16_t checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
    //first byte of the checksum (shifted 8 bits)
    byte checksum1 = checksum >> 8;
    //second byte of the checksum (last 8 bits only)
    byte checksum2 = checksum & 0xFF;
    // Build the command line
    byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge, Par1, Par2, checksum1, checksum2, End_Byte};
    //byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge, Par1, Par2, End_Byte};

    //Send the command line to the module
    for (byte k=0; k<10; k++) {
        Serial1.write( Command_line[k]);
    }
}

//scorer functions
//give each type of event a score - the more points, the less it matches
int SwipeDetector::analyzeIncrease(std::vector<uint16_t> e) {
    int score = 0;
    for(int i = 1; i < e.size(); i++) {
        //if it doesn't go up beyond a threshold, increase the score
        if(e[i]-e[i-1]<=DELTA_LOW&&e[i-1]>NOISE) {
            score++;
        }
    }
    return score;
}
int SwipeDetector::analyzeDecrease(std::vector<uint16_t> e) {
    int score = 0;
    for(int i = 1; i < e.size(); i++) {
        //if it doesn't go down beyond a threshold, increase the score
        if(e[i-1]-e[i]<=DELTA_LOW&&e[i]>NOISE) {
            score++;
        }
    }
    return score;
}
int SwipeDetector::analyzeTap(std::vector<uint16_t> e) {
    int score = 0;
    float average = 0;
    int size = 0;
    //calculate the average value
    for(int i = 0; i < e.size(); i++) {
        if(e[i]>NOISE) {
            average += e[i];
            size++;
        }
    }
    average /= (float)size;
    for(int i = 0; i < e.size(); i++) {
        if(e[i]>NOISE) {
            //if a value deviates too much from the average, increase the score
            if(abs(e[i]-average)>DELTA_HIGH) {
                score++;
            }
        } else {
            //if a value is a 0, increase the score by 2
            score+=2;
        }

    }
    return score;
}
SwipeDetector::SwipeDetector() {
    NOISE = 50;
    DELTA_LOW = 75;
    DELTA_HIGH = 100;
    ZEROS_DONE = 10;
}
SwipeDetector::SwipeDetector(uint16_t ns, uint16_t dl, uint16_t dh, uint16_t zd) {
    NOISE = ns;
    DELTA_LOW = dl;
    DELTA_HIGH = dh;
    ZEROS_DONE = zd;
}


void SwipeDetector::add(uint16_t value) {
    //if the value is just noise
    if(value < NOISE) {
        //if the event has started and has values in it, add the zero
        if(eventStarted && event.size()>0) {
            event.push_back(value);
        }
        //increment the number of zeros
        zeros++;
    //if it's a real value and the event has already started
    } else if(eventStarted) {
        //the event isn't done yet
        eventDone = false;
        //there is no longer a run of "zero values"
        zeros = 0;
        //add the value
        event.push_back(value);
    }
    //if there is a long enough run of zeros
    if(zeros>=ZEROS_DONE) {
        //if the event hasn't started
        if(event.size()==0) {
            //it's been long enough since the last event
            eventStarted = true;
            zeros = 0;
        //if the event has started
        } else {
            //the event is over now
            eventDone = true;
            eventStarted = false;
            //delete the last run of zeros from the actual event
            event.erase(event.end() - ZEROS_DONE, event.end());
            zeros = 0;
        }
    }
}

std::string SwipeDetector::detect() {
    if(eventDone) {
        int incScore = analyzeIncrease(event);
        int decScore = analyzeDecrease(event);
        int tapScore = analyzeTap(event);

        int scores[3] = {incScore, decScore, tapScore};
        int minIndex = 0;

        for(int i = 1; i < 3; i++) {
            if(scores[i]<scores[minIndex]) {
                minIndex = i;
            }
        }

        event.clear();
        eventStarted = false;
        eventDone = false;

        switch(minIndex) {
            case 0:
                return "increasing";
                break;
            case 1:
                return "decreasing";
                break;
            case 2:
                return "tap";
                break;
        }

    } else {
        return "";
    }
}

void MusicPlayer::unmute() {
    digitalWrite(MUTE_MUX, LOW);
    digitalWrite(SPEAKER_ON, HIGH);
}

void MusicPlayer::mute() {
    digitalWrite(MUTE_MUX, HIGH);
    digitalWrite(SPEAKER_ON, LOW);
}

void MusicPlayer::changeVolume(uint8_t level) {
    SPI.begin(SPI_MODE_MASTER,CS);
    digitalPotWrite(level);
    volume = level;
    SPI.end();
    pinMode(BRIGHTNESS_POT, INPUT);
}

void MusicPlayer::toggle() {
    if(state) {
        pause();
    } else {
        play();
    }
}

void CardMusicPlayer::sendSerial(std::vector<uint8_t> data) {
    for(int i = 0; i < data.size(); i++) {
        debugUDP(String(data[i])+" ");
    }
}
void CardMusicPlayer::play() {
    state = true;
    unmute();
    //play
    execute_CMD(0x0D,0,0);
    state = true;
}

void CardMusicPlayer::play(song s) {
    state = true;
    execute_CMD(0x0F,s.folder, s.number);
    delay(30);
    execute_CMD(0x0D,0,0);
    unmute();

}
void CardMusicPlayer::repeatPlay(song s) {
    state = true;
    execute_CMD(0x0F, s.folder, s.number);
    delay(30);
    execute_CMD(0x19,0,0);
    delay(30);
    execute_CMD(0x0D, 0, 0);
    unmute();
}


void CardMusicPlayer::pause() {
    //pause command
    execute_CMD(0x0E,0,0);
    state = false;
}

void CardMusicPlayer::next() {
    //next command
    execute_CMD(0x01,0,0);
}
void CardMusicPlayer::previous() {
    //previous command
    execute_CMD(0x02,0,0);
}
void CardMusicPlayer::startOver() {
    //fix this (todo)
    //execute_CMD(0x4B,0,0); //query for current sound
    //uint16_t song = Serial1.read(); //ths needs updated to get just the one byte
    //play(song);
}

void BluetoothMusicPlayer::sendSerial(std::string data) {
    //debugUDP("Received: "+String(data.c_str())+"\n");
    if(data.find("STATE")!=std::string::npos) {
        if(data.find("CONNECTED")==std::string::npos) {
            mute();
        }
        debugUDP("state\n");
    }
    if(data.find("PAIR_OK")!=std::string::npos||data.find("OPEN_OK")!=std::string::npos) {
        debugUDP("bluetooth says the pair is ok\n");
        unmute();
    }
    if(data.find("LINK_LOSS")!=std::string::npos||data.find("CLOSE_OK")!=std::string::npos) {
        debugUDP("it went away\n");
        mute();
    }
}
void BluetoothMusicPlayer::play() {
    unmute();
    state = true;
    Serial1.print("MUSIC PLAY\r");
}
void BluetoothMusicPlayer::play(song s) {
    state = true;
    unmute();
    play();
}

void BluetoothMusicPlayer::pause() {
    state = false;
    Serial1.print("MUSIC PAUSE\r");
    mute();
}

void BluetoothMusicPlayer::next() {
    Serial1.print("MUSIC FORWARD\r");
}

void BluetoothMusicPlayer::previous() {
    Serial1.print("MUSIC BACKWARD\r");
    delay(10);
    Serial1.print("MUSIC BACKWARD\r");
}

void BluetoothMusicPlayer::startOver() {
    Serial1.print("MUSIC BACKWARD\r");
}

void BluetoothMusicPlayer::playNote(note n) {
    std::string create = "TONE";
    if(n.timbre!=NULL) {
        create += "TI " + n.timbre;
    }
    create += "N " + n.value + "L " + itoa(n.length,NULL,10) + "\r";
    String create2 = create.c_str();
    Serial1.print(create2);
}
