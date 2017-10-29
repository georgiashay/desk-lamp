#ifndef PATTERNS_H
#define PATTERNS_H

#pragma SPARK_NO_PREPROCESSOR

#include <FastLED.h>
#include <string>
#include <vector>
#include "structures.h"

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

#define MONITOR_RATE 100

extern CRGB leds1[STRIP_LENGTH];
extern CRGB leds2[STRIP_LENGTH];
extern CRGB leds3[STRIP_LENGTH];
extern CRGB leds4[STRIP_LENGTH];

extern bool normalVPot;
extern bool normalBPot;
extern bool normalSPot;

extern bool monitorVPot;
extern bool monitorBPot;
extern bool monitorSPot;
extern bool monitorSerial;
extern bool monitorSpectrum;

extern uint16_t finalBands[7];

extern uint16_t vPotValue;
extern uint16_t sPotValue;
extern uint16_t bPotValue;

void copyToAll(uint8_t i, CRGB color);

void copyToOne(uint8_t spoke, uint8_t i, CRGB color);

void fillRange(uint8_t start, uint8_t end, CRGB color);

void fillRangeOne(uint8_t spoke, uint8_t start, uint8_t end, CRGB color);

CRGB rainbowColor(uint8_t i);

std::vector<std::string> extraParse(std::string extratext,std::string delimiter);

class patternEngine {
    protected:
        std::vector<CRGB> colors;
        std::string arguments;
        uint16_t speed;
    public:
        patternEngine();
        patternEngine(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        virtual uint16_t nextUpdate();
        virtual uint16_t monitorRate();
        virtual void update() = 0;
        virtual song getSong();
        virtual void reset();
        virtual void setup();
};

class cycleColor : public patternEngine {
    private:
        uint8_t index = 0;
    public:
        cycleColor(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        void reset();

};

class stripe : public patternEngine {
    private:
        uint8_t index = 0;
        uint8_t stripes;
    public:
        stripe(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        void reset();
};

class rain : public patternEngine {
    private:
        struct Drop {
            float lastPos;
            float position;
            float velocity;
        };
        CRGB dropColor = CRGB::Blue;
        std::vector<Drop> drops[4] = {{}, {}, {}, {}};
        float gravity = 0.1;
        song rainSong;
        void solid(CRGB color);
    public:
        rain(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        song getSong();
        void reset();
};

class rainbow : public patternEngine {
    private:
        uint8_t index = 0;
    public:
        rainbow(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        void reset();
};

class singlecolor : public patternEngine {
    public:
        singlecolor(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
};

class quadcolor : public patternEngine {
    public:
        quadcolor(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
};

class storm : public rain {
    private:
        uint8_t stormFrequency;
        bool flashing = false;
        uint8_t flashStart;
        uint8_t flashEnd;
        uint8_t numFlashes;
        uint8_t maxFlashes;
       // std::vector<int> brightness;
        std::vector<uint8_t> lengths;
        std::vector<uint8_t> delays;
        uint8_t flashCounter;
        uint8_t rainUpdates = 0;
        //uint8_t cumulativeDelays = 0;
        bool onFlash = false;
        song stormsong;
        std::vector<song> thunders;
    public:
        storm(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        uint16_t nextUpdate();
        song getSong();
        void reset();
};

class rotate : public patternEngine {
    private:
        uint8_t index = 0;
        uint8_t blendIndex = 0;
    public:
        rotate(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        void reset();
};

class spectrum : public patternEngine {
    private:
        uint8_t highest = 0;
        uint16_t level = 0;
        std::vector<CRGB> bandColors = {0x009051, 0x0432FF, 0x9437FF, 0xFF85FF, 0xED7D31, 0xFFC000, 0xFFFF00};
    public:
        spectrum(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void setup();
        uint16_t monitorRate();
        void update();
        void reset();
};

class hue : public patternEngine {
    public:
        hue(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void setup();
        void update();
};

class manualRotate : public patternEngine {
    private:
    public:
        manualRotate(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void setup();
        void update();
};

class paint : public patternEngine {
    private:
        CRGB pixels[STRIP_LENGTH];
        CRGB palette[9] = {CRGB::White, CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::Green, CRGB::Cyan, CRGB::Blue, CRGB::Magenta, CRGB::Orange};
    public:
        paint(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        void setup();
};

class firework : public patternEngine {
    private:
        struct particle {
            float lastPos;
            float position;
            float velocity;
            int strip;
        };
        bool rising = true;
        uint8_t dotIndex = 0;
        std::vector<particle> embers;
        uint8_t top = 45;
        uint8_t colorIndex = 0;
        uint8_t numEmbers = 22;
        float gravity = 0.2;
    public:
        firework(std::vector<CRGB> colorList, uint16_t spd, std::string args);
        void update();
        void reset();

};

std::vector<patternEngine*> makePatternList(std::vector<std::string> codes, std::vector<std::vector<CRGB>> colors, std::vector<uint16_t> speeds, std::vector<std::string> args);


#endif
