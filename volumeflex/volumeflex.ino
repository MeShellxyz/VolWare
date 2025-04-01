const int potentiometerInputPins[] = {A0, A1};
const int numPotentiometers = 2; // Number of sliders

int noiseTreshold = 2; // Threshold for noise filtering

int prevPotValue[numPotentiometers] = {};

void setup() {
    // Initialize the serial communication
    Serial.begin(9600);

    if (noiseTreshold < 1) {
        Serial.println("Noise threshold must be greater than 0.");
        while (true) {
            delay(1000); // Wait indefinitely
        }
    }
}

void loop() {
    String msg;
    bool changed = false;
    for (int i = 0; i < numPotentiometers; i++) {
        delay(10);
        int potentiometerReading = analogRead(potentiometerInputPins[i]);
        if (abs(prevPotValue[i] - potentiometerReading) >= noiseTreshold) {
            prevPotValue[i] = potentiometerReading;
            changed = true;
        }
        msg += (i != numPotentiometers - 1)
                   ? String(potentiometerReading) + ","
                   : String(potentiometerReading) + "\n";
    }
    if (changed) {
        Serial.print(msg);
    }
    delay(50);
}
