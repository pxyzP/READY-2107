/*
 * H2_Obstacle_Scanner.c
 * H-Task Example 2: Scan and find closest/furthest obstacles
 *
 * Description:
 * - Move forward and scan for obstacles using IR sensors
 * - When obstacle detected: stop, record distance, turn 60 deg
 * - After 4 obstacles detected, analyze and display:
 *   - Distance to each obstacle
 *   - Closest obstacle
 *   - Furthest obstacle
 * - Use manual min/max finding (NO library sort functions)
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

#define OBSTACLE_THRESHOLD_MM 300
#define NUM_OBSTACLES 4
#define TURN_60_DEG_MS 200

volatile uint32_t nr, nc, nl;

typedef struct {
    uint32_t distance_mm;
    uint8_t obstacle_num;
} ObstacleData;

ObstacleData obstacles[NUM_OBSTACLES];
uint8_t obstacle_count = 0;

void SensorRead_ISR(void){
    uint32_t raw17, raw12, raw16;
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    nr = LPF_Calc(raw17);
    nc = LPF_Calc2(raw12);
    nl = LPF_Calc3(raw16);
}

// Manual find minimum (NO library functions)
void FindClosestObstacle(uint8_t *index, uint32_t *minDist){
    *minDist = 0xFFFFFFFF;
    *index = 0;
    
    for (uint8_t i = 0; i < obstacle_count; i++) {
        if (obstacles[i].distance_mm < *minDist) {
            *minDist = obstacles[i].distance_mm;
            *index = i;
        }
    }
}

// Manual find maximum (NO library functions)
void FindFurthestObstacle(uint8_t *index, uint32_t *maxDist){
    *maxDist = 0;
    *index = 0;
    
    for (uint8_t i = 0; i < obstacle_count; i++) {
        if (obstacles[i].distance_mm > *maxDist) {
            *maxDist = obstacles[i].distance_mm;
            *index = i;
        }
    }
}

int main(){
    uint32_t raw17, raw12, raw16, s;
    uint32_t centerDist_mm;
    uint8_t closestIdx, furthestIdx;
    uint32_t closestDist, furthestDist;
    
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
    
    UART0_OutString("H-Task 2: Obstacle Scanner\r\n");
    UART0_OutString("Scanning for 4 obstacles...\r\n\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    Motor_Forward(2000, 2000);
    
    // Scanning phase
    while(obstacle_count < NUM_OBSTACLES){
        centerDist_mm = CenterConvert(nc);
        
        // Check if obstacle detected
        if (centerDist_mm < OBSTACLE_THRESHOLD_MM && centerDist_mm > 50) {
            // Stop
            Motor_Stop();
            
            // Record obstacle
            obstacles[obstacle_count].distance_mm = centerDist_mm;
            obstacles[obstacle_count].obstacle_num = obstacle_count + 1;
            
            UART0_OutString("Obstacle #");
            UART0_OutUDec5(obstacle_count + 1);
            UART0_OutString(" detected at ");
            UART0_OutUDec5(centerDist_mm);
            UART0_OutString(" mm\r\n");
            
            obstacle_count++;
            
            Clock_Delay1ms(500);
            
            // Turn 60 degrees
            UART0_OutString("Turning 60 degrees...\r\n\r\n");
            Motor_Right(2000, 2000);
            Clock_Delay1ms(TURN_60_DEG_MS);
            Motor_Stop();
            Clock_Delay1ms(500);
            
            // Continue if more obstacles needed
            if (obstacle_count < NUM_OBSTACLES) {
                Motor_Forward(2000, 2000);
            }
        }
        
        Clock_Delay1ms(50);
    }
    
    // Analysis phase
    Motor_Stop();
    UART0_OutString("\r\n========================================\r\n");
    UART0_OutString("SCAN COMPLETE - Analysis\r\n");
    UART0_OutString("========================================\r\n\r\n");
    
    // Display all obstacles
    UART0_OutString("All Detected Obstacles:\r\n");
    for (uint8_t i = 0; i < NUM_OBSTACLES; i++) {
        UART0_OutString("  Obstacle #");
        UART0_OutUDec5(obstacles[i].obstacle_num);
        UART0_OutString(": ");
        UART0_OutUDec5(obstacles[i].distance_mm);
        UART0_OutString(" mm\r\n");
    }
    
    // Find closest
    FindClosestObstacle(&closestIdx, &closestDist);
    UART0_OutString("\r\nCLOSEST Obstacle: #");
    UART0_OutUDec5(obstacles[closestIdx].obstacle_num);
    UART0_OutString(" at ");
    UART0_OutUDec5(closestDist);
    UART0_OutString(" mm\r\n");
    
    // Find furthest
    FindFurthestObstacle(&furthestIdx, &furthestDist);
    UART0_OutString("FURTHEST Obstacle: #");
    UART0_OutUDec5(obstacles[furthestIdx].obstacle_num);
    UART0_OutString(" at ");
    UART0_OutUDec5(furthestDist);
    UART0_OutString(" mm\r\n");
    
    UART0_OutString("========================================\r\n");
    
    // Blink to indicate completion
    while(1){
        LaunchPad_LED(1);
        Clock_Delay1ms(500);
        LaunchPad_LED(0);
        Clock_Delay1ms(500);
    }
}
