/*
 * L1_LED_Reflectance.c
 * L-Task Example 1: Blink RED LED only when black line is below reflectance sensor
 *
 * Description:
 * - Read center reflectance sensor
 * - Blink LED when black line detected (sensor reads 0)
 * - LED off when white surface detected (sensor reads 1)
 *
 *  Created on: 30 Oct 2025
 *      Author: nathan003
 */
#include "msp.h"
#include <stdint.h>
#include "..\..\inc\Clock.h"
#include "..\..\inc\LaunchPad.h"
#include "..\..\inc\Reflectance.h"
#include "..\..\inc\UART0.h"

void Blink(int time){
    LaunchPad_LED(1);
    Clock_Delay1ms(time);
    LaunchPad_LED(0);
    Clock_Delay1ms(time);
}

int main(){
    uint8_t rf_data;
    uint8_t center_sensors;
    uint32_t loop_count = 0;
    
    // Initialization
    Clock_Init48MHz();
    LaunchPad_Init();
    Reflectance_Init();
    UART0_Init();
    
    UART0_OutString("L-Task 1: LED Blink on Black Line Detection\r\n");
    UART0_OutString("Place center sensors over black line to blink LED\r\n");
    
    while(1){
        // Read reflectance sensors
        rf_data = Reflectance_Read(1000);
        
        // Extract center sensors (bits 3 and 4)
        center_sensors = (rf_data >> 3) & 0x03;
        
        // Check if black line detected (0 = dark line)
        if (center_sensors == 0x00) {
            // Both center sensors see black line - blink LED
            Blink(100);
            
            if (loop_count % 10 == 0) {
                UART0_OutString("BLACK LINE DETECTED - LED Blinking\r\n");
            }
        } else {
            // No black line - LED off
            LaunchPad_LED(0);
            
            if (loop_count % 100 == 0) {
                UART0_OutString("White surface - LED OFF\r\n");
            }
        }
        
        loop_count++;
        Clock_Delay1ms(50);
    }
}
