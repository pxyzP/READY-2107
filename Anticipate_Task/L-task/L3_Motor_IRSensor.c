/*
 * L3_Motor_IRSensor.c
 * L-Task Example 3: Move forward and stop when obstacle detected
 *
 * Description:
 * - Move RSLK forward at constant speed
 * - Monitor center IR sensor
 * - Stop when obstacle within 200mm detected
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

// IR sensor threshold (adjust based on calibration)
#define OBSTACLE_THRESHOLD_MM 200

volatile uint32_t nr, nc, nl;
volatile uint32_t ADCflag = 0;

void SensorRead_ISR(void){
    uint32_t raw17, raw12, raw16;
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    nr = LPF_Calc(raw17);   // right
    nc = LPF_Calc2(raw12);  // center
    nl = LPF_Calc3(raw16);  // left
    ADCflag = 1;
}

int main(){
    uint32_t raw17, raw12, raw16;
    uint32_t centerDist_mm;
    uint32_t s;
    
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
    
    UART0_OutString("L-Task 3: Stop at Obstacle\r\n");
    UART0_OutString("Moving forward...\r\n");
    
    // Start moving
    Motor_Forward(2500, 2500);
    
    while(1){
        // Convert to millimeters
        centerDist_mm = CenterConvert(nc);
        
        // Check if obstacle detected
        if (centerDist_mm < OBSTACLE_THRESHOLD_MM && centerDist_mm > 0) {
            // Obstacle detected - stop
            Motor_Stop();
            LaunchPad_LED(1);
            
            UART0_OutString("OBSTACLE DETECTED at ");
            UART0_OutUDec5(centerDist_mm);
            UART0_OutString(" mm - STOPPED\r\n");
            
            // Wait and display distance
            while(1){
                centerDist_mm = CenterConvert(nc);
                UART0_OutString("Distance: ");
                UART0_OutUDec5(centerDist_mm);
                UART0_OutString(" mm\r\n");
                Clock_Delay1ms(500);
            }
        }
        
        // Print distance periodically
        static uint32_t count = 0;
        if (count++ % 50 == 0) {
            UART0_OutString("Distance: ");
            UART0_OutUDec5(centerDist_mm);
            UART0_OutString(" mm\r\n");
        }
        
        Clock_Delay1ms(10);
    }
}
