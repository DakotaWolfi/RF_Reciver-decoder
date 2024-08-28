const int receiverPin = 8;
const int buttonPin = 7; // Button to toggle signal capturing
unsigned long highDuration;
unsigned long lowDuration;
String decodedSignal = "";
bool capturing = false;

void setup() {
  pinMode(receiverPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Button pin with internal pull-up resistor
  Serial.begin(115200);
}

void loop() {
  // Check if the button is pressed to toggle capturing
  if (digitalRead(buttonPin) == LOW) {
    capturing = !capturing;
    delay(300); // Debounce delay
    if (capturing) {
      Serial.println("Capturing started...");
    } else {
      Serial.println("Capturing stopped.");
    }
  }

  if (capturing) {
    captureSignal();
  }
}

void captureSignal() {
  // Wait for the pin to go HIGH
  while (digitalRead(receiverPin) == LOW);
  unsigned long startTime = micros();

  // Measure HIGH duration
  while (digitalRead(receiverPin) == HIGH);
  highDuration = micros() - startTime;

  // Wait for the pin to go LOW
  startTime = micros();

  // Measure LOW duration
  while (digitalRead(receiverPin) == LOW);
  lowDuration = micros() - startTime;

  // Decode bit based on duration
  if (highDuration > 700 && lowDuration < 300) {
    decodedSignal += "1";
  } else if (highDuration < 300 && lowDuration > 700) {
    decodedSignal += "0";
  } else if (highDuration > 1000) { // Detected a longer pause, likely end of the signal
    Serial.println("Full Decoded Signal: " + decodedSignal);
    if (validateChecksum(decodedSignal)) {
      extractAndPrintFields(decodedSignal);
    } else {
      Serial.println("Invalid signal (checksum failed).");
    }
    decodedSignal = ""; // Reset for the next signal
    Serial.println("__________________________________");
  }
}

bool validateChecksum(String signal) {
  // Ensure the signal is long enough
  if (signal.length() < 42) return false;

  // Extract the checksum and data fields
  String data = signal.substring(0, 32);
  String receivedChecksumStr = signal.substring(32, 40);
  unsigned long receivedChecksum = strtoul(receivedChecksumStr.c_str(), NULL, 2);

  // Calculate the checksum
  unsigned long calculatedChecksum = 0;
  for (int i = 0; i < 32; i += 8) {
    String byteStr = data.substring(i, i + 8);
    calculatedChecksum += strtoul(byteStr.c_str(), NULL, 2);
  }
  calculatedChecksum %= 256; // Ensure checksum is in the range of 0-255

  return calculatedChecksum == receivedChecksum;
}

void extractAndPrintFields(String signal) {
  if (signal.length() >= 42) {
    String transmitterID = signal.substring(0, 16);
    String channel = signal.substring(16, 20);
    String mode = signal.substring(20, 24);
    String strength = signal.substring(24, 32);
    String checksum = signal.substring(32, 40);
    String end = signal.substring(40, 42);

    unsigned long transmitterIDDecimal = strtoul(transmitterID.c_str(), NULL, 2);
    unsigned long channelDecimal = strtoul(channel.c_str(), NULL, 2);
    unsigned long strengthDecimal = strtoul(strength.c_str(), NULL, 2);
    unsigned long checksumDecimal = strtoul(checksum.c_str(), NULL, 2);
    unsigned long endDecimal = strtoul(end.c_str(), NULL, 2);

    String modeString = translateMode(mode);

    // Print the valid Transmitter ID, Channel, Mode, Strength, Checksum, and End
    Serial.println("Valid Signal:");
    Serial.println("Transmitter ID (Decimal): " + String(transmitterIDDecimal));
    Serial.println("Channel (Decimal): " + String(channelDecimal));
    Serial.println("Mode: " + modeString + " (Decimal: " + String(strtoul(mode.c_str(), NULL, 2)) + ")");
    Serial.println("Strength (Decimal): " + String(strengthDecimal));
    Serial.println("Checksum (Decimal): " + String(checksumDecimal));
    Serial.println("End (Decimal): " + String(endDecimal));
  } else {
    Serial.println("Signal too short, skipping...");
  }
}

String translateMode(String modeBits) {
  unsigned long modeValue = strtoul(modeBits.c_str(), NULL, 2);

  switch (modeValue) {
    case 1:
      return "Shock";
    case 2:
      return "Vibrate";
    case 3:
      return "Beep";
    default:
      return "Unknown";
  }
}
