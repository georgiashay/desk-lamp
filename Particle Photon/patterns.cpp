#pragma SPARK_NO_PREPROCESSOR

#include "patterns.h"
#include <string>
#include <vector>
#include <algorithm>

extern MusicPlayer* player;
extern CardMusicPlayer cardPlayer;
extern BluetoothMusicPlayer bluetoothPlayer;
extern String audioMode;

FASTLED_USING_NAMESPACE


song blankSong("", 0, 0);


void copyToAll(uint8_t i, CRGB color) {
    leds1[i] = color;
    leds2[i] = color;
    leds3[i] = color;
    leds4[i] = color;
}

void copyToOne(uint8_t spoke, uint8_t i, CRGB color) {
    if(spoke==0) {
        leds1[i] = color;
    } else if(spoke==1) {
        leds2[i] = color;
    } else if(spoke==2) {
        leds3[i] = color;
    } else if(spoke==3) {
        leds4[i] = color;
    }

}

void fillRange(uint8_t start, uint8_t end, CRGB color) {
    for(int i = start; i < end; i++) {
        copyToAll(i, color);
    }
}

void fillRangeOne(uint8_t spoke, uint8_t start, uint8_t end, CRGB color) {
    for(int i = start; i < end; i++) {
        copyToOne(spoke, i, color);
    }
}

CRGB rainbowColor(uint8_t i) {
    if(i < 85) {
        return CRGB(i*3, 255-i*3,0);
    } else if(i < 170) {
        i-=85;
        return CRGB(255-i*3, 0, i*3);
    } else {
        i -=170;
        return CRGB(0, i*3, 255-i*3);
    }
}

std::vector<std::string> extraParse(std::string extratext,std::string delimiter) {
    std::vector<std::string> retarray;
    size_t current;
    size_t next = -1;
    do {
        current = next + 1;
        next = extratext.find_first_of(delimiter,current);
        if(next!=std::string::npos) {retarray.push_back(extratext.substr(current,next-current));} else {retarray.push_back(extratext.substr(current,extratext.length()-current)); }
    } while (next != std::string::npos);

    return retarray;
}



patternEngine::patternEngine() {
    colors = {};
    arguments = "";
    speed = DEFAULT_SPEED;
}
patternEngine::patternEngine(std::vector<CRGB> colorList, uint16_t spd, std::string args) {
    colors = colorList;
    arguments = args;
    speed = spd;
}
uint16_t patternEngine::nextUpdate() {
    return speed;
}
uint16_t patternEngine::monitorRate() {
    return MONITOR_RATE;
}
song patternEngine::getSong() {
    return blankSong;
}
void patternEngine::reset() {

}
void patternEngine::setup() {

}

cycleColor::cycleColor(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};

void cycleColor::update() {
    for(int i = 0; i < STRIP_LENGTH; i++) {
        copyToAll(i, colors[index]);
    }
    index++;
    index%=colors.size();
}
void cycleColor::reset() {
    index = 0;
}



stripe::stripe(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {
    stripes = colorList.size();
}
void stripe::update() {
    for(int i = 0; i < stripes; i++) {
        int bottomIdx = (i * STRIP_LENGTH)/stripes + index;
        bottomIdx = bottomIdx % STRIP_LENGTH;
        int topIdx = (i+1) * STRIP_LENGTH/stripes + index;
        topIdx = topIdx % STRIP_LENGTH;
        if(topIdx> bottomIdx) {
            for(int j = bottomIdx; j < topIdx; j++) {
                copyToAll(j, colors[i]);
            }
        } else {
            for(int j = 0; j< topIdx; j++) {
                copyToAll(j, colors[i]);
            }
            for(int j = bottomIdx; j < STRIP_LENGTH; j++) {
                copyToAll(j, colors[i]);
            }
        }
    }
    index++;
    index%=STRIP_LENGTH;
}
void stripe::reset() {
    index = 0;
}




void rain::solid(CRGB color) {
    for(int i = 0; i < STRIP_LENGTH; i++) {
        copyToAll(i, color);
    }
}

rain::rain(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {
    if(colorList.size()>0) {
        dropColor = colorList[0];
    }
    rainSong.title = "rain",
    rainSong.number = 1;
    rainSong.folder = 1;
}
void rain::update() {
    fadeToBlackBy(leds1, STRIP_LENGTH, 125);
    fadeToBlackBy(leds2, STRIP_LENGTH, 125);
    fadeToBlackBy(leds3, STRIP_LENGTH, 125);
    fadeToBlackBy(leds4, STRIP_LENGTH, 125);
    for(int i = 0; i < 4; i++) {
        for(int d = 0; d < drops[i].size(); d++) {
            drops[i][d].velocity += gravity;
            drops[i][d].lastPos = drops[i][d].position;
            drops[i][d].position -= drops[i][d].velocity;
            if(drops[i][d].lastPos>0) {
                int lowerbound = max(int(drops[i][d].position),0);
                int upperbound = drops[i][d].lastPos;
                if(upperbound==lowerbound) {
                    upperbound++;
                }
                for(int j = lowerbound; j < upperbound; j++) {
                    copyToOne(i, j, dropColor);
                }
            } else {
                drops[i].erase(drops[i].begin()+d);
            }
        }
        int randNum = rand() % 8;
        if(randNum==0) {
            Drop newDrop;
            newDrop.position = STRIP_LENGTH;
            newDrop.velocity = 0;
            newDrop.lastPos = STRIP_LENGTH;
            drops[i].push_back(newDrop);
        }
    }

}
song rain::getSong() {
    return rainSong;
}
void rain::reset() {
    for(int i = 0; i < 4;i++) {
        drops[i].clear();
    }
}





rainbow::rainbow(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void rainbow::update() {
    for(int i = 0; i < STRIP_LENGTH; i++) {
        CRGB currentColor = rainbowColor((i+index)%256);
        copyToAll(i, currentColor);
    }
    index++;
    index%=256;
}
void rainbow::reset() {
     index = 0;
}


singlecolor::singlecolor(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void singlecolor::update() {
    for(int i = 0; i < STRIP_LENGTH; i++) {
        copyToAll(i, colors[0]);
    }
}



quadcolor::quadcolor(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void quadcolor::update() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < STRIP_LENGTH; j++) {
            copyToOne(i, j, colors[i]);
        }
    }
}


storm::storm(std::vector<CRGB> colorList, uint16_t spd, std::string args) : rain(colorList, spd, args) {
    std::vector<std::string> argList = extraParse(args,",");
    stormFrequency = atoi(argList[0].c_str());
    maxFlashes = atoi(argList[1].c_str());
    stormsong.title = "storm";
    stormsong.number = 1;
    stormsong.folder = 1;
    song thunder1("thunder1",2,1);
    song thunder2("thunder2",2,2);
    song thunder3("thunder3",2,3);
    thunders.push_back(thunder1);
    thunders.push_back(thunder2);
    thunders.push_back(thunder3);
}
void storm::update() {
    if(!flashing) {
        rain::update();
        if(random8(stormFrequency)==0) {
            uint8_t randomThunder = random8(2);
            if(audioMode=="CARD"||audioMode=="DUAL") {
                cardPlayer.play(thunders[randomThunder]);
            }
            flashing = true;
            flashStart = random8(STRIP_LENGTH);
            flashEnd = random8(flashStart, STRIP_LENGTH);
            numFlashes = random8(3, maxFlashes);
            delays.push_back(150);
            lengths.push_back(random8(4,10));
            for(int i = 1; i < numFlashes; i++) {
                lengths.push_back(random8(4,10));
                delays.push_back(50+random8(100));
            }
            flashCounter = 0;
            fillRange(flashStart, flashEnd, CRGB::White);
            onFlash = true;
        }

    } else {
        if(onFlash) {
            fillRange(flashStart, flashEnd, CRGB::Black);
            rain::update(); //need more sophisticated if want it to be exact
        } else {
            flashCounter++;
            if(flashCounter<lengths.size()) {
                fillRange(flashStart, flashEnd, CRGB::White);
            } else {
                lengths.clear();
                delays.clear();
                flashing = false;
            }
        }
        onFlash = !onFlash;
    }

}
uint16_t storm::nextUpdate() {
    if(!flashing) {
        return speed;
    } else {
        if(onFlash) {
            return lengths[flashCounter];
        } else {
            return delays[flashCounter];
        }
    }
}
song storm::getSong() {
   return stormsong;
}
void storm::reset() {
    rain::reset();
    flashing = false;
    lengths.clear();
    delays.clear();
}



rotate::rotate(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void rotate::update() {
    for(int i = 0; i < 4; i++) {
        CRGB color1 = colors[(i+index)%colors.size()];
        CRGB color2 = colors[(i+index+1)%colors.size()];
        uint8_t blendNum = blendIndex * 8;
        CRGB blendColor = blend(color1, color2, blendNum);
        fillRangeOne(i, 0, STRIP_LENGTH, blendColor);

        blendIndex++;
        blendIndex%=32;
        if(blendIndex==0) {
            index++;
            index%=4;
        }
    }
}
void rotate::reset() {
    blendIndex = 0;
    index = 0;
}



spectrum::spectrum(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void spectrum::setup() {
    monitorSpectrum = true;
}
uint16_t spectrum::monitorRate() {
    return speed/7;
}
void spectrum::update() {
    fillRange(0, STRIP_LENGTH, CRGB::Black);
    uint16_t totalLevel = 0;
    for(int i = 0; i < 7; i++) {
        uint16_t bandLevel = finalBands[i];
        for(int j = std::min(totalLevel/600,STRIP_LENGTH); j < std::min((totalLevel+bandLevel)/600,STRIP_LENGTH); j++) {
            copyToAll(j, bandColors[i]);
        }
        totalLevel += bandLevel;
    }
    if(totalLevel>highest) {
        highest = totalLevel;
        //debugUDP("HIGH: "+String(highest)+"\n");
    } else {
        highest -= 400;
        copyToAll(highest/400, CRGB::White);
    }
}
void spectrum::reset() {
    highest = 0;
    level = 0;
}



hue::hue(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void hue::setup() {
    normalVPot = false;
}
void hue::update() {
    CRGB col = rainbowColor(vPotValue/16);
    fillRange(0, STRIP_LENGTH, col);
}


manualRotate::manualRotate(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void manualRotate::setup() {
    normalVPot = false;
}
void manualRotate::update() {
    int index = vPotValue/1024;
    uint8_t inBetween = (vPotValue%1024)/4;
    for(int i = 0; i < 4; i++) {
        CRGB color1 = colors[(i+index)%colors.size()];
        CRGB color2 = colors[(i+index+1)%colors.size()];
        CRGB blendColor = blend(color1, color2, inBetween);
        fillRangeOne(i, 0, STRIP_LENGTH, blendColor);
    }

}



//currently not working, possibly due to escaping strange ascii characters
paint::paint(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {
    //debugUDP("arguments: "+String(args.c_str())+"\n");
    for(int i = STRIP_LENGTH/2; i < args.length(); i+=3) {
        uint32_t colorNum = strtol(args.substr(i,3).c_str(),NULL,256);
        CRGB palColor = CRGB(colorNum);
        palette[(i-STRIP_LENGTH/2)/3] = palColor;
    }
    for(int i = 0; i < STRIP_LENGTH/2; i++) {
        int pairNum = atoi(args.substr(i,1).c_str()) - 33;
        int firstNum = pairNum % 9;
        int secondNum = pairNum/9;
        pixels[i*2] = palette[firstNum];
        pixels[i*2+1] = palette[secondNum];
    }
}
void paint::update() {};

void paint::setup() {
    for(int i = 0; i < STRIP_LENGTH; i++) {
        copyToAll(i, pixels[i]);
    }
}


firework::firework(std::vector<CRGB> colorList, uint16_t spd, std::string args) : patternEngine(colorList, spd, args) {};
void firework::update() {
    fadeToBlackBy(leds1, STRIP_LENGTH, 200);
    fadeToBlackBy(leds2, STRIP_LENGTH, 200);
    fadeToBlackBy(leds3, STRIP_LENGTH, 200);
    fadeToBlackBy(leds4, STRIP_LENGTH, 200);
    if(rising) {
        dotIndex+=3;
        copyToAll(dotIndex, colors[colorIndex]);
        if(dotIndex>=top) {
            rising = false;
            for(int i = 0; i < numEmbers; i++) {
                particle ember;
                ember.lastPos = top;
                ember.position = top;
                ember.velocity = float(rand()%6-3)/float(3);
                ember.strip =  rand()%3;
                embers.push_back(ember);
            }
        }
    } else {
        int numInBounds = 0;
        for(int i = 0; i < embers.size(); i++) {
            embers[i].velocity += gravity;
            embers[i].lastPos = embers[i].position;
            embers[i].position -= embers[i].velocity;
            if(embers[i].lastPos>0) {numInBounds++;}
            if(embers[i].lastPos>0 && embers[i].position < STRIP_LENGTH) {
                int lowerbound = max(int(embers[i].position),0);
                int upperbound = min(int(embers[i].lastPos),STRIP_LENGTH-1);
                if(upperbound == lowerbound) {upperbound++;} //can't i just do <=
                for (int j=lowerbound;j<upperbound;j++) {
                    copyToOne(embers[i].strip, j, colors[colorIndex]);
                }
            }
        }
        if(numInBounds==0) {
            rising = true;
            dotIndex = 0;
            embers.clear();
            colorIndex++;
            colorIndex %= colors.size();
        }
    }
}
void firework::reset() {
    rising = true;
    dotIndex = 0;
    embers.clear();
    colorIndex = 0;
}


std::vector<patternEngine*> makePatternList(std::vector<std::string> codes, std::vector<std::vector<CRGB>> colors, std::vector<uint16_t> speeds, std::vector<std::string> args) {
    std::vector<patternEngine*> list;
    for(int i = 0; i < codes.size(); i++) {
        debugUDP("code " + String(i));
        std::string code = codes[i];
        debugUDP(": " + String(code.c_str())+"\n");
        if(code=="CC") {
            list.push_back(new cycleColor(colors[i], speeds[i], args[i]));
        } else if(code=="ST") {
            list.push_back(new stripe(colors[i], speeds[i], args[i]));
        } else if(code=="RN") {
            list.push_back(new rain(colors[i], speeds[i], args[i]));
        } else if(code=="RB") {
            list.push_back(new rainbow(colors[i], speeds[i], args[i]));
        } else if(code=="SC") {
            list.push_back(new singlecolor(colors[i], speeds[i], args[i]));
        } else if(code=="QC") {
            list.push_back(new quadcolor(colors[i], speeds[i], args[i]));
        } else if(code=="SM") {
            list.push_back(new storm(colors[i], speeds[i], args[i]));
        } else if(code=="RO") {
            list.push_back(new rotate(colors[i], speeds[i], args[i]));
        } else if(code=="SP") {
            list.push_back(new spectrum(colors[i], speeds[i], args[i]));
        } else if(code=="HU") {
            list.push_back(new hue(colors[i], speeds[i], args[i]));
        } else if(code=="MR") {
            list.push_back(new manualRotate(colors[i], speeds[i], args[i]));
        } else if(code=="FI") {
            list.push_back(new firework(colors[i],speeds[i],args[i]));
        }
        debugUDP("added "+String(code.c_str())+"\n");
    }
    return list;
}
