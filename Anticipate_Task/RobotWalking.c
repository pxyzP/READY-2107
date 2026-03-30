/*
 * Robot Navigation - Obstacle Detection and Navigation
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include <string.h>
#include "..\inc\UART0.h"
#include "..\inc\EUSCIA0.h"
#include "..\inc\FIFO0.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTickInts.h"
#include "..\inc\CortexM.h"
#include "..\inc\TimerA1.h"
#include "..\inc\Bump.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include "../inc/IRDistance.h"
#include "../inc/ADC14.h"
#include "../inc/LPF.h"
#include "..\inc\Reflectance.h"
#include "../inc/TA3InputCapture.h"
#include "../inc/Tachometer.h"

// Robot states
#define STATE_FORWARD 0
#define STATE_TURN 1
#define STATE_SCAN 2
#define STATE_GOTO_CLOSEST 3
#define STATE_RETURN_START 4
#define STATE_GOTO_FURTHEST 5
#define STATE_DONE 6

// Motor speeds
#define SPEED_NORMAL 3000
#define SPEED_TURN 2000

// Distance thresholds (adjust based on your IR sensors)
#define OBSTACLE_THRESHOLD 2000
#define TURN_ANGLE_60DEG 200  // ms to turn ~60 degrees (adjust by testing)

typedef struct {
    uint32_t distance;
    int16_t angle;
    uint8_t valid;
} Obstacle;

// Global variables
int cnt = 0;
int bump = 0;
uint8_t bump_data, rf_data, rf_center;
volatile uint32_t ADCvalue;
volatile uint32_t ADCflag;
volatile uint32_t nr, nc, nl;

// Navigation variables
uint8_t state = STATE_FORWARD;
Obstacle obstacles[4];
uint8_t obstacle_count = 0;
int16_t current_angle = 0;
uint8_t closest_idx = 0;
uint8_t furthest_idx = 0;

void SensorRead_ISR(void) {
    uint32_t raw17, raw12, raw16;
    P1OUT ^= 0x01;
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    nr = LPF_Calc(raw17);   // right
    nc = LPF_Calc2(raw12);  // center
    nl = LPF_Calc3(raw16);  // left
    ADCflag = 1;
    P1OUT ^= 0x01;
}

void Blink(int time) {
    LaunchPad_LED(1);
    Clock_Delay1ms(time);
    LaunchPad_LED(0);
    Clock_Delay1ms(time);
}

void BumpHandler() {
    P4->IFG &= ~0xFF;  // Clear interrupt flags
    bump_data = Bump_Read();
    if (bump_data != 0xFF) {  // Any bump detected
        bump = 1;
        Motor_Stop();
    }
}

void MoveForward(uint32_t duration_ms) {
    Motor_Forward(SPEED_NORMAL, SPEED_NORMAL);
    Clock_Delay1ms(duration_ms);
}

void TurnLeft(uint32_t duration_ms) {
    Motor_Left(SPEED_TURN, SPEED_TURN);
    Clock_Delay1ms(duration_ms);
    current_angle += 60;  // Approximate
}

void TurnRight(uint32_t duration_ms) {
    Motor_Right(SPEED_TURN, SPEED_TURN);
    Clock_Delay1ms(duration_ms);
    current_angle -= 60;  // Approximate
}

void Turn180() {
    Motor_Right(SPEED_TURN, SPEED_TURN);
    Clock_Delay1ms(TURN_ANGLE_60DEG * 3);  // 180 degrees
    Motor_Stop();
    current_angle += 180;
}

uint8_t DetectObstacle() {
    // Check if center IR sensor detects obstacle
    return (nc > OBSTACLE_THRESHOLD);
}

void RecordObstacle() {
    if (obstacle_count < 4) {
        obstacles[obstacle_count].distance = nc;
        obstacles[obstacle_count].angle = current_angle;
        obstacles[obstacle_count].valid = 1;
        
        UART0_OutString("Obstacle ");
        UART0_OutUDec5(obstacle_count + 1);
        UART0_OutString(" at angle ");
        UART0_OutUDec5(current_angle);
        UART0_OutString(" dist ");
        UART0_OutUDec5(nc);
        UART0_OutString("\r\n");
        
        obstacle_count++;
        Blink(100);
    }
}

void FindClosestAndFurthest() {
    uint32_t min_dist = 0xFFFFFFFF;
    uint32_t max_dist = 0;
    
    for (uint8_t i = 0; i < obstacle_count; i++) {
        if (obstacles[i].valid) {
            if (obstacles[i].distance < min_dist) {
                min_dist = obstacles[i].distance;
                closest_idx = i;
            }
            if (obstacles[i].distance > max_dist) {
                max_dist = obstacles[i].distance;
                furthest_idx = i;
            }
        }
    }
    
    UART0_OutString("Closest: #");
    UART0_OutUDec5(closest_idx + 1);
    UART0_OutString(" Furthest: #");
    UART0_OutUDec5(furthest_idx + 1);
    UART0_OutString("\r\n");
}

void NavigateToObstacle(uint8_t idx) {
    int16_t target_angle = obstacles[idx].angle;
    int16_t angle_diff = target_angle - current_angle;
    
    // Turn to face obstacle
    if (angle_diff > 0) {
        TurnLeft((angle_diff / 60) * TURN_ANGLE_60DEG);
    } else if (angle_diff < 0) {
        TurnRight((-angle_diff / 60) * TURN_ANGLE_60DEG);
    }
    
    Motor_Stop();
    Clock_Delay1ms(500);
    
    // Move forward until close to obstacle
    Motor_Forward(SPEED_NORMAL, SPEED_NORMAL);
    while (!DetectObstacle() && !bump) {
        Clock_Delay1ms(10);
    }
    Motor_Stop();
    
    UART0_OutString("Reached obstacle #");
    UART0_OutUDec5(idx + 1);
    UART0_OutString("\r\n");
    
    Blink(200);
    Blink(200);
}

int main() {
    uint32_t raw17, raw12, raw16;
    uint32_t s;

    // Initialization
    Clock_Init48MHz();
    Motor_Init();
    Motor_Stop();
    LaunchPad_Init();
    Reflectance_Init();
    Bump_Init(&BumpHandler);
    
    ADCflag = 0;
    s = 256;
    ADC0_InitSWTriggerCh17_12_16();
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    LPF_Init(raw17, s);
    LPF_Init2(raw12, s);
    LPF_Init3(raw16, s);
    
    UART0_Init();
    TimerA1_Init(&SensorRead_ISR, 250);
    EnableInterrupts();

    UART0_OutString("Robot Started\r\n");
    Clock_Delay1ms(2000);  // Wait 2 seconds before starting

    while (1) {
        switch (state) {
            case STATE_FORWARD:
                UART0_OutString("Moving forward...\r\n");
                
                if (DetectObstacle()) {
                    Motor_Stop();
                    state = STATE_TURN;
                    RecordObstacle();
                } else if (bump) {
                    Motor_Stop();
                    bump = 0;
                    state = STATE_TURN;
                } else {
                    MoveForward(100);
                }
                
                if (obstacle_count >= 4) {
                    Motor_Stop();
                    state = STATE_SCAN;
                }
                break;

            case STATE_TURN:
                UART0_OutString("Turning 60 degrees...\r\n");
                TurnRight(TURN_ANGLE_60DEG);
                Motor_Stop();
                Clock_Delay1ms(500);
                state = STATE_FORWARD;
                break;

            case STATE_SCAN:
                UART0_OutString("Scan complete. Analyzing...\r\n");
                FindClosestAndFurthest();
                Clock_Delay1ms(1000);
                state = STATE_GOTO_CLOSEST;
                break;

            case STATE_GOTO_CLOSEST:
                UART0_OutString("Going to closest obstacle...\r\n");
                NavigateToObstacle(closest_idx);
                Clock_Delay1ms(1000);
                state = STATE_RETURN_START;
                break;

            case STATE_RETURN_START:
                UART0_OutString("Returning to start...\r\n");
                Turn180();
                MoveForward(2000);  // Move back (adjust based on distance)
                Motor_Stop();
                Clock_Delay1ms(1000);
                state = STATE_GOTO_FURTHEST;
                break;

            case STATE_GOTO_FURTHEST:
                UART0_OutString("Going to furthest obstacle...\r\n");
                NavigateToObstacle(furthest_idx);
                Clock_Delay1ms(1000);
                Turn180();
                MoveForward(2000);  // Return
                Motor_Stop();
                state = STATE_DONE;
                break;

            case STATE_DONE:
                UART0_OutString("Mission complete!\r\n");
                Blink(100);
                Blink(100);
                Blink(100);
                Clock_Delay1ms(5000);
                break;
        }
        
        Clock_Delay1ms(10);
    }
}
