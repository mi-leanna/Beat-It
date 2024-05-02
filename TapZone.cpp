#include "TapZone.hpp"

TapZone::TapZone(int x, int y, int width, int height)
    : Rectangle(x, y, width, height) {
    // Initialize visibility flags
    _ball1Visible = true;
    _ball2Visible = true;
    _ball3Visible = true;
}

bool TapZone::isBall1Visible() {
    return _ball1Visible;
}

bool TapZone::isBall2Visible() {
    return _ball2Visible;
}

bool TapZone::isBall3Visible() {
    return _ball3Visible;
}

void TapZone::setBall1Visible(bool visible) {
    _ball1Visible = visible;
}

void TapZone::setBall2Visible(bool visible) {
    _ball2Visible = visible;
}

void TapZone::setBall3Visible(bool visible) {
    _ball3Visible = visible;
}

void TapZone::setBallVisible(bool visible, int ballIndex) {
    // Determine which ball to set visibility for based on ball index
    switch (ballIndex) {
    case 4:
        _ball1Visible = visible;
        break;
    case 6:
        _ball2Visible = visible;
        break;
    case 5:
        _ball3Visible = visible;
        break;
        // Add more cases if necessary for additional balls
    default:
        // Handle invalid ball index
        Serial.println("Invalid ball index");
        break;
    }
}

bool TapZone::isBallVisible(int ballIndex) {
    switch (ballIndex) {
    case 4:
        return _ball1Visible;
    case 6:
        return _ball2Visible;
    case 5:
        return _ball3Visible;
    default:
        Serial.println("Invalid ball index");
        return false;
    }
}
