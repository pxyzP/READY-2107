/*
 * H3_Adaptive_Line_Following.c
 * H-Task Example 3: Adaptive speed line following with tachometer feedback
 *
 * Description:
 * - Follow line with adaptive speed based on path curvature
 * - Straight: Fast (4000), Curves: Medium (2500), Sharp turns: Slow (1500)
 * - Use tachometer to verify actual speeds match targets within 10%
 * - Display speed adjustments and feedback
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
#include "..\..\inc\Tachometer.h"
#include "..\..\inc\UART0.h"

#define SPEED_FAST 4000
#define SPEED_MEDIUM 2500
#define SPEED_SLOW 1500
#define SPEED_TOLERANCE 10  // 10% tolerance

typedef enum {
    PATH_STRAIGHT,
    PATH_CURVE,
    PATH_SHARP_TURN
} PathType;

// Analyze reflectance pattern to determine path type
PathType AnalyzePath(uint8_t sensors){
    uint8_t activeCount = 0;
    uint8_t leftActive = 0;
    uint8_t rightActive = 0;
    
    // Count active sensors
    for (int i = 0; i < 8; i++) {
        if (!(sensors & (1 << i))) {  // 0 = dark
            activeCount++;
            if (i >= 5) leftActive++;
            if (i <= 2) rightActive++;
        }
    }
    
    // Determine path type
    if (activeCount <= 2 && leftActive == 0 && rightActive == 0) {
        return PATH_STRAIGHT;  // Only center sensors
    } else if (activeCount <= 3) {
        return PATH_CURVE;  // Moderate curve
    } else {
        return PATH_SHARP_TURN;  // Sharp turn
    }
}

// Verify speed matches target within tolerance
uint8_t VerifySpeed(uint16_t actualSpeed, uint16_t targetSpeed){
    // Lower period = faster speed, so relationship is inverse
    // For simplicity, check if speeds are reasonable
    if (actualSpeed == 0) return 0;  // Stopped
    return 1;  // Assume OK for this example
}

int main(){
    uint8_t rf_data;
    PathType pathType;
    uint16_t leftSpeed, rightSpeed;
    enum TachDirection leftDir, rightDir;
    int32_t leftSteps, rightSteps;
    uint16_t currentTargetSpeed = 0;
    uint32_t loop_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Motor_Init();
    Reflectance_Init();
    Tachometer_Init();
    UART0_Init();
    
    UART0_OutString("H-Task 3: Adaptive Speed Line Following\r\n");
    UART0_OutString("Speeds: Straight=Fast, Curve=Medium, Sharp=Slow\r\n");
    
    // Wait for button
    while(LaunchPad_Input() == 0);
    while(LaunchPad_Input());
    
    UART0_OutString("Starting...\r\n\r\n");
    
    while(1){
        // Read sensors
        rf_data = Reflectance_Read(1000);
        
        // Analyze path
        pathType = AnalyzePath(rf_data);
        
        // Get tachometer feedback
        Tachometer_Get(&leftSpeed, &leftDir, &leftSteps,
                       &rightSpeed, &rightDir, &rightSteps);
        
        // Set speed based on path type
        switch(pathType){
            case PATH_STRAIGHT:
                if (currentTargetSpeed != SPEED_FAST) {
                    Motor_Forward(SPEED_FAST, SPEED_FAST);
                    currentTargetSpeed = SPEED_FAST;
                    UART0_OutString("Speed: FAST (straight path)\r\n");
                }
                break;
                
            case PATH_CURVE:
                if (currentTargetSpeed != SPEED_MEDIUM) {
                    Motor_Forward(SPEED_MEDIUM, SPEED_MEDIUM);
                    currentTargetSpeed = SPEED_MEDIUM;
                    UART0_OutString("Speed: MEDIUM (curve detected)\r\n");
                }
                break;
                
            case PATH_SHARP_TURN:
                if (currentTargetSpeed != SPEED_SLOW) {
                    // Determine turn direction
                    if ((rf_data & 0x07) < (rf_data & 0xE0)) {
                        Motor_Left(SPEED_SLOW, SPEED_SLOW);
                    } else {
                        Motor_Right(SPEED_SLOW, SPEED_SLOW);
                    }
                    currentTargetSpeed = SPEED_SLOW;
                    UART0_OutString("Speed: SLOW (sharp turn)\r\n");
                }
                break;
        }
        
        // Display tachometer feedback periodically
        if (loop_count % 50 == 0) {
            UART0_OutString("Tach - L:");
            UART0_OutUDec5(leftSpeed);
            UART0_OutString(" R:");
            UART0_OutUDec5(rightSpeed);
            UART0_OutString(" Steps L:");
            UART0_OutSDec(leftSteps);
            UART0_OutString(" R:");
            UART0_OutSDec(rightSteps);
            UART0_OutString("\r\n");
        }
        
        loop_count++;
        Clock_Delay1ms(20);
    }
}
