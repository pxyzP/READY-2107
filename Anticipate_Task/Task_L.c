/*
 * Task_L.c
 * L Task - LED blinking when turning, Tachometer for wheel difference
 *
 * Features:
 * - Blink LED when turning left/right
 * - Stop blinking when going straight
 * - Use tachometer to check difference between left and right wheels
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\inc\Clock.h"
#include "..\inc\Motor.h"
#include "..\inc\Reflectance.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Bump.h"
#include "..\inc\Tachometer.h"
#include "..\inc\UART0.h"

// Motor speed settings
#define BASE_SPEED 2500
#define TURN_SPEED 1500

// Robot states
#define STATE_STRAIGHT 0
#define STATE_TURN_LEFT 1
#define STATE_TURN_RIGHT 2

// Global variables
uint8_t current_state = STATE_STRAIGHT;
uint8_t bump_data;
int bump_flag = 0;

// Tachometer variables
uint16_t leftSpeed, rightSpeed;
enum TachDirection leftDir, rightDir;
int32_t leftSteps, rightSteps;

void BumpHandler(){
    P4->IFG &= ~0xFF;
    bump_data = Bump_Read();
    if (bump_data != 0xFF) {
        bump_flag = 1;
        Motor_Stop();
    }
}

void Blink(int time){
    LaunchPad_LED(1);
    Clock_Delay1ms(time);
    LaunchPad_LED(0);
    Clock_Delay1ms(time);
}

// Blink LED continuously while in turning state
void BlinkWhileTurning(uint8_t state){
    if (state == STATE_TURN_LEFT || state == STATE_TURN_RIGHT) {
        // Blink LED
        P1OUT ^= 0x01;  // Toggle LED
    } else {
        // Straight - turn off LED
        P1OUT &= ~0x01;  // LED off
    }
}

// Determine line position from reflectance data
uint8_t GetLinePosition(uint8_t sensors) {
    // Check center sensors (bits 3,4)
    if ((sensors & 0x18) == 0x00) {
        return STATE_STRAIGHT;
    }
    // Check if line is to the left (bits 5,6,7)
    if ((sensors & 0xE0) < (sensors & 0x07)) {
        return STATE_TURN_LEFT;
    }
    // Check if line is to the right (bits 0,1,2)
    if ((sensors & 0x07) < (sensors & 0xE0)) {
        return STATE_TURN_RIGHT;
    }
    return STATE_STRAIGHT;
}

// Control motors based on state
void ControlMotors(uint8_t state) {
    switch(state) {
        case STATE_STRAIGHT:
            Motor_Forward(BASE_SPEED, BASE_SPEED);
            break;
        case STATE_TURN_LEFT:
            Motor_Left(TURN_SPEED, TURN_SPEED);
            break;
        case STATE_TURN_RIGHT:
            Motor_Right(TURN_SPEED, TURN_SPEED);
            break;
    }
}

void TimedPause(uint32_t time){
    Clock_Delay1ms(time);
    Motor_Stop();
    UART0_OutString("Press button to start...\r\n");
    while(LaunchPad_Input()==0);
    while(LaunchPad_Input());
}

int main(){
    uint8_t rf_data;
    uint32_t loop_count = 0;
    int32_t wheel_diff;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Motor_Stop();
    Reflectance_Init();
    Bump_Init(&BumpHandler);
    Tachometer_Init();
    UART0_Init();
    
    UART0_OutString("Task L - LED Blink on Turn + Tachometer\r\n");
    UART0_OutString("LED blinks when turning left/right\r\n");
    UART0_OutString("LED off when going straight\r\n");
    
    // Wait for button press
    TimedPause(1000);
    
    // Start indication
    Blink(100);
    Blink(100);
    Blink(100);
    
    UART0_OutString("Starting...\r\n");
    
    while(1){
        // Read reflectance sensors
        rf_data = Reflectance_Read(1000);
        
        // Determine state based on line position
        current_state = GetLinePosition(rf_data);
        
        // Get tachometer data
        Tachometer_Get(&leftSpeed, &leftDir, &leftSteps,
                       &rightSpeed, &rightDir, &rightSteps);
        
        // Calculate wheel difference (for monitoring)
        wheel_diff = leftSteps - rightSteps;
        
        // Check for bump
        if (bump_flag) {
            bump_flag = 0;
            Motor_Stop();
            P1OUT &= ~0x01;  // LED off
            UART0_OutString("BUMP! Stopped.\r\n");
            
            // Back up
            Motor_Backward(BASE_SPEED, BASE_SPEED);
            Clock_Delay1ms(500);
            Motor_Stop();
            
            TimedPause(500);
        }
        
        // Blink LED if turning
        BlinkWhileTurning(current_state);
        
        // Control motors
        ControlMotors(current_state);
        
        // Print debug info every 50 loops (~500ms)
        if (loop_count % 50 == 0) {
            UART0_OutString("State: ");
            switch(current_state) {
                case STATE_STRAIGHT:
                    UART0_OutString("STRAIGHT");
                    break;
                case STATE_TURN_LEFT:
                    UART0_OutString("LEFT");
                    break;
                case STATE_TURN_RIGHT:
                    UART0_OutString("RIGHT");
                    break;
            }
            
            UART0_OutString(" | L_speed: ");
            UART0_OutUDec5(leftSpeed);
            UART0_OutString(" R_speed: ");
            UART0_OutUDec5(rightSpeed);
            
            UART0_OutString(" | L_steps: ");
            UART0_OutSDec(leftSteps);
            UART0_OutString(" R_steps: ");
            UART0_OutSDec(rightSteps);
            
            UART0_OutString(" | Diff: ");
            UART0_OutSDec(wheel_diff);
            UART0_OutString("\r\n");
        }
        
        loop_count++;
        Clock_Delay1ms(10);
    }
}
