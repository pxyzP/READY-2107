/*
 * Task_M.c
 * M Task - Blink LED based on bump switch press count
 *
 * Features:
 * - Press any of switches 1-5 (bump 0-4) n times total
 * - When switch 6 (bump 5) is pressed, blink LED n times
 * - Reset counter after blinking
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\inc\Clock.h"
#include "..\inc\Motor.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Bump.h"
#include "..\inc\UART0.h"

// Global variables
volatile uint32_t press_count = 0;  // Count of switches 1-5 presses
volatile uint8_t bump_data = 0xFF;
volatile uint8_t trigger_blink = 0;

void Blink(int time){
    LaunchPad_LED(1);
    Clock_Delay1ms(time);
    LaunchPad_LED(0);
    Clock_Delay1ms(time);
}

void BumpHandler(){
    P4->IFG &= ~0xFF;  // Clear interrupt flags
    bump_data = Bump_Read();
    
    // Check which switches are pressed
    // Bump sensors are negative logic (0 = pressed, 1 = not pressed)
    // bit 5 = Bump5 (switch 6)
    // bit 4 = Bump4 (switch 5)
    // bit 3 = Bump3 (switch 4)
    // bit 2 = Bump2 (switch 3)
    // bit 1 = Bump1 (switch 2)
    // bit 0 = Bump0 (switch 1)
    
    // Check if switch 6 (Bump5, bit 5) is pressed
    if (!(bump_data & (1 << 5))) {
        // Switch 6 pressed - trigger blink sequence
        trigger_blink = 1;
    } else {
        // Check switches 1-5 (bits 0-4)
        // Count how many are pressed
        for (uint8_t i = 0; i < 5; i++) {
            if (!(bump_data & (1 << i))) {
                press_count++;
                UART0_OutString("Switch ");
                UART0_OutUDec5(i + 1);
                UART0_OutString(" pressed. Total count: ");
                UART0_OutUDec5(press_count);
                UART0_OutString("\r\n");
            }
        }
    }
}

void BlinkNTimes(uint32_t n) {
    UART0_OutString("Blinking ");
    UART0_OutUDec5(n);
    UART0_OutString(" times...\r\n");
    
    for (uint32_t i = 0; i < n; i++) {
        Blink(300);  // 300ms on, 300ms off
        Clock_Delay1ms(200);  // Extra delay between blinks
    }
    
    UART0_OutString("Blink sequence complete!\r\n");
}

void TimedPause(uint32_t time){
    Clock_Delay1ms(time);
    UART0_OutString("Press button to start...\r\n");
    while(LaunchPad_Input()==0);
    while(LaunchPad_Input());
}

int main(){
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Bump_Init(&BumpHandler);
    UART0_Init();
    
    UART0_OutString("========================================\r\n");
    UART0_OutString("Task M - Bump Switch Counter\r\n");
    UART0_OutString("========================================\r\n");
    UART0_OutString("Instructions:\r\n");
    UART0_OutString("1. Press switches 1-5 multiple times\r\n");
    UART0_OutString("2. Press switch 6 to blink LED\r\n");
    UART0_OutString("3. LED will blink n times (n = total presses)\r\n");
    UART0_OutString("4. Counter resets after blinking\r\n");
    UART0_OutString("========================================\r\n");
    
    // Wait for button press
    TimedPause(1000);
    
    // Start indication
    UART0_OutString("System ready. Press bump switches!\r\n");
    Blink(100);
    Blink(100);
    
    // Reset counter
    press_count = 0;
    
    while(1){
        // Check if switch 6 was pressed
        if (trigger_blink) {
            trigger_blink = 0;
            
            UART0_OutString("\r\n=== Switch 6 Pressed! ===\r\n");
            UART0_OutString("Total presses counted: ");
            UART0_OutUDec5(press_count);
            UART0_OutString("\r\n");
            
            if (press_count > 0) {
                // Blink n times
                BlinkNTimes(press_count);
                
                // Reset counter
                press_count = 0;
                UART0_OutString("Counter reset to 0\r\n");
            } else {
                UART0_OutString("No presses counted yet!\r\n");
            }
            
            UART0_OutString("=========================\r\n\r\n");
        }
        
        // Small delay to prevent busy waiting
        Clock_Delay1ms(50);
    }
}
