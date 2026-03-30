/*
 * M5_Obstacle_Avoidance.c
 * M-Task Example 5: Obstacle avoidance using IR sensors
 *
 * Description:
 * - Move forward while scanning with IR sensors
 * - If obstacle detected ahead: stop, turn right 90°, continue
 * - Display avoidance actions on UART
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Motor.h"
#include "..\..\inc\ADC14.h"
#include "..\..\inc\LPF.h"
#include "..\..\inc\IRDistance.h"
#include "..\..\inc\TimerA1.h"
#include "..\..\inc\UART0.h"

#define OBSTACLE_THRESHOLD 250  // mm
#define BASE_SPEED 2500
#define TURN_90_MS 300

volatile uint32_t nr, nc, nl;

void SensorRead_ISR(void){
    uint32_t raw17, raw12, raw16;
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    nr = LPF_Calc(raw17);
    nc = LPF_Calc2(raw12);
    nl = LPF_Calc3(raw16);
}

int main(){
    uint32_t raw17, raw12, raw16, s;
    uint32_t centerDist, leftDist, rightDist;
    uint32_t avoid_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    
    ADC0_InitSWTriggerCh17_12_16();
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    s = 256;
    LPF_Init(raw17, s);
    LPF_Init2(raw12, s);
    LPF_Init3(raw16, s);
    
    UART0_Init();
    TimerA1_Init(&SensorRead_ISR, 250);
    EnableInterrupts();
    
    UART0_OutString("M-Task 5: Obstacle Avoidance\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting navigation...\r\n");
    Motor_Forward(BASE_SPEED, BASE_SPEED);
    
    while(1){
        // Get distances
        centerDist = CenterConvert(nc);
        leftDist = LeftConvert(nl);
        rightDist = RightConvert(nr);
        
        // Check for obstacle ahead
        if (centerDist < OBSTACLE_THRESHOLD && centerDist > 50) {
            // Obstacle detected
            Motor_Stop();
            avoid_count++;
            
            UART0_OutString("\r\n*** Obstacle #");
            UART0_OutUDec5(avoid_count);
            UART0_OutString(" at ");
            UART0_OutUDec5(centerDist);
            UART0_OutString(" mm ***\r\n");
            
            Clock_Delay1ms(300);
            
            // Turn right 90 degrees
            UART0_OutString("Avoiding: Turn right 90 deg\r\n");
            Motor_Right(2000, 2000);
            Clock_Delay1ms(TURN_90_MS);
            Motor_Stop();
            Clock_Delay1ms(300);
            
            UART0_OutString("Continuing...\r\n\r\n");
            Motor_Forward(BASE_SPEED, BASE_SPEED);
        }
        
        Clock_Delay1ms(50);
    }
}
