#define STEP_PIN 2
#define DIR_PIN 3

#define BTN1 4
#define BTN2 5
#define BTN3 6
#define BTN4 7

long totalSteps = 0;

void stepMotor(int steps, bool dir)
{
  digitalWrite(DIR_PIN, dir);

  for (int i = 0; i < steps; i++)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(700);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(700);
  }

  if (dir)
    totalSteps += steps;
  else
    totalSteps -= steps;
}

void setup()
{
  Serial.begin(9600);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);

  Serial.println("Stepper Controller Ready");
}

void loop()
{
  if (digitalRead(BTN1) == LOW)
  {
    stepMotor(200, HIGH);     // Clockwise
    Serial.println("CW 200 Steps");
    delay(250);
  }

  if (digitalRead(BTN2) == LOW)
  {
    stepMotor(200, LOW);      // Counter Clockwise
    Serial.println("CCW 200 Steps");
    delay(250);
  }

  if (digitalRead(BTN3) == LOW)
  {
    stepMotor(800, HIGH);     // One Revolution CW
    Serial.println("1 Revolution CW");
    delay(250);
  }

  if (digitalRead(BTN4) == LOW)
  {
    totalSteps = 0;           // Home Position
    Serial.println("HOME SET");
    delay(250);
  }

  Serial.print("Total Steps = ");
  Serial.println(totalSteps);

  delay(50);
}
