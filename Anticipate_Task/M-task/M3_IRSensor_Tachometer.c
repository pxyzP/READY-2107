/*
 * M3_IRSensor_Tachometer.c
 * M-Task Example 3: Monitor wheel speed difference
 *
 * Description:
 * - Move RSLK forward while monitoring IR sensors
 * - Use tachometer to check both wheel speeds
 * - If speed difference > 10%, stop and display difference
 * - Also stop if obstacle detected
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Motor.h"
#include "..\..\inc\Tachometer.h"
#include "..\..\inc\ADC14.h"
#include "..\..\inc\LPF.h"
#include "..\..\inc\IRDistance.h"
#include "..\..\inc\TimerA1.h"
#include "..\..\inc\UART0.h"

#define SPEED_DIFF_THRESHOLD 10  // 10% difference
#define OBSTACLE_THRESHOLD_MM 150

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
    uint16_t leftSpeed, rightSpeed;
    enum TachDirection leftDir, rightDir;
    int32_t leftSteps, rightSteps;
    int32_t speedDiff, percentDiff;
    uint32_t centerDist_mm;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Tachometer_Init();
    
    ADC0_InitSWTriggerCh17_12_16();
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    s = 256;
    LPF_Init(raw17, s);
    LPF_Init2(raw12, s);
    LPF_Init3(raw16, s);
    
    UART0_Init();
    TimerA1_Init(&SensorRead_ISR, 250);
    EnableInterrupts();
    
    UART0_OutString("M-Task 3: Wheel Speed Monitoring\r\n");
    UART0_OutString("Monitors: Speed difference & Obstacles\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting...\r\n");
    Motor_Forward(3000, 3000);
    
    while(1){
        // Get tachometer data
        Tachometer_Get(&leftSpeed, &leftDir, &leftSteps,
                       &rightSpeed, &rightDir, &rightSteps);
        
        // Get IR sensor distance
        centerDist_mm = CenterConvert(nc);
        
        // Calculate speed difference (lower period = faster)
        if (leftSpeed > 0 && rightSpeed > 0) {
            speedDiff = (int32_t)leftSpeed - (int32_t)rightSpeed;
            
            // Calculate percentage (use average as reference)
            uint16_t avgSpeed = (leftSpeed + rightSpeed) / 2;
            if (avgSpeed > 0) {
                percentDiff = (speedDiff * 100) / avgSpeed;
                if (percentDiff < 0) percentDiff = -percentDiff;
                
                // Check if difference exceeds threshold
                if (percentDiff > SPEED_DIFF_THRESHOLD) {
                    Motor_Stop();
                    UART0_OutString("\r\n*** SPEED IMBALANCE DETECTED ***\r\n");
                    UART0_OutString("Left Speed: ");
                    UART0_OutUDec5(leftSpeed);
                    UART0_OutString("\r\nRight Speed: ");
                    UART0_OutUDec5(rightSpeed);
                    UART0_OutString("\r\nDifference: ");
                    UART0_OutSDec(percentDiff);
                    UART0_OutString("%\r\n");
                    
                    if (speedDiff < 0) {
                        UART0_OutString("Left wheel FASTER\r\n");
                    } else {
                        UART0_OutString("Right wheel FASTER\r\n");
                    }
                    
                    // Wait for button to continue
                    while(LaunchPad_Input() == 0);
                    while(LaunchPad_Input());
                    Motor_Forward(3000, 3000);
                }
            }
        }
        
        // Check for obstacle
        if (centerDist_mm < OBSTACLE_THRESHOLD_MM && centerDist_mm > 0) {
            Motor_Stop();
            UART0_OutString("\r\n*** OBSTACLE at ");
            UART0_OutUDec5(centerDist_mm);
            UART0_OutString(" mm ***\r\n");
            while(1);
        }
        
        Clock_Delay1ms(100);
    }
}
