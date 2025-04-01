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
 * value1,value2
 * - where values are between 0-1023 representing potentiometer positions
 */

/*
 * USER SPECIFIC SETTINGS
 */

// Hardware configuration - pins where potentiometers are connected
const int potentiometerInputPins[] = {A0, A1};
const int numPotentiometers = 2; // Number of potentiometers connected

// Sensitivity setting - how much a value must change to be reported
int noiseThreshold =
    2; // Increase to filter more noise, decrease for higher sensitivity

/**
 * Setup function - runs once when Arduino powers on
 * Initializes communication and validates settings
 */
// Array to store previous readings for comparison
int prevPotValue[numPotentiometers] = {};
void setup() {
    // Start serial communication at 9600 bits per second
    Serial.begin(9600);

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

    // Read each potentiometer one by one
    for (int i = 0; i < numPotentiometers; i++) {
        delay(10); // Short pause for stable readings

        // Read the current position of the potentiometer (range: 0-1023)
        int potentiometerReading = analogRead(potentiometerInputPins[i]);

        // Check if value changed enough to be reported (beyond noise threshold)
        if (abs(prevPotValue[i] - potentiometerReading) >= noiseThreshold) {
            prevPotValue[i] = potentiometerReading; // Update stored value
            changed = true; // Mark that we have a significant change
        }

        // Build the output message with all current values
        // Format: "value1,value2\n"
        msg +=
            (i != numPotentiometers - 1)
                ? String(potentiometerReading) + "," // Add comma between values
                : String(potentiometerReading) +
                      "\n"; // Add newline after last value
    }

    // Only send data when at least one value has changed significantly
    if (changed) {
        Serial.print(msg); // Send the formatted message
    }

    delay(50); // Wait before next reading cycle - adjust for response speed
}