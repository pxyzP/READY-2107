/*
 * M4_Line_Following_Simple.c
 * M-Task Example 4: Basic line following with reflectance sensors
 *
 * Description:
 * - Follow black line using reflectance sensors
 * - Simple control: center sensors drive forward, side sensors turn
 * - Display sensor status on UART
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

int main(){
    uint8_t rf_data;
    uint8_t center, left, right;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Reflectance_Init();
    UART0_Init();
    
    UART0_OutString("M-Task 4: Line Following\r\n");
    UART0_OutString("Following black line...\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    while(1){
        // Read sensors
        rf_data = Reflectance_Read(1000);
        
        // Extract sensor groups
        center = (rf_data >> 3) & 0x03;  // Bits 3,4
        left = (rf_data >> 5) & 0x07;    // Bits 5,6,7
        right = rf_data & 0x07;          // Bits 0,1,2
        
        // Line following logic
        if (center == 0x00) {
            // Center sensors on line - go straight
            Motor_Forward(BASE_SPEED, BASE_SPEED);
            UART0_OutString("Forward\r\n");
        } else if (left < right) {
            // Line to the left - turn left
            Motor_Left(TURN_SPEED, TURN_SPEED);
            UART0_OutString("Turn Left\r\n");
        } else if (right < left) {
            // Line to the right - turn right
            Motor_Right(TURN_SPEED, TURN_SPEED);
            UART0_OutString("Turn Right\r\n");
        } else {
            // Line lost - stop
            Motor_Stop();
            UART0_OutString("Line Lost\r\n");
        }
        
        Clock_Delay1ms(100);
    }
}
