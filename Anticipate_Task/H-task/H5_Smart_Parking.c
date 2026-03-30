/*
 * H5_Smart_Parking.c
 * H-Task Example 5: Parallel parking with tachometer precision
 *
 * Description:
 * - Move forward until parking space detected (gap in black line)
 * - Perform parallel parking maneuver:
 *   1. Detect parking entrance
 *   2. Reverse while turning right 45°
 *   3. Straighten wheels
 *   4. Align parallel to parking line
 * - Use tachometer for precise wheel control
 * - Display parking status and tachometer readings
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Motor.h"
#include "..\..\inc\Reflectance.h"
#include "..\..\inc\Tachometer.h"
#include "..\..\inc\UART0.h"

#define BASE_SPEED 2000
#define PARK_SPEED 1500
#define TURN_45_STEPS 50   // Approximate steps for 45° turn
#define REVERSE_STEPS 100  // Steps to reverse

typedef enum {
    SEARCH_PARKING,
    FOUND_SPACE,
    PARKING_STEP1,
    PARKING_STEP2,
    PARKING_STEP3,
    PARKED
} ParkState;

int main(){
    uint8_t rf_data;
    ParkState state = SEARCH_PARKING;
    uint16_t leftSpeed, rightSpeed;
    enum TachDirection leftDir, rightDir;
    int32_t leftSteps, rightSteps;
    int32_t parkStartSteps = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Reflectance_Init();
    Tachometer_Init();
    UART0_Init();
    
    UART0_OutString("H-Task 5: Smart Parallel Parking\r\n");
    UART0_OutString("Searching for parking space...\r\n\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    while(state != PARKED){
        // Get tachometer data
        Tachometer_Get(&leftSpeed, &leftDir, &leftSteps,
                       &rightSpeed, &rightDir, &rightSteps);
        
        // Read reflectance
        rf_data = Reflectance_Read(1000);
        
        switch(state){
            case SEARCH_PARKING:
                // Move forward along line
                Motor_Forward(BASE_SPEED, BASE_SPEED);
                
                // Check for parking space (gap in line - all sensors see white)
                if (rf_data == 0xFF) {
                    state = FOUND_SPACE;
                    Motor_Stop();
                    UART0_OutString("*** Parking Space Found! ***\r\n");
                    Clock_Delay1ms(500);
                }
                break;
                
            case FOUND_SPACE:
                UART0_OutString("Initiating parking maneuver...\r\n");
                parkStartSteps = rightSteps;
                state = PARKING_STEP1;
                break;
                
            case PARKING_STEP1:
                UART0_OutString("Step 1: Reverse + Turn Right 45deg\r\n");
                // Reverse while turning right
                Motor_Backward(PARK_SPEED - 500, PARK_SPEED);  // Differential for turn
                
                // Wait until turned enough
                if ((rightSteps - parkStartSteps) >= TURN_45_STEPS) {
                    Motor_Stop();
                    UART0_OutString("Step 1 complete - R_steps: ");
                    UART0_OutSDec(rightSteps - parkStartSteps);
                    UART0_OutString("\r\n");
                    Clock_Delay1ms(500);
                    parkStartSteps = rightSteps;
                    state = PARKING_STEP2;
                }
                break;
                
            case PARKING_STEP2:
                UART0_OutString("Step 2: Straighten + Continue Reverse\r\n");
                // Reverse straight
                Motor_Backward(PARK_SPEED, PARK_SPEED);
                
                // Reverse for a bit
                if ((rightSteps - parkStartSteps) >= REVERSE_STEPS) {
                    Motor_Stop();
                    UART0_OutString("Step 2 complete - R_steps: ");
                    UART0_OutSDec(rightSteps - parkStartSteps);
                    UART0_OutString("\r\n");
                    Clock_Delay1ms(500);
                    parkStartSteps = rightSteps;
                    state = PARKING_STEP3;
                }
                break;
                
            case PARKING_STEP3:
                UART0_OutString("Step 3: Align Parallel\r\n");
                // Turn left slightly to align
                Motor_Backward(PARK_SPEED, PARK_SPEED - 500);
                
                if ((rightSteps - parkStartSteps) >= TURN_45_STEPS) {
                    Motor_Stop();
                    UART0_OutString("Step 3 complete - R_steps: ");
                    UART0_OutSDec(rightSteps - parkStartSteps);
                    UART0_OutString("\r\n");
                    state = PARKED;
                }
                break;
                
            case PARKED:
                // Will exit loop
                break;
        }
        
        Clock_Delay1ms(10);
    }
    
    // Parking complete
    Motor_Stop();
    UART0_OutString("\r\n*** PARKING COMPLETE ***\r\n");
    UART0_OutString("Final tachometer readings:\r\n");
    UART0_OutString("Left wheel: ");
    UART0_OutSDec(leftSteps);
    UART0_OutString(" steps\r\n");
    UART0_OutString("Right wheel: ");
    UART0_OutSDec(rightSteps);
    UART0_OutString(" steps\r\n");
    
    // Victory blink
    for (int i = 0; i < 10; i++) {
        LaunchPad_LED(1);
        Clock_Delay1ms(100);
        LaunchPad_LED(0);
        Clock_Delay1ms(100);
    }
    
    while(1);
}
