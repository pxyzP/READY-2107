/*
 * H4_Maze_Navigator.c
 * H-Task Example 4: Simple maze navigation
 *
 * Description:
 * - Navigate through maze using IR sensors and bump switches
 * - Strategy: Wall-following (keep right wall close)
 * - Use IR sensors to detect walls
 * - Bump switches as backup collision detection
 * - Stop when black line detected (parking zone)
 * - Display all navigation decisions
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
#include "..\..\inc\Bump.h"
#include "..\..\inc\ADC14.h"
#include "..\..\inc\LPF.h"
#include "..\..\inc\IRDistance.h"
#include "..\..\inc\TimerA1.h"
#include "..\..\inc\UART0.h"

#define WALL_THRESHOLD 200     // mm - wall detection
#define FRONT_THRESHOLD 150    // mm - front obstacle
#define BASE_SPEED 2000
#define TURN_90_MS 300

volatile uint32_t nr, nc, nl;
volatile uint8_t bump_detected = 0;

void SensorRead_ISR(void){
    uint32_t raw17, raw12, raw16;
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    nr = LPF_Calc(raw17);
    nc = LPF_Calc2(raw12);
    nl = LPF_Calc3(raw16);
}

void BumpHandler(){
    P4->IFG &= ~0xFF;
    uint8_t data = Bump_Read();
    if (data != 0x3F) {
        bump_detected = 1;
        Motor_Stop();
    }
}

typedef enum {
    NAV_FORWARD,
    NAV_TURN_LEFT,
    NAV_TURN_RIGHT,
    NAV_REVERSE,
    NAV_FINISHED
} NavState;

int main(){
    uint32_t raw17, raw12, raw16, s;
    uint32_t leftDist, centerDist, rightDist;
    uint8_t rf_data;
    NavState state = NAV_FORWARD;
    uint32_t move_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Reflectance_Init();
    Bump_Init(&BumpHandler);
    
    ADC0_InitSWTriggerCh17_12_16();
    ADC_In17_12_16(&raw17, &raw12, &raw16);
    s = 256;
    LPF_Init(raw17, s);
    LPF_Init2(raw12, s);
    LPF_Init3(raw16, s);
    
    UART0_Init();
    TimerA1_Init(&SensorRead_ISR, 250);
    EnableInterrupts();
    
    UART0_OutString("H-Task 4: Maze Navigator\r\n");
    UART0_OutString("Strategy: Right wall following\r\n");
    UART0_OutString("Stop: Black line (parking zone)\r\n\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting navigation...\r\n\r\n");
    
    while(state != NAV_FINISHED){
        // Get sensor data
        leftDist = LeftConvert(nl);
        centerDist = CenterConvert(nc);
        rightDist = RightConvert(nr);
        rf_data = Reflectance_Read(1000);
        
        // Check for parking zone (black line)
        if ((rf_data & 0x18) == 0x00) {  // Center sensors see black
            state = NAV_FINISHED;
            Motor_Stop();
            UART0_OutString("\r\n*** PARKING ZONE DETECTED ***\r\n");
            UART0_OutString("Mission Complete!\r\n");
            break;
        }
        
        // Handle bump collision
        if (bump_detected) {
            bump_detected = 0;
            UART0_OutString("! BUMP - Reversing\r\n");
            Motor_Backward(BASE_SPEED, BASE_SPEED);
            Clock_Delay1ms(500);
            Motor_Stop();
            state = NAV_TURN_LEFT;
            continue;
        }
        
        // Navigation logic - right wall following
        if (centerDist < FRONT_THRESHOLD && centerDist > 50) {
            // Wall ahead - turn left
            UART0_OutString("Wall ahead - Turn LEFT\r\n");
            Motor_Stop();
            Clock_Delay1ms(100);
            Motor_Left(BASE_SPEED, BASE_SPEED);
            Clock_Delay1ms(TURN_90_MS);
            Motor_Stop();
            Clock_Delay1ms(100);
            state = NAV_FORWARD;
        }
        else if (rightDist > WALL_THRESHOLD || rightDist < 50) {
            // No right wall - turn right to follow wall
            UART0_OutString("No right wall - Turn RIGHT\r\n");
            Motor_Stop();
            Clock_Delay1ms(100);
            Motor_Right(BASE_SPEED, BASE_SPEED);
            Clock_Delay1ms(TURN_90_MS);
            Motor_Stop();
            Clock_Delay1ms(100);
            state = NAV_FORWARD;
        }
        else {
            // Path clear - move forward
            Motor_Forward(BASE_SPEED, BASE_SPEED);
            
            if (move_count++ % 20 == 0) {
                UART0_OutString("Forward - L:");
                UART0_OutUDec5(leftDist);
                UART0_OutString(" C:");
                UART0_OutUDec5(centerDist);
                UART0_OutString(" R:");
                UART0_OutUDec5(rightDist);
                UART0_OutString(" mm\r\n");
            }
        }
        
        Clock_Delay1ms(100);
    }
    
    // Park - align to center
    UART0_OutString("Parking...\r\n");
    Motor_Stop();
    
    // Celebrate
    for (int i = 0; i < 5; i++) {
        LaunchPad_LED(1);
        Clock_Delay1ms(200);
        LaunchPad_LED(0);
        Clock_Delay1ms(200);
    }
    
    UART0_OutString("Robot parked successfully!\r\n");
    
    while(1);
}
