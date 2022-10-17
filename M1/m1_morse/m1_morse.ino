void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  dot();dot();dot();dot();endLetter(); // H
  dot();endLetter(); // E
  dot();dash();dot();dot();endLetter(); // L
  dot();dash();dot();dot();endLetter(); // L
  dash();dash();dash();endWord(); // O

  dot();dot();endLetter(); // I
  dash();dash();endLetter(); // M
  dot();dot();dash();endWord(); // U
}

const int dot_duration = 200;
const int dash_duration = 3 * dot_duration;
const int space_duration = dot_duration;
const int letter_space_duration = 3 * dot_duration;
const int word_space_duration = 7 * dot_duration;

void dot() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(dot_duration);
  digitalWrite(LED_BUILTIN, LOW);
  delay(space_duration);
}

void dash() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(dash_duration);
  digitalWrite(LED_BUILTIN, LOW);
  delay(space_duration);
}

void endLetter() {
  delay(letter_space_duration - space_duration);
}

void endWord() {
  delay(word_space_duration - space_duration);
}
