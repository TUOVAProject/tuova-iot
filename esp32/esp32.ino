// tuova-iot
// ESP32S3-Devkit-C + SCT013G-100

const int adcPin1 = 4; 
const int adcPin2 = 7;
const int adcPin3 = 8;

long baseline1 = 0;
long baseline2 = 0;
long baseline3 = 0;

const int samples = 1000;

double Irms1 = 0;
double Irms2 = 0;
double Irms3 = 0;

// poor man's calibration function
void calibrate() {
  for (int i = 0; i < samples; i++) {
    baseline1 += analogRead(adcPin1);
    baseline2 += analogRead(adcPin2);
    baseline3 += analogRead(adcPin3);
  
  }
  baseline1 /= samples;
  baseline2 /= samples;
  baseline3 /= samples;
}

// run once at startup
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  calibrate();

}

// loop forever
void loop() {
  double sumSq1 = 0;
  double sumSq2 = 0;
  double sumSq3 = 0;

  for (int i = 0; i < samples; i++) {
    int raw1 = analogRead(adcPin1);
    int raw2 = analogRead(adcPin2);
    int raw3 = analogRead(adcPin3);

    int centered1 = raw1 - baseline1;
    int centered2 = raw2 - baseline2;
    int centered3 = raw3 - baseline3;

    double volts1 = centered1 * (3.3 / 4095.0);
    double volts2 = centered2 * (3.3 / 4095.0);
    double volts3 = centered3 * (3.3 / 4095.0);

    sumSq1 += volts1 * volts1;
    sumSq2 += volts2 * volts2;
    sumSq3 += volts3 * volts3;
  }

  double Vrms1 = sqrt(sumSq1 / samples);
  double Vrms2 = sqrt(sumSq2 / samples);
  double Vrms3 = sqrt(sumSq3 / samples);

  Irms1 = Vrms1 * 100.0;
  Irms2 = Vrms2 * 100.0;
  Irms3 = Vrms3 * 100.0;

  // serial listener
  if(Serial.available()) {
    String cmd = Serial.readStringUntil('\n');

    // m = get measurements as "VAL1,VAL2,VAL3"
    if(cmd == "m") {
      Serial.print(Irms1, 2);
      Serial.print(",");
      Serial.print(Irms2, 2);
      Serial.print(",");
      Serial.println(Irms3, 2);
    }

    // c = calibrate
    if(cmd == "c") {
      calibrate();
      Serial.println("Calibrated.");
    }

    // r = reset
    if(cmd == "r") {
      Serial.println("Restarting...");
      ESP.restart();
    }
     
  }

  delay(500);
}
