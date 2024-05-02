#ifndef TAPZONE_H
#define TAPZONE_H

// From the MakeabilityLab Arduino Library:
// Jon E. Froehlich
// @jonfroehlich
// - http://makeabilitylab.io
// - https://github.com/makeabilitylab/arduino/blob/master/MakeabilityLab_Arduino_Library/src/Shape.hpp#L231
#include <Shape.hpp>

class TapZone : public Rectangle {
protected:
    bool _ball1Visible;
    bool _ball2Visible;
    bool _ball3Visible;

public:
    TapZone(int x, int y, int width, int height);
    bool isBall1Visible();
    bool isBall2Visible();
    bool isBall3Visible();
    void setBall1Visible(bool visible);
    void setBall2Visible(bool visible);
    void setBall3Visible(bool visible);
    void setBallVisible(bool visible, int ballIndex);
    bool isBallVisible(int ballIndex);
};

#endif