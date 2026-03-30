/*
 * M6_Tachometer_Distance.c
 * M-Task Example 6: Precise distance movement using tachometer
 *
 * Description:
 * - Move robot forward exactly X centimeters using tachometer
 * - 360 steps ≈ 220mm (one wheel rotation)
 * - Calculate steps needed, monitor with tachometer, stop when reached
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
#include "..\..\inc\UART0.h"

#define STEPS_PER_CM 16  // 360 steps / 22cm ≈ 16.4 steps/cm
#define TARGET_DISTANCE_CM 50  // Move 50 cm

int main(){
    uint16_t leftSpeed, rightSpeed;
    enum TachDirection leftDir, rightDir;
    int32_t leftSteps, rightSteps;
    int32_t targetSteps;
    int32_t avgSteps;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Tachometer_Init();
    UART0_Init();
    
    // Calculate target steps
    targetSteps = TARGET_DISTANCE_CM * STEPS_PER_CM;
    
    UART0_OutString("M-Task 6: Precise Distance Movement\r\n");
    UART0_OutString("Target: ");
    UART0_OutUDec5(TARGET_DISTANCE_CM);
    UART0_OutString(" cm (");
    UART0_OutUDec5(targetSteps);
    UART0_OutString(" steps)\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting movement...\r\n");
    
    // Start moving
    Motor_Forward(2500, 2500);
    
    while(1){
        // Get tachometer data
        Tachometer_Get(&leftSpeed, &leftDir, &leftSteps,
                       &rightSpeed, &rightDir, &rightSteps);
        
        // Calculate average steps
        avgSteps = (leftSteps + rightSteps) / 2;
        
        // Display progress every 100 steps
        if (avgSteps % 100 == 0 && avgSteps > 0) {
            UART0_OutString("Progress: ");
            UART0_OutUDec5(avgSteps);
            UART0_OutString(" / ");
            UART0_OutUDec5(targetSteps);
            UART0_OutString(" steps\r\n");
        }
        
        // Check if target reached
        if (avgSteps >= targetSteps) {
            Motor_Stop();
            
            UART0_OutString("\r\n*** Target Reached! ***\r\n");
            UART0_OutString("Left wheel: ");
            UART0_OutSDec(leftSteps);
            UART0_OutString(" steps\r\n");
            UART0_OutString("Right wheel: ");
            UART0_OutSDec(rightSteps);
            UART0_OutString(" steps\r\n");
            UART0_OutString("Average: ");
            UART0_OutUDec5(avgSteps);
            UART0_OutString(" steps\r\n");
            
            // Calculate actual distance
            uint32_t actualDist = avgSteps / STEPS_PER_CM;
            UART0_OutString("Distance traveled: ~");
            UART0_OutUDec5(actualDist);
            UART0_OutString(" cm\r\n");
            
            break;
        }
        
        Clock_Delay1ms(10);
    }
    
    // Blink to indicate completion
    while(1){
        LaunchPad_LED(1);
        Clock_Delay1ms(200);
        LaunchPad_LED(0);
        Clock_Delay1ms(200);
    }
}
