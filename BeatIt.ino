#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "TapZone.hpp"

// Acknowledgments:
// A lot of the following declarations/initializations below
// are inspired and adapted from:
// Jon E. Froehlich
// @jonfroehlich
// http://makeabilitylab.io
// - Shape.hpp by Jon was also utilized to create a Ball object, and to
//   inherit fields of the Rectangle class for TapZone

// OLED Display Setup
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = 4;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, & Wire, OLED_RESET);

// Enumeration for game states
enum GameState {
  NEW_GAME,
  PLAYING,
  GAME_OVER,
};

// Current game state
GameState gameState = NEW_GAME;

// Constants
constexpr int IGNORE_INPUT_AFTER_GAME_OVER_MS = 500;
const char STR_PRESS_UP_TO_PLAY[] = "Press Up to Play";

// Pin Constants
constexpr int BALL1_INPUT_PIN = 4;
constexpr int BALL2_INPUT_PIN = 6;
constexpr int BALL3_INPUT_PIN = 5;
constexpr int VIBRATION_PIN = 8;
constexpr int SPEAKER_INPUT_PIN = 11;

// LED Pin Constants
constexpr int ledPinA0 = A0; // Define the LED pin connected to A0
constexpr int ledPinA1 = A1; // Define the LED pin connected to A1
constexpr int ledPinA2 = A2; // Define the LED pin connected to A2

// LED State Variables
bool ledOffA0 = true; // Variable to track if LED connected to A0 is off
bool ledOffA1 = true; // Variable to track if LED connected to A1 is off
bool ledOffA2 = true; // Variable to track if LED connected to A2 is off

// Global Variables
unsigned long gameOverTimestamp = 0;
unsigned long startTime = 0; // Global variable to store the start time

// Variables
int highScoreCurSession = 0;
int points = 0;
int allTimeScore = 0;
int allTimeScoreEEPROMIdx = 0;

// Delay Constant
constexpr int DELAY_LOOP_MS = 5;

// String Constants
const char STR_LOADSCREEN_CREATOR[] = "Made by Leanna :)";
const char STR_LOADSCREEN_APP_NAME_LINE1[] = "Beat";
const char STR_LOADSCREEN_APP_NAME_LINE2[] = "It!";
const char STR_GAME_OVER[] = "Game Over!";
constexpr int LOAD_SCREEN_SHOW_MS = 3000;
bool buttonPressed = false;

// Ball objects
Ball ball1(32, 0, 6);
Ball ball2(64, 0, 6);
Ball ball3(96, 0, 6);

// TapZone object
TapZone tapZone(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15);

// FPS variables
float fps = 0;
unsigned long frameCount = 0;
unsigned long fpsStartTimeStamp = 0;

// Other Variables
int tempo = 120;
int currentNoteIndex = 0;
int minFreq = 400;
int maxFreq = 750;
bool loserTonePlayed = false;

// Song Length Constant
constexpr int songLength = 88;

// Define the zelda lost woods melody frequencies
// Adapted from RandyDeng on GitHub
// - https://github.com/RandyDeng/Arduino-PiezoBuzzer
int notes[] = {698,880,988,698,880,988,698,880,988,1319,1175,988,1047,988,784,659,587,659,784,659,
               698,880,988,698,880,988,698,880,988,1319,1175,988,1047,1319,988,784,988,784,587,659,
               587,659,698,784,880,988,1047,988,659,698,784,880,988,1047,1175,1319,1397,1568,
               587,659,698,784,880,988,1047,988,659,
               784,698,880,784,988,880,1047,988,1175,1047,1319,1175,1397,1319,1319,1397,1175,1319,
               0,0,0
              };

////////////////////// Function Declarations ////////////////////////////////

// This function resets the position and speed of a given ball object.
void resetBall(Ball & ball);

// This function handles ball movement, collision detection, and score 
// updates based on the provided input pin and visibility flag.
// - Utilized Google Gemini to help debug if the user pressed the ball
//   outside of the TapZone. Also used to help debug if the user missed 
//   the beat
void handleBall(Ball & ball, int inputPin, bool ballVisible);

// This function displays the "New Game" or "Game Over" screen, handles 
// high score updates, and checks for user input to start a new game.
// - Inspired and adapted from:
//   Jon E. Froehlich
//   @jonfroehlich
//   https://github.com/makeabilitylab/arduino/blob/master/OLED/FlappyBird/FlappyBird.ino
void nonGamePlayLoop();

// This function draws the tap zone and handles ball interactions 
//within the zone for each visible ball.
// - Inspired and adapted from:
//   Jon E. Froehlich
//   @jonfroehlich
//   https://github.com/makeabilitylab/arduino/blob/master/OLED/FlappyBird/FlappyBird.ino
void gamePlayLoop();

// This function resets the game entities (balls, score, 
// visibility flags) to their initial state.
// - Inspired and adapted from:
//   Jon E. Froehlich
//   @jonfroehlich
//   https://github.com/makeabilitylab/arduino/blob/master/OLED/FlappyBird/FlappyBird.ino
void initializeGameEntities();

// This function initializes the OLED display and displays the startup screen.
// - Inspired and adapted from:
//   Jon E. Froehlich
//   @jonfroehlich
// https://github.com/makeabilitylab/arduino/blob/master/OLED/BallBounceObjectOriented/BallBounceObjectOriented.ino
void initializeOledAndShowStartupScreen();

// This function plays the next note in the melody sequence.
void playNextNote();

// This function resets the game entities (balls, score, 
// visibility flags) to their initial state.
// - Inspired and adapted from:
//   Jon E. Froehlich
//   @jonfroehlich
//   https://github.com/makeabilitylab/arduino/blob/master/OLED/FlappyBird/FlappyBird.ino
// This function calculates the current frame rate.
void calcFrameRate();

// This function draws the status bar displaying the current score.
// - Inspired and adapted from:
//   Jon E. Froehlich
//   @jonfroehlich
//   https://github.com/makeabilitylab/arduino/blob/master/OLED/FlappyBird/FlappyBird.ino
void drawStatusBar();
///////////////////////////////////////////////////////////////////////////////

void setup() {
  // Setup serial communication
  Serial.begin(9600);
  EEPROM.write(allTimeScoreEEPROMIdx, 0);

  // Initialize input pins
  pinMode(BALL1_INPUT_PIN, INPUT_PULLUP);
  pinMode(BALL2_INPUT_PIN, INPUT_PULLUP);
  pinMode(BALL3_INPUT_PIN, INPUT_PULLUP);
  pinMode(SPEAKER_INPUT_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  pinMode(ledPinA0, OUTPUT); // Set A0 pin as output
  pinMode(ledPinA1, OUTPUT); // Set A1 pin as output
  pinMode(ledPinA2, OUTPUT); // Set A2 pin as output

  // Initialize OLED display and show startup screen
  initializeOledAndShowStartupScreen();

  randomSeed(analogRead(A3));

  // Set the speed of each ball to a random integer between 2 and 5
  ball1.setSpeed(0, random(2, 4));
  ball2.setSpeed(0, random(2, 3));
  ball3.setSpeed(0, random(2, 4));

  ball1.setDrawFill(true);
  ball2.setDrawFill(true);
  ball3.setDrawFill(true);

  // Record start time for FPS calculation
  fpsStartTimeStamp = millis();
}

void playLoserTone() {
  const int noteDuration = 650;
  const int restDuration = 1500;
  const int vibrationDuration = noteDuration - 50;

  const float notes[] = {
    293.66,
    277.18,
    261.63
  }; // D4, D#4, E4

  for (int i = 0; i < 3; ++i) {
    tone(SPEAKER_INPUT_PIN, notes[i], noteDuration);
    digitalWrite(VIBRATION_PIN, HIGH);
    delay(vibrationDuration);
    digitalWrite(VIBRATION_PIN, LOW);
    delay(50);
  }

  noTone(SPEAKER_INPUT_PIN);
  digitalWrite(VIBRATION_PIN, LOW);
  delay(restDuration);
}

void loop() {
  // Clear display buffer
  display.clearDisplay();

  // Draw status bar
  drawStatusBar();

  // Check game state and perform actions accordingly
  if (gameState == NEW_GAME || gameState == GAME_OVER) {
    if (gameState == GAME_OVER && !loserTonePlayed) {
      playLoserTone();
      loserTonePlayed = true;
    }
    nonGamePlayLoop();
  } else if (gameState == PLAYING) {
    loserTonePlayed = false;
    gamePlayLoop();
  }

  // Control LEDs based on condition
  digitalWrite(ledPinA0, !ledOffA0 ? HIGH : LOW);
  digitalWrite(ledPinA1, !ledOffA1 ? HIGH : LOW);
  digitalWrite(ledPinA2, !ledOffA2 ? HIGH : LOW);

  // Calculate FPS
  calcFrameRate();

  // Render buffer to screen
  display.display();

  // Delay loop if necessary
  if (DELAY_LOOP_MS > 0) {
    delay(DELAY_LOOP_MS);
  }
}

void resetBall(Ball & ball) {
  // Reset ball position
  int resetY = -random(10, 100);
  ball.setY(resetY);

  // Calculate the number of milliseconds elapsed since the game started
  unsigned long currentTime = millis();
  int elapsedTime = currentTime - startTime;

  // Calculate the number of 3.5-second intervals elapsed
  int intervals = elapsedTime / 3500;

  // Calculate random speed within a range, with increasing tendency
  const float minSpeed = 2.0;
  const float maxSpeed = 17.0;
  float randomIncrement = random(intervals, intervals + 1);
  float newYSpeed = minSpeed + randomIncrement;

  // Ensure newYSpeed does not exceed maxSpeed
  newYSpeed = min(newYSpeed, maxSpeed);

  ball.setSpeed(0, newYSpeed);
}

void playNextNote() {
  if (notes[currentNoteIndex] != 0) {
    tone(SPEAKER_INPUT_PIN, notes[currentNoteIndex], 100);
  }

  currentNoteIndex++;

  if (currentNoteIndex >= songLength) {
    currentNoteIndex = 0;
  }
}

void handleBall(Ball & ball, int inputPin, TapZone & tapZone) {
  ball.update();

  // Check if ball reached the bottom
  if (ball.getTop() >= SCREEN_HEIGHT) {
    resetBall(ball);
  }

  // Handle button press
  if (digitalRead(inputPin) == LOW) {
    if (tapZone.overlaps(ball)) {
      // Reset ball position and increment points if ball overlaps tap zone
      resetBall(ball);
      points++;
      playNextNote(); // Play the next note in the melody when the ball is hit
    } else if (ball.getBottom() > 15 && points > 1) {
      // Handle when ball is not in tap zone
      Serial.println(ball.getBottom());
      Serial.println("Ball not in tap zone! ");
      digitalWrite(VIBRATION_PIN, HIGH);
      delay(125);
      tapZone.setBallVisible(false, inputPin);

      // Turn on corresponding LED based on the inputPin
      if (inputPin == BALL1_INPUT_PIN) {
        ledOffA0 = false;
      } else if (inputPin == BALL2_INPUT_PIN) {
        ledOffA1 = false;
      } else if (inputPin == BALL3_INPUT_PIN) {
        ledOffA2 = false;
      }
    }
  }

  // Handle missed beats
  if (tapZone.overlaps(ball) && digitalRead(inputPin) == HIGH) {
    if (ball.getTop() >= 60) {
      // Handle missed beat
      Serial.println("Missed the beat");
      tapZone.setBallVisible(false, inputPin);
      digitalWrite(VIBRATION_PIN, HIGH);
      delay(125);
      if (inputPin == BALL1_INPUT_PIN) {
        ledOffA0 = false;
      } else if (inputPin == BALL2_INPUT_PIN) {
        ledOffA1 = false;
      } else if (inputPin == BALL3_INPUT_PIN) {
        ledOffA2 = false;
      }
    }
  }

  // Draw the ball if it's visible
  if (tapZone.isBallVisible(inputPin)) {
    ball.draw(display);
  }

  // Make sure vibration is off
  digitalWrite(VIBRATION_PIN, LOW);
}

void initializeOledAndShowStartupScreen() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    // If display initialization fails, print error message and loop forever
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Define variables for text positioning
  int16_t x1, y1;
  uint16_t w, h;
  int yText = 10;

  // Clear the display buffer
  display.clearDisplay();

  // Show load screen
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);

  // Show the first line of the startup screen
  display.getTextBounds(STR_LOADSCREEN_APP_NAME_LINE1, 0, 0, & x1, & y1, & w, & h);
  display.setCursor(display.width() / 2 - w / 2, yText);
  display.print(STR_LOADSCREEN_APP_NAME_LINE1);

  // Move to the next line for the second line of the startup screen
  yText = yText + h + 1;
  display.getTextBounds(STR_LOADSCREEN_APP_NAME_LINE2, 0, 0, & x1, & y1, & w, & h);
  display.setCursor(display.width() / 2 - w / 2, yText);
  display.print(STR_LOADSCREEN_APP_NAME_LINE2);

  // Move to the next line for the creator's name
  yText = yText + h + 1;
  display.setTextSize(1);
  display.getTextBounds(STR_LOADSCREEN_CREATOR, 0, 0, & x1, & y1, & w, & h);
  display.setCursor(display.width() / 2 - w / 2, yText);
  display.print(STR_LOADSCREEN_CREATOR);

  // Display the content on the OLED screen
  display.display();

  // Delay to show the startup screen
  delay(LOAD_SCREEN_SHOW_MS);

  // Clear the display buffer after showing the startup screen
  display.clearDisplay();
  display.setTextSize(1);
}

void gamePlayLoop() {
  // Draw the tap zone on the display
  tapZone.draw(display); // (0, 49)

  // Handle ball interaction if it's visible in the tap zone
  if (tapZone.isBall1Visible()) {
    handleBall(ball1, BALL1_INPUT_PIN, tapZone);
  }

  if (tapZone.isBall2Visible()) {
    handleBall(ball2, BALL2_INPUT_PIN, tapZone);
  }

  if (tapZone.isBall3Visible()) {
    handleBall(ball3, BALL3_INPUT_PIN, tapZone);
  }

  // Check if all balls are no longer visible in the tap zone
  if (!tapZone.isBall1Visible() && !tapZone.isBall2Visible() && !tapZone.isBall3Visible()) {
    // Change game state to GAME_OVER and record the timestamp
    gameState = GAME_OVER;
    gameOverTimestamp = millis();
  }
}

void nonGamePlayLoop() {
  int16_t x1, y1;
  uint16_t w, h;
  int twoval = digitalRead(BALL2_INPUT_PIN);

  // Handle behavior for NEW_GAME state
  if (gameState == NEW_GAME) {
    // Display message prompting user to start the game
    display.getTextBounds(STR_PRESS_UP_TO_PLAY, 0, 0, & x1, & y1, & w, & h);
    display.setCursor(display.width() / 2 - w / 2, 20);
    display.print(STR_PRESS_UP_TO_PLAY);

    // Check if the user pressed the button to start the game
    if (twoval == LOW) {
      gameState = PLAYING;
    }
  }
  // Handle behavior for GAME_OVER state
  else if (gameState == GAME_OVER) {
    // Update high score if current session's score is higher
    if (points > highScoreCurSession) {
      highScoreCurSession = points;
    }

    // Read the all-time high score from EEPROM
    int value = EEPROM.read(allTimeScoreEEPROMIdx);

    // Update all-time high score if current session's score is higher
    if (highScoreCurSession > value) {
      EEPROM.write(allTimeScoreEEPROMIdx, highScoreCurSession);
      allTimeScore = highScoreCurSession;
    } else {
      allTimeScore = value;
    }

    // Display game over message and scores
    display.setTextSize(1);
    display.getTextBounds("<= Your Score", 0, 0, & x1, & y1, & w, & h);
    display.setCursor(20, 0);
    display.print("<= Your Score");

    display.setTextSize(2);
    display.getTextBounds(STR_GAME_OVER, 0, 0, & x1, & y1, & w, & h);
    int yText = 15;
    display.setCursor(display.width() / 2 - w / 2, yText);
    display.print(STR_GAME_OVER);

    yText = yText + h + 2;
    display.setTextSize(1);
    display.getTextBounds(STR_PRESS_UP_TO_PLAY, 0, 0, & x1, & y1, & w, & h);
    display.setCursor(display.width() / 2 - w / 2, yText + 5);
    display.print(STR_PRESS_UP_TO_PLAY);

    yText = yText + h + 2;
    display.setTextSize(1);
    display.getTextBounds(String(points), 0, 0, & x1, & y1, & w, & h);
    display.setCursor(display.width() / 2 - w / 2, yText + 10);
    display.print(String(points));

    display.setTextSize(1);
    display.setCursor(0, 55); // draw points
    display.print(allTimeScore);

    display.setTextSize(1);
    display.getTextBounds("<= Highest Score", 0, 0, & x1, & y1, & w, & h);
    display.setCursor(20, 55);
    display.print("<= High Score");

    // Reset the game if user presses the button after a delay from game over
    if (twoval == LOW && millis() - gameOverTimestamp >= IGNORE_INPUT_AFTER_GAME_OVER_MS) {
      initializeGameEntities(); // Reset game entities
      gameState = PLAYING; // Change game state to PLAYING
    }
  }
}

void initializeGameEntities() {
  // Reset variables related to game state and timing
  currentNoteIndex = 0;
  startTime = millis();

  // Reset the state of existing balls
  ball1.setX(32);
  ball1.setY(0);
  ball1.setRadius(6);

  ball2.setX(64);
  ball2.setY(0);
  ball2.setRadius(6);

  ball3.setX(96);
  ball3.setY(0);
  ball3.setRadius(6);

  // Reset session score
  points = 0;

  // Reset ball visibility using TapZone object
  tapZone.setBall1Visible(true);
  tapZone.setBall2Visible(true);
  tapZone.setBall3Visible(true);

  // Reset ball speeds with random values
  ball1.setSpeed(0, random(1, 3));
  ball2.setSpeed(0, random(1, 3));
  ball3.setSpeed(0, random(2, 3));

  // Reset LED status flags
  ledOffA0 = true;
  ledOffA1 = true;
  ledOffA2 = true;
}

void calcFrameRate() {
  unsigned long elapsedTime = millis() - fpsStartTimeStamp;
  frameCount++;
  if (elapsedTime > 1000) {
    fps = frameCount / (elapsedTime / 1000.0);
    fpsStartTimeStamp = millis();
    frameCount = 0;
  }
}

void drawStatusBar() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(points);
}
