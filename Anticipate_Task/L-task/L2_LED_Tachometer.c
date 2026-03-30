/*
 * L2_LED_Tachometer.c
 * L-Task Example 2: Blink LED at different rates based on wheel speed
 *
 * Description:
 * - Fast blink when speed > 2000 (fast movement)
 * - Slow blink when speed < 1000 (slow movement)
 * - Solid LED when stopped
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

// Speed thresholds
#define SPEED_FAST_THRESHOLD 2000
#define SPEED_SLOW_THRESHOLD 1000

void BlinkFast(){
    LaunchPad_LED(1);
    Clock_Delay1ms(100);
    LaunchPad_LED(0);
    Clock_Delay1ms(100);
}

void BlinkSlow(){
    LaunchPad_LED(1);
    Clock_Delay1ms(500);
    LaunchPad_LED(0);
    Clock_Delay1ms(500);
}

int main(){
    uint16_t leftSpeed, rightSpeed;
    enum TachDirection leftDir, rightDir;
    int32_t leftSteps, rightSteps;
    uint16_t avgSpeed;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Tachometer_Init();
    UART0_Init();
    
    UART0_OutString("L-Task 2: LED Blink Rate Based on Speed\r\n");
    UART0_OutString("Fast blink = High speed\r\n");
    UART0_OutString("Slow blink = Low speed\r\n");
    UART0_OutString("Solid LED = Stopped\r\n");
    
    // Start moving forward
    Motor_Forward(3000, 3000);
    Clock_Delay1ms(2000);
    
    while(1){
        // Get tachometer data
        Tachometer_Get(&leftSpeed, &leftDir, &leftSteps,
                       &rightSpeed, &rightDir, &rightSteps);
        
        // Calculate average speed (lower value = faster)
        avgSpeed = (leftSpeed + rightSpeed) / 2;
        
        // Blink based on speed
        if (avgSpeed == 0 || leftDir == STOPPED) {
            // Stopped - solid LED
            LaunchPad_LED(1);
            UART0_OutString("STOPPED - LED Solid\r\n");
            Clock_Delay1ms(1000);
        } else if (avgSpeed < SPEED_SLOW_THRESHOLD) {
            // Fast movement - fast blink
            BlinkFast();
            UART0_OutString("FAST - Speed: ");
            UART0_OutUDec5(avgSpeed);
            UART0_OutString("\r\n");
        } else if (avgSpeed < SPEED_FAST_THRESHOLD) {
            // Slow movement - slow blink
            BlinkSlow();
            UART0_OutString("SLOW - Speed: ");
            UART0_OutUDec5(avgSpeed);
            UART0_OutString("\r\n");
        } else {
            // Very slow - solid on
            LaunchPad_LED(1);
            Clock_Delay1ms(1000);
        }
    }
}
