/*
    User Config for OSSM - Reference board users should tweak this to match their personal build.
*/
#pragma once

/*
        Motion System Config
*/

#define OSSM_STEP_PER_REV      2000      // How many steps per revolution of the motor (S1 off, S2 on, S3 on, S4 off)
#define OSSM_PULLEY_TEETH      20        // How many teeth has the pulley
#define OSSM_BELT_PITCH        2         // What is the timing belt pitch in mm
#define OSSM_MAX_RPM           3000.0    // Maximum RPM of motor
#define OSSM_MAX_ACCELERATION  25000     // Maximum linear acceleration in mm/s²

// This is in millimeters, and is what's used to define how much of
// your rail is usable.
// The absolute max your OSSM would have is the distance between the belt attachments subtract
// the linear block holder length (75mm on OSSM)
#define OSSM_MAX_STROKEINMM 150.0  // Real physical travel from one hard endstop to the other
#define OSSM_STROKEBOUNDARY 10.0  // Safe distance the motion is constrained to avoiding crashes

// Calculation Aid:
#define OSSM_STEP_PER_MM       OSSM_STEP_PER_REV / (OSSM_PULLEY_TEETH * OSSM_BELT_PITCH)
#define OSSM_MAX_SPEED         (OSSM_MAX_RPM / 60.0) * OSSM_PULLEY_TEETH * OSSM_BELT_PITCH

#define OSSM_DEPTH_RESULTION 5 // Depth Resultion in mm per encoder Klick
#define OSSM_STROKE_RESULTION 5 // STROKE Resultion in mm per encoder Klick
#define OSSM_ENCODER_RESULTION 36 // Klicks per turn
#define OSSM_USER_SPEEDLIMIT 300 // Speed in Cycles (in & out) per minute.

// The minimum value of the pot in percent
// prevents noisy pots registering commands when turned down to zero by user
#define OSSM_SPEED_POT_DEADZONE_PERCENT 1.

// Maximum size of the buffer in streaming mode. If more frames that that are
// sent, they will be dimissed.
#define OSSM_MAX_STREAMING_FRAME_BUFFER_SIZE 1024

// Uncomment the following line if you wish to print DEBUG info
#define DEBUG

// Some useful macros
#define MIN(X,Y) (((X)<(Y)) ? (X):(Y))
#define MAP(X,IN_MIN,IN_MAX,OUT_MIN,OUT_MAX) (((X)-(IN_MIN))*((OUT_MAX)-(OUT_MIN))/((IN_MAX)-(IN_MIN))+(OUT_MIN))
