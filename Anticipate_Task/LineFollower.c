/*
 * LineFollower.c
 * Line Following Robot using Reflectance Sensors
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
#include "..\inc\UART0.h"

// Motor speed settings
#define BASE_SPEED 2500
#define TURN_SPEED 1500
#define FAST_SPEED 3500
#define SLOW_SPEED 1000

// Line position states
#define ON_LINE_CENTER 0
#define LINE_LEFT 1
#define LINE_RIGHT 2
#define LINE_LOST 3

// Global variables
uint8_t bump_data;
int bump_flag = 0;

void BumpHandler(){
    P4->IFG &= ~0xFF;  // Clear interrupt flags
    bump_data = Bump_Read();
    if (bump_data != 0xFF) {
        bump_flag = 1;
        Motor_Stop();
    }
}

// Analyze reflectance sensor data to determine line position
uint8_t GetLinePosition(uint8_t sensors) {
    // sensors = 8-bit value from Reflectance_Read()
    // bit pattern: [S7 S6 S5 S4 S3 S2 S1 S0]
    // Dark line = 0, Light surface = 1
    
    // Check if the  center sensors on black (bits 3,4 - middle two sensors)
    if ((sensors & 0x18) == 0x00) { // 0001 1000
        // Both center sensors see dark line
        return ON_LINE_CENTER;
    }
    
    // Check if line is to the left (bits 5,6,7)
    if ((sensors & 0xE0) < (sensors & 0x07)) {
        return LINE_LEFT;
    }
    
    // Check if line is to the right (bits 0,1,2)
    if ((sensors & 0x07) < (sensors & 0xE0)) {
        return LINE_RIGHT;
    }
    
    // No clear line detected
    return LINE_LOST;
}



// Control motors based on line position
void FollowLine(uint8_t position, int32_t center) {
    switch(position) {
        case ON_LINE_CENTER:
            // Line is centered - go straight
            if (center == 0 || center == -1) {
                // Perfect center
                Motor_Forward(BASE_SPEED, BASE_SPEED);
            } else if (center < 0) {
                // Slightly left - compensate
                Motor_Forward(BASE_SPEED - 300, BASE_SPEED);
            } else {
                // Slightly right - compensate
                Motor_Forward(BASE_SPEED, BASE_SPEED - 300);
            }
            break;
            
        case LINE_LEFT:
            // Line is to the left - turn left
            Motor_Left(TURN_SPEED, TURN_SPEED);
            break;
            
        case LINE_RIGHT:
            // Line is to the right - turn right
            Motor_Right(TURN_SPEED, TURN_SPEED);
            break;
            
        case LINE_LOST:
            // Lost the line - stop and search
            Motor_Stop();
            Clock_Delay1ms(100);
            // Try slow backward to find line again
            Motor_Backward(SLOW_SPEED, SLOW_SPEED);
            Clock_Delay1ms(200);
            Motor_Stop();
            break;
    }
}

void Blink(int time){
    LaunchPad_LED(1);
    Clock_Delay1ms(time);
    LaunchPad_LED(0);
    Clock_Delay1ms(time);
}

void TimedPause(uint32_t time){
    Clock_Delay1ms(time);
    Motor_Stop();
    UART0_OutString("Press button to start...\r\n");
    while(LaunchPad_Input()==0);  // wait for touch
    while(LaunchPad_Input());     // wait for release
}

int main(){
    uint8_t rf_data;
    int32_t rf_center;
    uint8_t line_position;
    uint32_t loop_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Motor_Stop();
    Reflectance_Init();
    Bump_Init(&BumpHandler);
    UART0_Init();
    
    UART0_OutString("Line Follower Initialized\r\n");
    UART0_OutString("Reflectance Sensor Values:\r\n");
    UART0_OutString("  0 = Dark (line)\r\n");
    UART0_OutString("  1 = Light (surface)\r\n");
    
    // Wait for button press to start
    TimedPause(1000);
    
    // Calibration blink
    Blink(100);
    Blink(100);
    Blink(100);
    
    UART0_OutString("Starting line following...\r\n");
    
    while(1){
        // Read reflectance sensors
        rf_data = Reflectance_Read(1000);
        rf_center = Reflectance_Center(rf_data);
        
        // Determine line position
        line_position = GetLinePosition(rf_data);
        
        // Check for bump
        if (bump_flag) {
            bump_flag = 0;
            Motor_Stop();
            UART0_OutString("BUMP detected! Stopped.\r\n");
            
            // Back up slightly
            Motor_Backward(BASE_SPEED, BASE_SPEED);
            Clock_Delay1ms(500);
            Motor_Stop();
            
            // Wait for button press to continue
            TimedPause(500);
        }
        
        // Follow the line
        FollowLine(line_position, rf_center);




        
        // Print debug info every 100 loops (~1 second)
        if (loop_count % 100 == 0) {
            UART0_OutString("Sensors: 0b");
            // Print binary representation
            for (int i = 7; i >= 0; i--) {
                if (rf_data & (1 << i)) {
                    UART0_OutChar('1');
                } else {
                    UART0_OutChar('0');
                }
            }
            UART0_OutString(" Center: ");
            UART0_OutSDec(rf_center);
            UART0_OutString(" Pos: ");
            switch(line_position) {
                case ON_LINE_CENTER: UART0_OutString("CENTER"); break;
                case LINE_LEFT: UART0_OutString("LEFT"); break;
                case LINE_RIGHT: UART0_OutString("RIGHT"); break;
                case LINE_LOST: UART0_OutString("LOST"); break;
            }
            UART0_OutString("\r\n");
        }
        
        loop_count++;




        
        Clock_Delay1ms(10);  // 10ms delay between readings
    }
}
