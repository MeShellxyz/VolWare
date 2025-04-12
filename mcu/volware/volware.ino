/**
 * VolumeFlex - Potentiometer Value Reader
 *
 * This program reads values from physical knobs (potentiometers) connected to
 * the Arduino and reports changes over the serial connection. It filters out
 * small fluctuations to report only significant changes.
 *
 * HARDWARE SETUP:
 * - Connect first potentiometer middle pin to analog pin A0
 * - Connect second potentiometer middle pin to analog pin A1
 * - Connect outer pins of each potentiometer to 5V and GND
 *
 * OUTPUT FORMAT:
 * value1,value2,...,mute1,mute2,...\n
 * - where values are between 0-1023 representing potentiometer positions
 * - mute values are 0 or 1 indicating mute state (if applicable)
 */

/*
 * USER SPECIFIC SETTINGS
 */

// Hardware configuration - pins where potentiometers are connected
const int potentiometerInputPins[] = {A0, A1};
const int numPotentiometers = 2; // Number of potentiometers connected

const int muteInputPins[] = {2, 3}; // Digital pins for mute buttons
const int muteLedPins[] = {4, 5};   // Digital pins for mute LEDs
const int numMuteButtons = 0;       // Number of mute buttons connected

// PinMode settings for mute buttons
void setupMuteButtons() {
    for (int i = 0; i < numMuteButtons; i++) {
        pinMode(muteInputPins[i],
                INPUT_PULLUP); // Set mute pins as input with pull-up resistor
        pinMode(muteLedPins[i], OUTPUT); // Set mute LED pins as output
        digitalWrite(muteLedPins[i],
                     LOW); // Initialize mute LEDs to off (LOW)
    }
}

// Sensitivity setting - how much a value must change to be reported
int noiseThreshold =
    2; // Increase to filter more noise, decrease for higher sensitivity

/**
 * Setup function - runs once when Arduino powers on
 * Initializes communication and validates settings
 */
// Array to store previous readings for comparison
int potValues[numPotentiometers] = {};
byte muteValues[numPotentiometers] = {}; // Initialize mute values
void setup() {
    // Start serial communication
    Serial.begin(115200);

    if (numMuteButtons > 0) {
        setupMuteButtons(); // Setup mute buttons if any are connected
    }

    // Safety check to prevent threshold being too low
    if (noiseThreshold < 1) {
        Serial.println("Noise threshold must be greater than 0.");
        while (true) {
            delay(1000); // Wait indefinitely - program stops here
        }
    }
}

/**
 * Main program loop - runs continuously
 * Reads potentiometer values and sends updates when significant changes occur
 */
void loop() {
    String msg; // Message that will be sent via serial if changes are detected

    // Flag to track if any potentiometer changed significantly
    bool changed = false;

    // Check serial port for sync command
    if (Serial.available()) {
        char c = Serial.read(); // Read the incoming byte
        if (c == 's') {
            changed = true;
        } // Set changed to true if 's' is received
    }

    // Read each potentiometer one by one
    for (int i = 0; i < numPotentiometers; i++) {
        delay(10);
        // Read the current position of the potentiometer (range: 0-1023)
        int potReading = analogRead(potentiometerInputPins[i]);
        byte muteReading = digitalRead(potentiometerInputPins[i]);

        // Check if value changed enough to be reported (beyond noise
        // threshold)
        if (abs(potValues[i] - potReading) >= noiseThreshold) {
            potValues[i] = potReading; // Update stored value
            changed = true;            // Mark that we have a significant change
        }

        // Check if mute button pressed
        if (muteReading == HIGH && numMuteButtons > 0) {
            muteValues[i] = muteValues[i] == 0 ? 1 : 0; // Toggle mute state
            digitalWrite(muteLedPins[i],
                         muteValues[i] == 1 ? HIGH : LOW); // Update LED
            changed = true; // Mark that we have a significant change
        }

        // Build the output message with all current values Format
        //  : "value1,value2\n"
    }

    for (int i = 0; i < numPotentiometers; i++) {
        msg += String(potValues[i]); // Append potentiometer value to message
        msg += ",";                  // Add comma separator
    }
    for (int i = 0; i < numMuteButtons; i++) {
        msg += String(muteValues[i]); // Append mute value to message
        msg += ",";                   // Add comma separator
    }
    msg.remove(msg.length() - 1); // Remove the last comma
    msg += "\n";                  // Add newline character at the end

    // Only send data when at least one value has changed significantly
    if (changed) {
        Serial.print(msg); // Send the formatted message
    }
}