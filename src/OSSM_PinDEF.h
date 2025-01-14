/*
    Pin Definitions - Drivers, Buttons and Remotes
    OSSM Reference board users are unlikely to need to modify this! See OSSM_Config.h
*/

/*
        Driver pins
*/

// Pin that pulses on servo/stepper steps - likely labelled PUL on drivers.
#define OSSM_SERVO_PULSE       14
// Pin connected to driver/servo step direction - likely labelled DIR on drivers.
// N.b. to iHSV57 users - DIP switch #5 can be flipped to invert motor direction entirely
#define OSSM_SERVO_DIR         27
// Enable Pin Motor Driver
#define OSSM_SERVO_ENABLE      26

#define OSSM_SERVO_ALM_PIN        39        // Servo ALM Output
#define OSSM_SERVO_PED_PIN        36        // Servo PED Output

/*
    Homing and safety pins
*/
#define OSSM_SERVO_ENDSTOP     12        // Homing Pin one Homing Switch IO pin to Ground

/*These are configured for the OSSM Remote - which has a screen, a potentiometer and an encoder which clicks*/
#define OSSM_SPEED_POT_PIN 34                // Speed Poti Pin
#define OSSM_ENC_SW 35                       // Ecoder Button Switch
#define OSSM_ENC_CLK 18                      // Encoder A
#define OSSM_ENC_DT 5                        // Encoder B
#define OSSM_ENC_TOL 1                       // Nb of clicks of Tolerance
#define OSSM_V_LONG_PRESS_MS 300             // Encoder Switch Long Press Activation Time in ms
#define OSSM_LONG_PRESS_MS 50                // Encoder Switch Short Press Activation Time in ms
#define OSSM_REMOTE_SDA 21
#define OSSM_REMOTE_CLK 19
#define OSSM_REMOTE_ADDRESS 0x3C             // I2C Address - use 0x3C or 0x3D depending on your display
