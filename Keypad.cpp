// #include <Arduino.h>

// // Five-button text entry: UP/DOWN select a character, LEFT deletes,
// // RIGHT inserts a space, and ENTER adds the selected character.
// // Hold ENTER for 800 ms to submit the completed text over Serial.
// namespace {
// constexpr uint8_t BUTTON_COUNT = 5;
// constexpr uint8_t UP_BUTTON = 0;
// constexpr uint8_t DOWN_BUTTON = 1;
// constexpr uint8_t BACKSPACE_BUTTON = 2;
// constexpr uint8_t SPACE_BUTTON = 3;
// constexpr uint8_t ENTER_BUTTON = 4;

// // Change these GPIOs to match the wiring. Buttons connect each GPIO to GND.
// const uint8_t buttonPins[BUTTON_COUNT] = {32, 33, 25, 26, 27};
// constexpr unsigned long DEBOUNCE_MS = 35;
// constexpr unsigned long SUBMIT_HOLD_MS = 800;
// constexpr size_t MAX_TEXT_LENGTH = 32;

// const char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
// constexpr size_t CHARACTER_COUNT = sizeof(characters) - 1;

// bool lastRawState[BUTTON_COUNT] = {};
// bool stablePressed[BUTTON_COUNT] = {};
// unsigned long lastStateChange[BUTTON_COUNT] = {};
// unsigned long enterPressedAt = 0;
// String text;
// size_t selectedCharacter = 0;

// void showState() {
// 	Serial.print("Text: [");
// 	Serial.print(text);
// 	Serial.print("]  Selected: ");
// 	Serial.println(characters[selectedCharacter]);
// }

// void cycleCharacter(int direction) {
// 	selectedCharacter = (selectedCharacter + CHARACTER_COUNT + direction) % CHARACTER_COUNT;
// 	showState();
// }

// void appendCharacter(char value) {
// 	if (text.length() < MAX_TEXT_LENGTH) {
// 		text += value;
// 	}
// 	showState();
// }

// void submitText() {
// 	Serial.print("Submitted: ");
// 	Serial.println(text);
// 	text = "";
// 	showState();
// }

// void updateButtons() {
// 	const unsigned long now = millis();
// 	for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
// 		const bool rawPressed = digitalRead(buttonPins[i]) == LOW;
// 		if (rawPressed != lastRawState[i]) {
// 			lastRawState[i] = rawPressed;
// 			lastStateChange[i] = now;
// 		}

// 		if (rawPressed == stablePressed[i] || now - lastStateChange[i] < DEBOUNCE_MS) {
// 			continue;
// 		}

// 		stablePressed[i] = rawPressed;
// 		if (i == ENTER_BUTTON) {
// 			if (stablePressed[i]) {
// 				enterPressedAt = now;
// 			} else if (now - enterPressedAt >= SUBMIT_HOLD_MS) {
// 				submitText();
// 			} else {
// 				appendCharacter(characters[selectedCharacter]);
// 			}
// 		} else if (stablePressed[i]) {
// 			switch (i) {
// 				case UP_BUTTON: cycleCharacter(1); break;
// 				case DOWN_BUTTON: cycleCharacter(-1); break;
// 				case BACKSPACE_BUTTON:
// 					if (!text.isEmpty()) text.remove(text.length() - 1);
// 					showState();
// 					break;
// 				case SPACE_BUTTON: appendCharacter(' '); break;
// 			}
// 		}
// 	}
// }
// }  // namespace

// void setup() {
// 	Serial.begin(115200);
// 	text.reserve(MAX_TEXT_LENGTH);

// 	for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
// 		pinMode(buttonPins[i], INPUT_PULLUP);
// 		lastRawState[i] = digitalRead(buttonPins[i]) == LOW;
// 		stablePressed[i] = lastRawState[i];
// 		lastStateChange[i] = millis();
// 	}

// 	Serial.println("Five-button alphanumeric input ready.");
// 	Serial.println("UP/DOWN: select | LEFT: backspace | RIGHT: space | ENTER: add");
// 	Serial.println("Hold ENTER to submit.");
// 	showState();
// }

// void loop() {
// 	updateButtons();
// }
