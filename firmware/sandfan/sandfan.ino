// SandFan firmware - XIAO RP2040 (Earle Philhower arduino-pico core)
// Two PWM-driven motors (fan, sander), 5 buttons, sander stall protection.

// ---- Pin map (must match hardware/ schematic) ----
const uint8_t PIN_SHUNT    = D0;   // sander low-side shunt, via RC filter (ADC)
const uint8_t PIN_VBAT     = D1;   // battery divider, 1/2 (ADC)
const uint8_t PIN_FAN_UP   = D2;
const uint8_t PIN_FAN_DN   = D3;
const uint8_t PIN_SAND_UP  = D4;
const uint8_t PIN_SAND_DN  = D5;
const uint8_t PIN_LED_FAN  = D6;
const uint8_t PIN_LED_SAND = D7;
const uint8_t PIN_FAN_GATE = D8;   // MOSFET gate, fan
const uint8_t PIN_SAND_GATE= D9;   // MOSFET gate, sander
const uint8_t PIN_POWER    = D10;  // all-off / wake

// ---- Tunables ----
const uint8_t  LEVELS          = 5;        // 0 = off .. 5 = full
const float    SHUNT_OHMS      = 0.1f;
const float    STALL_AMPS      = 1.5f;     // tune against the real motor
const uint16_t STALL_MS        = 150;      // must exceed limit this long
const uint16_t STALL_LOCKOUT_MS= 2000;
const float    VBAT_LOW        = 3.3f;     // cut motors below this

uint8_t fanLevel = 0, sandLevel = 0;
uint32_t overSince = 0, lockoutUntil = 0;

struct Button {
  uint8_t pin; bool last = true; uint32_t t = 0;
  bool pressed() {                         // debounced falling edge
    bool now = digitalRead(pin);
    bool hit = false;
    if (now != last && millis() - t > 30) { hit = !now; last = now; t = millis(); }
    return hit;
  }
};
Button bFanUp{PIN_FAN_UP}, bFanDn{PIN_FAN_DN}, bSandUp{PIN_SAND_UP},
       bSandDn{PIN_SAND_DN}, bPower{PIN_POWER};

float readVolts(uint8_t pin) { return analogRead(pin) * 3.3f / 4095.0f; }

void applyOutputs(bool sandAllowed) {
  analogWrite(PIN_FAN_GATE,  fanLevel * 255 / LEVELS);
  analogWrite(PIN_SAND_GATE, sandAllowed ? sandLevel * 255 / LEVELS : 0);
  digitalWrite(PIN_LED_FAN,  fanLevel > 0);
  digitalWrite(PIN_LED_SAND, sandAllowed ? sandLevel > 0 : (millis() / 100) % 2);  // blink on stall
}

void setup() {
  analogReadResolution(12);
  analogWriteFreq(20000);                  // above audible
  analogWriteRange(255);
  for (uint8_t p : {PIN_FAN_UP, PIN_FAN_DN, PIN_SAND_UP, PIN_SAND_DN, PIN_POWER})
    pinMode(p, INPUT_PULLUP);
  for (uint8_t p : {PIN_LED_FAN, PIN_LED_SAND, PIN_FAN_GATE, PIN_SAND_GATE})
    pinMode(p, OUTPUT);
}

void loop() {
  if (bFanUp.pressed()   && fanLevel  < LEVELS) fanLevel++;
  if (bFanDn.pressed()   && fanLevel  > 0)      fanLevel--;
  if (bSandUp.pressed()  && sandLevel < LEVELS) sandLevel++;
  if (bSandDn.pressed()  && sandLevel > 0)      sandLevel--;
  if (bPower.pressed()) fanLevel = sandLevel = 0;

  // Battery cutoff (divider is 1/2)
  if (readVolts(PIN_VBAT) * 2.0f < VBAT_LOW) fanLevel = sandLevel = 0;

  // Stall protection: sustained over-current on the sander
  float amps = readVolts(PIN_SHUNT) / SHUNT_OHMS;
  uint32_t now = millis();
  if (sandLevel > 0 && amps > STALL_AMPS) {
    if (!overSince) overSince = now;
    if (now - overSince > STALL_MS) { lockoutUntil = now + STALL_LOCKOUT_MS; sandLevel = 0; }
  } else {
    overSince = 0;
  }

  applyOutputs(now >= lockoutUntil);
}
