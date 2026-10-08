// tuova-iot
// ESP32S3-Devkit-C + SCT013G-100

// analog pins to use
const int adcPin1 = 4;
const int adcPin2 = 7;
const int adcPin3 = 8;

// averaging sample count
const int samples = 1000;

// globals
long baseline1 = 0;
long baseline2 = 0;
long baseline3 = 0;

double Irms1 = 0;
double Irms2 = 0;
double Irms3 = 0;

// apparently this is something called "offset compensation"
// doesn't really matter whether there's current going or not since it's AC where both peaks have the same absolute amplitude
// might get skewed with less samples but should be fine like this
void compensateOffset() {
    // reset these here or expect trouble with repeated calls
    baseline1 = 0;
    baseline2 = 0;
    baseline3 = 0;

    // measure the baseline adc (offset) values
    for (int i = 0; i < samples; i++) {
        baseline1 += analogRead(adcPin1);
        baseline2 += analogRead(adcPin2);
        baseline3 += analogRead(adcPin3);
    }
    // average
    baseline1 /= samples;
    baseline2 /= samples;
    baseline3 /= samples;
}

// run once at startup
void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    compensateOffset();
}

// loop forever
void loop() {
    // averaging sums
    double sumSq1 = 0;
    double sumSq2 = 0;
    double sumSq3 = 0;

    for (int i = 0; i < samples; i++) {
        // read the adc values
        int raw1 = analogRead(adcPin1);
        int raw2 = analogRead(adcPin2);
        int raw3 = analogRead(adcPin3);

        // subtract the baseline offsets
        int centered1 = raw1 - baseline1;
        int centered2 = raw2 - baseline2;
        int centered3 = raw3 - baseline3;

        // convert to voltages
        double volts1 = centered1 * (3.3 / 4095.0);
        double volts2 = centered2 * (3.3 / 4095.0);
        double volts3 = centered3 * (3.3 / 4095.0);

        // square them
        sumSq1 += volts1 * volts1;
        sumSq2 += volts2 * volts2;
        sumSq3 += volts3 * volts3;
    }

    // calculate average rms voltages
    double Vrms1 = sqrt(sumSq1 / samples);
    double Vrms2 = sqrt(sumSq2 / samples);
    double Vrms3 = sqrt(sumSq3 / samples);

    // calculate rms currents
    Irms1 = Vrms1 * 100.0;
    Irms2 = Vrms2 * 100.0;
    Irms3 = Vrms3 * 100.0;

    // serial listener
    if (Serial.available()) {
        // get command
        String cmd = Serial.readStringUntil('\n');

        // m = get measurements as "VAL1,VAL2,VAL3"
        if (cmd == "m") {
            Serial.print(Irms1, 2);
            Serial.print(",");
            Serial.print(Irms2, 2);
            Serial.print(",");
            Serial.println(Irms3, 2);
        }

        // c = compensate!
        if (cmd == "c") {
            compensateOffset();
            Serial.println("Compensated for your enjoyment.");
        }

        // r = reset
        if (cmd == "r") {
            Serial.println("Restarting...");
            ESP.restart();
        }
    }

    // wait 500ms between iterations
    delay(500);
}
