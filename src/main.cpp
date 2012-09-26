/*
 * Target: AVR328P
 * Crystal: 16.000Mhz
 *
 *
 */
#include <Arduino.h>
#include <MIDI.h>

#define MIDI_CHANNEL	16

#define GROW_POT_PIN 	A0
#define GAP_PIN 	8


// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 80;    // the debounce time; increase if the output flickers

unsigned long lastGrowTime = 0;
unsigned long growDelay = 0;

uint8_t	lastGapState;
uint8_t	currentGapState;

volatile int controlValue = -1;


int get_grow_delay() {
	  int reading = analogRead(GROW_POT_PIN);

	  return reading / 12;
}

void grow_control_value() {

	if (controlValue < 128) {
		//Serial.println(controlValue);
		MIDI.sendControlChange(MIDI_CHANNEL, controlValue, 1);
		controlValue++;
	} else {
		//Timer1.stop();
	}
}

void setup() {
	MIDI.begin(MIDI_CHANNEL_OMNI);
	//Serial.begin(9600);

	pinMode(GROW_POT_PIN, INPUT);
	pinMode(GAP_PIN, INPUT);

	lastGapState = digitalRead(GAP_PIN);
	currentGapState = lastGapState;

	growDelay = get_grow_delay();
}

void loop() {

	unsigned long currentMillis = millis();

	MIDI.read();

	uint8_t reading = digitalRead(GAP_PIN);

	if (reading != lastGapState) {
		lastDebounceTime = currentMillis;
	}

	if ((currentMillis - lastDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:

		if (reading != currentGapState)
		{
			//Serial.print("Pin state=");
			//Serial.println(reading);

			if (HIGH == reading) {
				growDelay = get_grow_delay();
				controlValue = 0;
			} else {
				controlValue = -1;
				MIDI.sendControlChange(MIDI_CHANNEL, 0, 1);
				//Serial.println(controlValue);
			}

			currentGapState = reading;
		}
	}


	lastGapState = reading;

	if (controlValue < 0)
		return;

	if ((currentMillis - lastGrowTime) > growDelay) {
		grow_control_value();
		lastGrowTime = currentMillis;
	}

}

////////// ---------------------------------- ////////
int main(void) {
	init();
	setup();
	for (;;) {
		loop();
		//if (serialEventRun) serialEventRun();
	}
	return 0;
}
