/*
 * L5_Motor_Speed_Control.c
 * L-Task Example 5: Variable speed motor control with button
 *
 * Description:
 * - Press LaunchPad button to cycle through speeds
 * - Speeds: Stop -> Slow -> Medium -> Fast -> Stop
 * - Display current speed on UART
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Motor.h"
#include "..\..\inc\UART0.h"

#define SPEED_SLOW 2000
#define SPEED_MEDIUM 3000
#define SPEED_FAST 4500

uint8_t speed_state = 0;

void SetSpeed(uint8_t state){
    switch(state){
        case 0:
            Motor_Stop();
            UART0_OutString("Speed: STOPPED\r\n");
            break;
        case 1:
            Motor_Forward(SPEED_SLOW, SPEED_SLOW);
            UART0_OutString("Speed: SLOW (2000)\r\n");
            break;
        case 2:
            Motor_Forward(SPEED_MEDIUM, SPEED_MEDIUM);
            UART0_OutString("Speed: MEDIUM (3000)\r\n");
            break;
        case 3:
            Motor_Forward(SPEED_FAST, SPEED_FAST);
            UART0_OutString("Speed: FAST (4500)\r\n");
            break;
    }
}

int main(){
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    UART0_Init();
    
    UART0_OutString("L-Task 5: Variable Speed Control\r\n");
    UART0_OutString("Press button to cycle speeds\r\n\r\n");
    
    SetSpeed(0);
    
    while(1){
        // Check for button press
        if (LaunchPad_Input()) {
            // Button pressed
            while(LaunchPad_Input());  // Wait for release
            
            // Cycle to next speed
            speed_state = (speed_state + 1) % 4;
            SetSpeed(speed_state);
            
            Clock_Delay1ms(200);  // Debounce
        }
        
        Clock_Delay1ms(10);
    }
}
