/*
 */

#include <FastLED.h>
#include <Fsm.h>
#include <NewPing.h>

// NewPing
#define TRIGGER_PIN  12
#define ECHO_PIN     11
#define MAX_DISTANCE 400

// Finite state machine
#define EVENT_PING   0
#define TIMEOUT_LED  3000

// WS2812B
#define BRIGHTNESS  200
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
#define LED_PIN     5
#define NUM_LEDS    5

// Prototypes
void check_ping();
void led_off();
void led_on();
void print_uS(unsigned int uS);

// Instance of HC-SR04
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned int last = 0;
bool led_state = false;

// Finite state machine states
State state_led_off(&led_off, &check_ping, NULL);
State state_led_on(&led_on, &check_ping, NULL);
// New finite state machine
Fsm fsm(&state_led_off);

// FastLED
CRGB leds[NUM_LEDS];

/*
 */
void check_ping()
{
    // Get current distance in cm
    unsigned int uS = NewPingConvert(sonar.ping_median(), US_ROUNDTRIP_CM);
    if (0 == uS)
    {
        // Sanity check
        return;
    }
    if ((uS < last - 5) || (uS > last + 5))
    {
        Serial.println(F("check_ping(): EVENT_PING"));
        if (!led_state)
        {
            // Transition state if LED is not turned on
            fsm.trigger(EVENT_PING);
        }
    }
    last = uS;
    print_uS(uS);
}

/*
 */
void led_off()
{
    Serial.println(F("led_off()"));
    digitalWrite(LED_BUILTIN, LOW);
    // Clear before drawing
    FastLED.clear();
    FastLED.show();
    // Disable LED state
    led_state = false;
}

/*
 */
void led_on()
{
    Serial.println(F("led_on()"));
    digitalWrite(LED_BUILTIN, HIGH);
    // Update the display
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
    // Enable LED state
    led_state = true;
}

/*
 */
void print_uS(unsigned int uS)
{
    Serial.print("Ping: ");
    Serial.print(uS);
    Serial.println("cm");
}

/*
 */
void setup()
{
    // Initialize serial communications with the PC
    Serial.begin(115200);
    // Do nothing if no serial port is opened
    while (!Serial);

    // Set up LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Set up transitions
    fsm.add_transition(&state_led_off, &state_led_on, EVENT_PING, NULL);
    fsm.add_timed_transition(&state_led_on, &state_led_off, TIMEOUT_LED, NULL);
    fsm.add_transition(&state_led_on, &state_led_off, EVENT_PING, NULL);

    // Set up FastLED
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    Serial.println(F("Setup(): Started"));
}

/*
 */
void loop()
{
    // Run the finite state machine
    fsm.run_machine();
    // Wait
    delay(50);
}
