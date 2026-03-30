/*
 * M1_Reflectance_Motor.c
 * M-Task Example 1: Turn 45 degrees left when sensor detects black line
 *
 * Description:
 * - Move RSLK forward
 * - Monitor reflectance sensor #3 (center-left)
 * - When black line detected, turn 45 degrees left
 * - Continue forward after turn
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
#include "..\..\inc\UART0.h"

#define BASE_SPEED 2500
#define TURN_SPEED 2000
#define TURN_45_DEG_MS 150  // Adjust based on testing

int main(){
    uint8_t rf_data;
    uint8_t sensor3;
    uint32_t turn_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Reflectance_Init();
    UART0_Init();
    
    UART0_OutString("M-Task 1: Turn Left on Line Detection\r\n");
    UART0_OutString("Sensor #3 detects line -> Turn 45 deg left\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting...\r\n");
    
    while(1){
        // Move forward
        Motor_Forward(BASE_SPEED, BASE_SPEED);
        
        // Read reflectance sensors
        rf_data = Reflectance_Read(1000);
        
        // Check sensor #3 (bit 3)
        sensor3 = (rf_data >> 3) & 0x01;
        
        // If sensor #3 detects black line (0 = dark)
        if (sensor3 == 0) {
            // Stop
            Motor_Stop();
            Clock_Delay1ms(100);
            
            UART0_OutString("Line detected! Turning 45 deg left...\r\n");
            
            // Turn 45 degrees left
            Motor_Left(TURN_SPEED, TURN_SPEED);
            Clock_Delay1ms(TURN_45_DEG_MS);
            
            // Stop
            Motor_Stop();
            Clock_Delay1ms(100);
            
            turn_count++;
            UART0_OutString("Turn complete. Total turns: ");
            UART0_OutUDec5(turn_count);
            UART0_OutString("\r\n");
            
            // Wait until sensor no longer sees line
            while(((Reflectance_Read(1000) >> 3) & 0x01) == 0){
                Clock_Delay1ms(50);
            }
            
            UART0_OutString("Continuing forward...\r\n");
        }
        
        Clock_Delay1ms(10);
    }
}
