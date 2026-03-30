/*
 * M2_Bump_Motor.c
 * M-Task Example 2: Obstacle avoidance with bump switch
 *
 * Description:
 * - Move RSLK forward
 * - When bump switch #2 pressed:
 *   1. Reverse 500ms
 *   2. Turn 90 degrees right
 *   3. Continue forward
 * - Display "Obstacle Avoided" on serial terminal
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Motor.h"
#include "..\..\inc\Bump.h"
#include "..\..\inc\UART0.h"

#define BASE_SPEED 2500
#define TURN_SPEED 2000
#define TURN_90_DEG_MS 300  // Adjust based on testing

volatile uint8_t bump_flag = 0;
volatile uint8_t bump_data = 0xFF;

void BumpHandler(){
    P4->IFG &= ~0xFF;
    bump_data = Bump_Read();
    
    // Check if bump switch #2 (bit 2) is pressed
    if (!(bump_data & (1 << 2))) {
        bump_flag = 1;
        Motor_Stop();
    }
}

int main(){
    uint32_t avoid_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Bump_Init(&BumpHandler);
    UART0_Init();
    
    UART0_OutString("M-Task 2: Bump Switch Obstacle Avoidance\r\n");
    UART0_OutString("Bump #2 -> Reverse + Turn Right 90 deg\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting...\r\n");
    Motor_Forward(BASE_SPEED, BASE_SPEED);
    
    while(1){
        if (bump_flag) {
            bump_flag = 0;
            
            UART0_OutString("\r\n*** BUMP #2 DETECTED ***\r\n");
            
            // Step 1: Reverse for 500ms
            UART0_OutString("Step 1: Reversing...\r\n");
            Motor_Backward(BASE_SPEED, BASE_SPEED);
            Clock_Delay1ms(500);
            Motor_Stop();
            Clock_Delay1ms(100);
            
            // Step 2: Turn 90 degrees right
            UART0_OutString("Step 2: Turning 90 deg right...\r\n");
            Motor_Right(TURN_SPEED, TURN_SPEED);
            Clock_Delay1ms(TURN_90_DEG_MS);
            Motor_Stop();
            Clock_Delay1ms(100);
            
            // Step 3: Continue forward
            avoid_count++;
            UART0_OutString("Step 3: Continuing forward\r\n");
            UART0_OutString("*** Obstacle Avoided ***\r\n");
            UART0_OutString("Total avoidances: ");
            UART0_OutUDec5(avoid_count);
            UART0_OutString("\r\n\r\n");
            
            Motor_Forward(BASE_SPEED, BASE_SPEED);
        }
        
        Clock_Delay1ms(10);
    }
}
