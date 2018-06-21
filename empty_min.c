/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty_min.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
// #include <ti/drivers/I2C.h>
#include <ti/drivers/PIN.h>
// #include <ti/drivers/SPI.h>
 #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>

/* For usleep() */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Board Header files */
#include "Board.h"

#define TASKSTACKSIZE   512

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State ledPinState;
static PIN_Handle buttonPinHandle;
static PIN_State buttonPinState;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config ledPinTable[] = {
    Board_RESET | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_SDA | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_SCL | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_CS | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_DISPLAY | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_DC | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

PIN_Config buttonPinTable[] = {
    Board_BUSY | PIN_INPUT_EN  | PIN_PULLUP  | PIN_HYSTERESIS,
    PIN_TERMINATE
};


uint32_t bus = 0;
uint32_t bus1 = 0;

char input;
char output;
char echoPrompt[] = "Echoing characters: ";
UART_Handle uart;
UART_Params uartParams;

#define bit unsigned char

#define uchar  unsigned char
#define uint  unsigned int

#define DELAY_TIME  2                                   // DELAY TIME

#define MODE1                                           // panel scan direction

#define PIC_BLACK       252
#define PIC_WHITE       255
#define PIC_A           1
#define PIC_B           2
#define PIC_C           3
#define PIC_HLINE       4
#define PIC_VLINE       5
#define PIC_R           6
#define PIC_D           7
#define PIC_E           8
#define PIC_F           9

const unsigned char gImage_mb_bw[2756] = { 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XF5, 0X55, 0X5F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XEA, 0XAA,
                                           0XAF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF5,
                                           0X55, 0X5F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XEA, 0XAA, 0XAF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XF5, 0X55, 0X5F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XEA, 0XAA, 0XAF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XF5, 0X55, 0X5F,
                                           0XF0, 0X00, 0X00, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XEA, 0XAA,
                                           0XAF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF5,
                                           0X55, 0X5F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XEA, 0XAA, 0XAF, 0XF0, 0X00, 0X00,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XF5, 0X55, 0X5F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XEA, 0XAA, 0XAF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XF5, 0X55, 0X5F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XEA, 0XAA,
                                           0XAF, 0XF0, 0X00, 0X00, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF5,
                                           0X55, 0X5F, 0XF0, 0X00, 0X00, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XEA, 0XAA, 0XAF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XF5, 0X55, 0X5F, 0XFF, 0XFF,
                                           0XFF, 0XFB, 0X8B, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XEA, 0XAA, 0XAF, 0XFF,
                                           0XFF, 0XFF, 0XFA, 0X99, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFD, 0X9B, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XF0, 0X00, 0X00, 0X7E, 0X27,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XF0, 0X00, 0X00, 0X7F,
                                           0XB1, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFB, 0X17, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XF0, 0X00,
                                           0X00, 0X78, 0XB7, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XF0,
                                           0X00, 0X00, 0X7E, 0XBD, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XF0, 0X00, 0X00, 0X7D, 0X83, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFD, 0XB7,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFE, 0X00, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XF3, 0XFF, 0XF0, 0X00, 0X1F,
                                           0XFE, 0X00, 0X00, 0X1F, 0XF0, 0X00,
                                           0X00, 0X7F, 0XCF, 0XFF, 0XE0, 0X00,
                                           0X0F, 0XFE, 0X00, 0X00, 0X1F, 0XF0,
                                           0X00, 0X00, 0X7F, 0X3F, 0XFF, 0XC0,
                                           0X00, 0X07, 0XFE, 0X00, 0X00, 0X1F,
                                           0XF0, 0X00, 0X00, 0X7C, 0XFF, 0XFF,
                                           0X81, 0XFF, 0X03, 0XFE, 0X00, 0X00,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X87, 0XFF, 0XC3, 0XFE, 0X00,
                                           0X00, 0X1F, 0XFF, 0XFF, 0XFF, 0XF8,
                                           0XDF, 0XFF, 0X87, 0XFF, 0XC3, 0XFE,
                                           0X10, 0X80, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7B, 0XDB, 0XFF, 0X87, 0XFF, 0XC3,
                                           0XFE, 0X30, 0XB9, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFB, 0XDB, 0XFF, 0X87, 0XFF,
                                           0XC3, 0XFE, 0X10, 0XA1, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFC, 0X1B, 0XFF, 0X81,
                                           0XFF, 0X03, 0XFE, 0X09, 0XA1, 0X9F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XDB, 0XFF,
                                           0XC0, 0X00, 0X07, 0XFE, 0X0A, 0X91,
                                           0X5F, 0XF0, 0X00, 0X00, 0X7F, 0XDB,
                                           0XFF, 0XE0, 0X00, 0X0F, 0XFE, 0X04,
                                           0X89, 0X1F, 0XF0, 0X00, 0X00, 0X7E,
                                           0X1B, 0XFF, 0XF0, 0X00, 0X1F, 0XFE,
                                           0X02, 0X86, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7D, 0XDB, 0XFF, 0XFC, 0X00, 0XFF,
                                           0XFE, 0X09, 0XE3, 0XDF, 0XF0, 0X00,
                                           0X00, 0X7B, 0XDF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFE, 0X10, 0X92, 0X1F, 0XF0,
                                           0X00, 0X00, 0X7B, 0XDF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFE, 0X20, 0X82, 0X1F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFE, 0X3F, 0X82,
                                           0X1F, 0XF0, 0X00, 0X00, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFE, 0X02,
                                           0X80, 0XDF, 0XF0, 0X00, 0X00, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                           0X04, 0XBF, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFE, 0X04, 0X82, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFE, 0X08, 0X84, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X80,
                                           0X00, 0X03, 0XFE, 0X08, 0X88, 0X1F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X80, 0X00, 0X03, 0XFE, 0X00, 0X00,
                                           0X1F, 0XF0, 0X00, 0X00, 0X7F, 0XFF,
                                           0XFF, 0X80, 0X00, 0X03, 0XFE, 0X18,
                                           0X10, 0X1F, 0XF0, 0X00, 0X00, 0X7F,
                                           0XFF, 0XFF, 0X80, 0X00, 0X07, 0XFE,
                                           0X24, 0XF0, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0X0F,
                                           0XFE, 0X25, 0X10, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                           0X1F, 0XFE, 0X25, 0X10, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFC, 0X3F, 0XFE, 0X25, 0X10, 0X1F,
                                           0XF0, 0X00, 0X00, 0X78, 0X7F, 0XFF,
                                           0XFF, 0XFC, 0X3F, 0XFE, 0X25, 0X10,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XF8, 0X7F,
                                           0XFF, 0XFF, 0XF8, 0X7F, 0XFE, 0X2A,
                                           0XE0, 0X1F, 0XFF, 0XFF, 0XFF, 0XF8,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                           0X18, 0X00, 0X1F, 0XF0, 0X00, 0X00,
                                           0X78, 0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFE, 0X00, 0X00, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFE, 0X00, 0X00, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFE, 0X01, 0XFC, 0X1F,
                                           0XF0, 0X00, 0X00, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFE, 0X06, 0X03,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFC,
                                           0X03, 0XFF, 0XFF, 0XFF, 0XFE, 0X08,
                                           0X00, 0X9F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XE0, 0X00, 0X7F, 0XFF, 0XFF, 0XFE,
                                           0X08, 0X00, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X80, 0X00, 0X3F, 0XFF, 0XFF,
                                           0XFE, 0X08, 0X00, 0X9F, 0XF0, 0X00,
                                           0X00, 0X7F, 0X00, 0X00, 0X0F, 0XFF,
                                           0XFF, 0XFE, 0X08, 0X00, 0X9F, 0XF0,
                                           0X00, 0X00, 0X7E, 0X00, 0X00, 0X07,
                                           0XFF, 0XFF, 0XFE, 0X06, 0X03, 0X1F,
                                           0XF0, 0X00, 0X00, 0X7C, 0X00, 0X00,
                                           0X07, 0XFF, 0XFF, 0XFE, 0X01, 0XFC,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XFC, 0X00,
                                           0X00, 0X03, 0XFF, 0XFF, 0XFE, 0X00,
                                           0X00, 0X1F, 0XFF, 0XFF, 0XFF, 0XF8,
                                           0X03, 0XF8, 0X03, 0XFF, 0XFF, 0XFE,
                                           0X00, 0X00, 0X1F, 0XFF, 0XFF, 0XFF,
                                           0XF8, 0X07, 0XFC, 0X01, 0XFF, 0XFF,
                                           0XFE, 0X00, 0X00, 0X1F, 0XF0, 0X00,
                                           0X00, 0X70, 0X0F, 0XFE, 0X01, 0XFF,
                                           0XFF, 0XFE, 0X0E, 0X0F, 0X1F, 0XF0,
                                           0X00, 0X00, 0X70, 0X1F, 0XFF, 0X01,
                                           0XFF, 0XFF, 0XFE, 0X08, 0X30, 0X9F,
                                           0XFF, 0XFF, 0XFF, 0XF0, 0X1F, 0XFF,
                                           0X01, 0XFF, 0XFF, 0XFE, 0X08, 0X40,
                                           0X9F, 0XF0, 0X00, 0X00, 0X70, 0X1F,
                                           0XFF, 0X01, 0XFF, 0XFF, 0XFE, 0X08,
                                           0X80, 0X9F, 0XF0, 0X00, 0X00, 0X70,
                                           0X1F, 0XFF, 0X01, 0XFF, 0XFF, 0XFE,
                                           0X09, 0X00, 0X9F, 0XF0, 0X00, 0X00,
                                           0X70, 0X1F, 0XFF, 0X03, 0XF7, 0XFF,
                                           0XFE, 0X0A, 0X00, 0X9F, 0XF0, 0X00,
                                           0X00, 0X70, 0X0F, 0XFE, 0X02, 0X07,
                                           0XFF, 0XFE, 0X0C, 0X07, 0X1F, 0XF0,
                                           0X00, 0X00, 0X78, 0X07, 0XFE, 0X00,
                                           0X07, 0XFF, 0XFE, 0X00, 0X00, 0X1F,
                                           0XF0, 0X00, 0X00, 0X78, 0X01, 0XFC,
                                           0X00, 0X07, 0XFF, 0XFE, 0X00, 0X00,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XF8, 0X00,
                                           0X78, 0X00, 0X07, 0XFF, 0XFE, 0X00,
                                           0X00, 0X1F, 0XFF, 0XFF, 0XFF, 0XFC,
                                           0X00, 0X78, 0X00, 0X07, 0XFF, 0XFE,
                                           0X00, 0X00, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7E, 0X00, 0X78, 0X00, 0X07, 0XFF,
                                           0XFE, 0X08, 0X00, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X00, 0X7C, 0X00, 0X0F,
                                           0XFF, 0XFE, 0X08, 0X00, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X80, 0XFC, 0X01,
                                           0XFF, 0XFF, 0XFE, 0X0F, 0XFF, 0X9F,
                                           0XF0, 0X00, 0X00, 0X7F, 0XE0, 0XFC,
                                           0X7F, 0XFF, 0XFF, 0XFE, 0X08, 0X01,
                                           0X1F, 0XF0, 0X00, 0X00, 0X7F, 0XF8,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFE, 0X08,
                                           0X01, 0X1F, 0XF0, 0X00, 0X00, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                           0X00, 0X00, 0X1F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X00, 0X00, 0X1F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                           0X1F, 0XF0, 0X00, 0X00, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X3E,
                                           0X00, 0X1F, 0XF0, 0X00, 0X00, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X20, 0X00, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X20, 0XFF, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X21, 0X01, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X22, 0X01, 0X1F,
                                           0XF0, 0X00, 0X00, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X21, 0X01,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X20,
                                           0XF9, 0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X20, 0X89, 0X1F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X20, 0X89, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X20, 0X89, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X20, 0X89, 0X1F,
                                           0XF0, 0X00, 0X00, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X20, 0X89,
                                           0XDF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X1F,
                                           0XFE, 0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X00, 0X08, 0X1F, 0XF0, 0X00, 0X00,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X00, 0X10, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X00, 0X20, 0X1F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X00, 0X00, 0X1F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                           0X00, 0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X00, 0X04, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X3F, 0XFE, 0XDF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X10, 0X04, 0X9F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X10, 0X04, 0X9F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X10, 0X04,
                                           0X9F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X10,
                                           0X04, 0X9F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X1F, 0XFC, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X11, 0X24, 0X9F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X11, 0X25, 0X9F, 0XF2,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X1F, 0XFE, 0X9F,
                                           0XED, 0XBF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X10, 0X04,
                                           0X9F, 0XED, 0XBF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X10,
                                           0X04, 0X9F, 0XF2, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X10, 0X04, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X3F, 0XFC, 0X9F, 0XF2, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X00, 0X00, 0X9F, 0XED,
                                           0XBF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X00, 0X00, 0X9F,
                                           0XED, 0XBF, 0XFF, 0XFF, 0XFB, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                           0X1F, 0XF7, 0X7F, 0XFF, 0XFF, 0XFB,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XF1, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XF0, 0X7F, 0XFF,
                                           0XFF, 0XF1, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0XBF,
                                           0XFF, 0XFF, 0XEA, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XEF,
                                           0XBF, 0XFF, 0XFF, 0XFB, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XF0, 0X7F, 0XFF, 0XFF, 0XFB, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFB,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XEF, 0XFF, 0XFF, 0XFF,
                                           0XFB, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XE0, 0X3F, 0XFF,
                                           0XFF, 0XFB, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XEF, 0X7F,
                                           0XFF, 0XFF, 0XFB, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFB, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XF2, 0X7F, 0XFF, 0XFF, 0XFB,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XED, 0XBF, 0XFF, 0XFF,
                                           0X05, 0XFF, 0XFF, 0XFF, 0XFC, 0X1F,
                                           0XFF, 0XFF, 0XFF, 0XED, 0XBF, 0XFF,
                                           0XFF, 0XFE, 0XFF, 0XFF, 0XFF, 0XFB,
                                           0X6F, 0XFF, 0XFF, 0XFF, 0XF2, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0X3F, 0XFF, 0XFF,
                                           0XFB, 0X6F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X86, 0XFF, 0XFF,
                                           0XFF, 0XFD, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XEF, 0XFF, 0XFF, 0XFF, 0X7D, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XE0, 0X3F, 0XFF, 0XFF, 0XFB,
                                           0X3F, 0XFF, 0XFF, 0XFC, 0XDF, 0XFF,
                                           0XFF, 0XFF, 0XEF, 0X7F, 0XFF, 0XFF,
                                           0X00, 0XFF, 0XFF, 0XFF, 0XFB, 0X6F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFD, 0XFF, 0XFF, 0XFF, 0XFB,
                                           0X6F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFC, 0X1F, 0XFF, 0XFF, 0XFF, 0XF3,
                                           0X7F, 0XFF, 0XFF, 0XD5, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XED, 0XBF, 0XFF, 0XFF, 0X15, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XCF, 0XFF, 0XFF,
                                           0XFF, 0XED, 0XBF, 0XFF, 0XFF, 0X55,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0X2F, 0XFF,
                                           0XFF, 0XFF, 0XF0, 0X7F, 0XFF, 0XFF,
                                           0X40, 0X3F, 0XFF, 0XFF, 0XF8, 0XEF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0X55, 0X7F, 0XFF, 0XFF, 0XFF,
                                           0XEF, 0XFF, 0XFF, 0XFF, 0XEF, 0XFF,
                                           0XFF, 0XFF, 0X55, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XE0,
                                           0X3F, 0XFF, 0XFF, 0X55, 0X7F, 0XFF,
                                           0XFF, 0XFC, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XEF, 0X7F, 0XFF, 0XFF, 0X00, 0X3F,
                                           0XFF, 0XFF, 0XFB, 0X6F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFC,
                                           0XFF, 0XFF, 0XFF, 0XFB, 0X6F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFB, 0XFF, 0XFF, 0XFF, 0XFD, 0XDF,
                                           0XFF, 0XFF, 0XFF, 0XEE, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XED, 0XBF,
                                           0XFF, 0XFF, 0XEC, 0XFF, 0XFF, 0XFF,
                                           0XFC, 0X9F, 0XFF, 0XFF, 0XFF, 0XEB,
                                           0XBF, 0XFF, 0XFF, 0XEA, 0XBF, 0XFF,
                                           0XFF, 0XFB, 0X6F, 0XFF, 0XFF, 0XFF,
                                           0XE7, 0X7F, 0XFF, 0XFF, 0X6A, 0XBF,
                                           0XFF, 0XFF, 0XFB, 0X6F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0X56,
                                           0XBF, 0XFF, 0XFF, 0XFD, 0XDF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X9F, 0XFF, 0XFF,
                                           0X16, 0XBF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFE, 0X7F, 0XFF,
                                           0XFF, 0X48, 0X3F, 0XFF, 0XFF, 0XFC,
                                           0X1F, 0XFF, 0XFF, 0XFF, 0XF9, 0XFF,
                                           0XFF, 0XFF, 0X56, 0XBF, 0XFF, 0XFF,
                                           0XFB, 0XEF, 0XFF, 0XFF, 0XFF, 0XE7,
                                           0XFF, 0XFF, 0XFF, 0XEA, 0XBF, 0XFF,
                                           0XFF, 0XFB, 0XEF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XEA, 0XBF,
                                           0XFF, 0XFF, 0XFC, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XF2, 0X7F, 0XFF, 0XFF, 0XEC,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XCF,
                                           0XFF, 0X7F, 0XED, 0XBF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XCF,
                                           0XEC, 0X03, 0X3F, 0XED, 0XBF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0X2F, 0XF7, 0XFB, 0X7F, 0XF7, 0X7F,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XF8, 0XEF, 0XFB, 0XFB, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XEF, 0XFC, 0X0A, 0X7F,
                                           0XEF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFB, 0XF9,
                                           0X7F, 0XE0, 0X3F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFB, 0X9F, 0XF7,
                                           0XFB, 0X7F, 0XEF, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFB, 0X6F,
                                           0XEC, 0X03, 0X7F, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFA,
                                           0XEF, 0XDF, 0XFD, 0X7F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XF9, 0XDF, 0XDB, 0X05, 0XFF, 0XFF,
                                           0X9F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XF7, 0X7D, 0XFF,
                                           0XFE, 0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFB, 0X9F, 0XEF, 0X7D,
                                           0XFF, 0XF9, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFB, 0X6F, 0XE0,
                                           0X00, 0X3F, 0XE7, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFA, 0XEF,
                                           0XFF, 0X7D, 0XFF, 0XFF, 0XF8, 0X0D,
                                           0XFF, 0X7F, 0X9C, 0XBF, 0XFF, 0XF9,
                                           0XDF, 0XFF, 0X7D, 0XFF, 0XF3, 0XBB,
                                           0XED, 0XE1, 0X4F, 0XAE, 0XBF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0X05, 0XFF, 0XED,
                                           0XBB, 0XED, 0XF5, 0XAF, 0XAA, 0X1E,
                                           0X07, 0XFB, 0X9F, 0XFF, 0XFF, 0XFF,
                                           0XED, 0XB8, 0X0D, 0XF5, 0XAF, 0XCA,
                                           0XBD, 0X77, 0XFB, 0X6F, 0XFF, 0XFF,
                                           0XFF, 0XEC, 0X3A, 0XA9, 0XE1, 0X47,
                                           0XEA, 0X3D, 0X77, 0XFA, 0XEF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XF8, 0X05, 0XFF,
                                           0X5F, 0XCA, 0XBF, 0X8F, 0XF9, 0XDF,
                                           0XFF, 0X7F, 0XFF, 0XF0, 0X7B, 0XED,
                                           0XFE, 0XDF, 0XAA, 0X1F, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X7F, 0X7F, 0XEF, 0XBB,
                                           0XED, 0XE0, 0X07, 0XAE, 0XBF, 0X83,
                                           0XFC, 0X1F, 0XC1, 0X7D, 0X7F, 0XEF,
                                           0XB8, 0X0D, 0XFE, 0X5F, 0XAC, 0XBF,
                                           0X7D, 0XFB, 0XEF, 0XED, 0X45, 0X7F,
                                           0XF0, 0X7F, 0XFF, 0XFD, 0XDF, 0XFF,
                                           0XBF, 0X7D, 0XFB, 0XEF, 0XED, 0X55,
                                           0X7F, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0X83, 0XFC, 0X1F, 0XED,
                                           0X15, 0X7F, 0XFF, 0XFF, 0X57, 0XFF,
                                           0X7F, 0XDF, 0X7F, 0XFF, 0XFF, 0XFF,
                                           0XC1, 0X55, 0X7F, 0XFF, 0XFF, 0X57,
                                           0XE1, 0XBF, 0XEB, 0X7F, 0X73, 0XFC,
                                           0X1F, 0XFF, 0X54, 0X3F, 0XFF, 0XF8,
                                           0X01, 0XF5, 0X5F, 0XF3, 0X7F, 0X6D,
                                           0XFB, 0X6F, 0XE1, 0X55, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XF5, 0X6F, 0XFB, 0X7F,
                                           0X5D, 0XFB, 0X6F, 0XDD, 0X15, 0X7F,
                                           0XFF, 0XF8, 0X01, 0XF5, 0X77, 0XC0,
                                           0X7F, 0X3B, 0XFD, 0X9F, 0XDD, 0X55,
                                           0X7F, 0XFF, 0XFF, 0X57, 0XF5, 0X6F,
                                           0XBB, 0X7F, 0XFF, 0XFF, 0XFF, 0XF8,
                                           0X55, 0X7F, 0XFF, 0XFF, 0X57, 0XF5,
                                           0X5F, 0XBB, 0X1F, 0X7F, 0XFC, 0XDF,
                                           0XF5, 0X45, 0X7F, 0XFF, 0XFC, 0X01,
                                           0XE1, 0XBF, 0XF2, 0X7F, 0X01, 0XFB,
                                           0X6F, 0XED, 0X7D, 0X7F, 0XFF, 0XFB,
                                           0XB7, 0XFF, 0X7F, 0XE9, 0X7F, 0X7B,
                                           0XFB, 0X6F, 0XDD, 0X7F, 0X7F, 0XFF,
                                           0XFB, 0X77, 0XFF, 0X7F, 0XDB, 0X7F,
                                           0XFF, 0XFC, 0X1F, 0XDF, 0X7F, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                           0XFF, 0XFF };

const unsigned char gImage_mb_red[2756] = { 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X55,
                                            0X55, 0X54, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X2A, 0XAA, 0XA8, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X55, 0X55, 0X54, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X2A, 0XAA, 0XA8, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X55, 0X55, 0X54,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X2A, 0XAA,
                                            0XA8, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X55,
                                            0X55, 0X54, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X2A, 0XAA, 0XA8, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X55, 0X55, 0X54, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X2A, 0XAA, 0XA8, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X55, 0X55, 0X54,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X2A, 0XAA,
                                            0XA8, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X55,
                                            0X55, 0X54, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X2A, 0XAA, 0XA8, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X55, 0X55, 0X54, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X2A, 0XAA, 0XA8, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X55, 0X55, 0X54,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X2A, 0XAA,
                                            0XA8, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X55,
                                            0X55, 0X54, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X10, 0X80, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X30, 0XB9, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X10, 0XA1, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X09, 0XA1, 0X80,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X0A, 0X91,
                                            0X40, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X04,
                                            0X89, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X02, 0X86, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X09, 0XE3, 0XC0, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X10, 0X92, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X20, 0X82, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X3F, 0X82,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X02,
                                            0X80, 0XC0, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X04, 0XBF, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X04, 0X82, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X08, 0X84, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X08, 0X88, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X18,
                                            0X10, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X24, 0XF0, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X25, 0X10, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X25, 0X10, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X25, 0X10, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X25, 0X10,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X2A,
                                            0XE0, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X18, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X01, 0XFC, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X06, 0X03,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X08,
                                            0X00, 0X80, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X08, 0X00, 0X80, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X07, 0XF8,
                                            0X00, 0X08, 0X00, 0X80, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X07,
                                            0XF8, 0X00, 0X08, 0X00, 0X80, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X07, 0XF8, 0X00, 0X06, 0X03, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X07, 0XF8, 0X00, 0X01, 0XFC,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X07, 0XF8, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X07, 0XF8, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X07, 0XF8,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X07,
                                            0XF8, 0X00, 0X0E, 0X0F, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X07, 0XF8, 0X00, 0X08, 0X30, 0X80,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X07, 0XF8, 0X00, 0X08, 0X40,
                                            0X80, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X07, 0XF8, 0X00, 0X08,
                                            0X80, 0X80, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X07, 0XF8, 0X00,
                                            0X09, 0X00, 0X80, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X07, 0XF8,
                                            0X00, 0X0A, 0X00, 0X80, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X07,
                                            0XF8, 0X00, 0X0C, 0X07, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X07, 0XF8, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X07, 0XF8, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X07, 0XF8, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X07, 0XF8, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X07, 0XC0,
                                            0X00, 0X08, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X08, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X0F, 0XFF, 0X80,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X08, 0X01,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X08,
                                            0X01, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XFF, 0XFC, 0X3F, 0XFF, 0XE0,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XFF, 0XFC, 0X3F, 0XFF,
                                            0XE0, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XFF, 0XFC, 0X3F,
                                            0XFF, 0XE0, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X3F, 0XFB, 0XFC,
                                            0X3F, 0XFF, 0XE0, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X3F, 0XFB,
                                            0XFC, 0X3F, 0XFF, 0XE0, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X3F,
                                            0XF1, 0XFC, 0X3F, 0XFF, 0XE0, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XF1, 0XFC, 0X3F, 0XFF, 0XE0,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XEA, 0XFC, 0X3F, 0XFF,
                                            0XE0, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XFB, 0XFC, 0X3F,
                                            0XFF, 0XE0, 0X00, 0X00, 0X1C, 0X00,
                                            0X00, 0X00, 0X00, 0X3F, 0XFB, 0XFC,
                                            0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XD0,
                                            0X80, 0X00, 0X00, 0X00, 0X3F, 0XFB,
                                            0XFC, 0X3F, 0XFF, 0XE0, 0X00, 0X12,
                                            0X50, 0X80, 0X00, 0X00, 0X00, 0X3F,
                                            0XFB, 0XFC, 0X3F, 0XFF, 0XE0, 0X00,
                                            0X12, 0X51, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XFB, 0XFC, 0X3F, 0XFF, 0XE0,
                                            0X00, 0X12, 0X51, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XFB, 0XFC, 0X3F, 0XFF,
                                            0XE0, 0X00, 0X12, 0X52, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XFF, 0XFC, 0X3F,
                                            0XFF, 0XE0, 0X00, 0X12, 0X52, 0X00,
                                            0X00, 0X00, 0X00, 0X3F, 0XFB, 0XFC,
                                            0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XCF,
                                            0XC0, 0X00, 0X00, 0X00, 0X3F, 0XFB,
                                            0XFC, 0X3F, 0XFF, 0XE0, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X3F,
                                            0X05, 0XFC, 0X3F, 0XFF, 0XE0, 0X00,
                                            0X00, 0X02, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XFE, 0XFC, 0X3F, 0XC3, 0XE0,
                                            0X00, 0X00, 0X12, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XFF, 0X3C, 0X3F, 0X3C,
                                            0XE0, 0X00, 0X00, 0X22, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0X86, 0XFC, 0X3E,
                                            0XFF, 0X60, 0X00, 0X1F, 0XFF, 0XC0,
                                            0X00, 0X00, 0X00, 0X3F, 0X7D, 0XFC,
                                            0X3F, 0XFF, 0XE0, 0X00, 0X20, 0X42,
                                            0X00, 0X00, 0X00, 0X00, 0X3F, 0XFB,
                                            0X3C, 0X3F, 0XFF, 0XE0, 0X00, 0X10,
                                            0X82, 0X00, 0X00, 0X00, 0X00, 0X3F,
                                            0X00, 0XFC, 0X3F, 0XFF, 0XE0, 0X00,
                                            0X00, 0X82, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XFD, 0XFC, 0X3F, 0XFF, 0XE0,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XFF, 0XFC, 0X3F, 0X9D,
                                            0XE0, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XD5, 0X7C, 0X3F,
                                            0X6D, 0XE0, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X3F, 0X15, 0X7C,
                                            0X3F, 0X6D, 0XE0, 0X00, 0X00, 0X80,
                                            0X00, 0X00, 0X00, 0X00, 0X3F, 0X55,
                                            0X7C, 0X3F, 0X73, 0XE0, 0X00, 0X00,
                                            0XC0, 0X00, 0X00, 0X00, 0X00, 0X3F,
                                            0X40, 0X3C, 0X3F, 0XFF, 0XE0, 0X00,
                                            0X00, 0X88, 0X80, 0X00, 0X00, 0X00,
                                            0X3F, 0X55, 0X7C, 0X3F, 0XFF, 0XE0,
                                            0X00, 0X00, 0X8C, 0XC0, 0X00, 0X00,
                                            0X00, 0X3F, 0X55, 0X7C, 0X3F, 0XFF,
                                            0XE0, 0X00, 0X00, 0X88, 0X80, 0X00,
                                            0X00, 0X00, 0X3F, 0X55, 0X7C, 0X3F,
                                            0XFF, 0XE0, 0X00, 0X00, 0X88, 0X80,
                                            0X00, 0X00, 0X00, 0X3F, 0X00, 0X3C,
                                            0X3F, 0XFF, 0XE0, 0X00, 0X00, 0X88,
                                            0X80, 0X00, 0X00, 0X00, 0X3F, 0XFC,
                                            0XFC, 0X3E, 0XFF, 0X60, 0X00, 0X00,
                                            0X88, 0X80, 0X00, 0X00, 0X00, 0X3F,
                                            0XFB, 0XFC, 0X3F, 0X3C, 0XE0, 0X00,
                                            0X1F, 0XFF, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XFF, 0XFC, 0X3F, 0XC3, 0XE0,
                                            0X00, 0X20, 0X89, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XEC, 0XFC, 0X3F, 0XFF,
                                            0XE0, 0X00, 0X10, 0X89, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XEA, 0XBC, 0X3F,
                                            0XFF, 0XE0, 0X00, 0X00, 0X89, 0X00,
                                            0X00, 0X00, 0X00, 0X3F, 0X6A, 0XBC,
                                            0X3F, 0XFF, 0XE0, 0X00, 0X00, 0X89,
                                            0X00, 0X00, 0X00, 0X00, 0X3F, 0X56,
                                            0XBC, 0X3F, 0XFF, 0XE0, 0X00, 0X00,
                                            0X89, 0X00, 0X00, 0X00, 0X00, 0X3F,
                                            0X16, 0XBC, 0X3F, 0X3F, 0XE0, 0X00,
                                            0X00, 0X89, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0X48, 0X3C, 0X3F, 0XC7, 0XE0,
                                            0X00, 0X00, 0X80, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0X56, 0XBC, 0X3F, 0XD9,
                                            0XE0, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XEA, 0XBC, 0X3F,
                                            0XC7, 0XE0, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X3F, 0XEA, 0XBC,
                                            0X3F, 0X3F, 0XE0, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X3F, 0XEC,
                                            0XFC, 0X3F, 0XFF, 0XE0, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X3F,
                                            0XFF, 0XFC, 0X3F, 0XFF, 0XE0, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X3F, 0XFF, 0XFC, 0X3F, 0XFF, 0XE0,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X3F, 0XFF, 0XFC, 0X3F, 0XFF,
                                            0XE0, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X3F, 0XFF, 0XFC, 0X3F,
                                            0XFF, 0XE0, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
                                            0X00, 0X00 };

const unsigned char gImage_HStest_bw[2756] = { 0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XE0, 0X3F, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X01, 0XFC, 0X07, 0XDF, 0XDF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X02,
                                               0X07, 0XDF, 0XDF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X02, 0X02, 0X07, 0XDF,
                                               0XDF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X02,
                                               0X02, 0X07, 0XE0, 0X3F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X01, 0XFC, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0XFC, 0X07, 0XF0, 0X3F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X01, 0X22, 0X07,
                                               0XED, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X42, 0X07, 0XDB, 0XDF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X42,
                                               0X07, 0XDB, 0XDF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X03, 0X3C, 0X07, 0XCC,
                                               0X3F, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XE3, 0X3F, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X01, 0XCC, 0X07, 0XDC,
                                               0XDF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X02,
                                               0X32, 0X07, 0XDD, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X22, 0X07,
                                               0XDC, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X32, 0X07, 0XE3, 0X3F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X01, 0XCC,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X02, 0X07, 0XFE, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X1E,
                                               0X07, 0XC1, 0XDF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X03, 0XE2, 0X07, 0XFF,
                                               0XDF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X02, 0X07, 0XFF, 0X9F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X06, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X01,
                                               0XE6, 0X07, 0XE1, 0X9F, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X12, 0X07,
                                               0XDE, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X12, 0X07, 0XDE, 0XDF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X24,
                                               0X07, 0XDD, 0XBF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X01, 0XF8, 0X07, 0XE0,
                                               0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X01, 0XE2,
                                               0X07, 0XE1, 0XDF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X02, 0X12, 0X07, 0XDE,
                                               0XDF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X02,
                                               0X12, 0X07, 0XDE, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X12, 0X07,
                                               0XDE, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X01, 0XBE, 0X07, 0XE4, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XDF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X00, 0X07, 0XC0, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X03, 0XFE,
                                               0X07, 0XD7, 0XBF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X02, 0X84, 0X07, 0XF7,
                                               0X7F, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X88, 0X07, 0XF4, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0XB0, 0X07,
                                               0XFB, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X40, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XE1,
                                               0X3F, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X01,
                                               0XEC, 0X07, 0XDE, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X12, 0X07,
                                               0XDE, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X12, 0X07, 0XDF, 0XDF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X02,
                                               0X07, 0XEF, 0XBF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X01, 0X04, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X1C,
                                               0X07, 0XDE, 0X3F, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X02, 0X22, 0X07, 0XDD,
                                               0XDF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X02,
                                               0X42, 0X07, 0XDB, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X82, 0X07,
                                               0XD7, 0XDF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X03, 0X0C, 0X07, 0XCF, 0X3F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X00, 0X07, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X03, 0XFE,
                                               0X07, 0XC0, 0X1F, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X02, 0X04, 0X07, 0XDF,
                                               0XBF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XF7, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFE, 0XF7, 0X7F, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFE, 0XF6,
                                               0X7F, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFE, 0X97, 0X7F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFE,
                                               0X6B, 0X9F, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X04, 0X5A, 0X07,
                                               0XDD, 0X2F, 0XFE, 0XE8, 0X7F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X05, 0X53, 0X07, 0XD5, 0X67,
                                               0XFE, 0X8C, 0XBF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X4A,
                                               0X87, 0XED, 0XAB, 0XFE, 0X68,
                                               0XBF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X01, 0X44, 0X07, 0XF5,
                                               0XDF, 0XFE, 0XEA, 0X3F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0XC7, 0X87, 0XF9, 0XC3, 0XFE,
                                               0X17, 0X9F, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X64, 0X07,
                                               0XEC, 0XDF, 0XFE, 0XF7, 0X7F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X04, 0XC4, 0X07, 0XD9, 0XDF,
                                               0XFE, 0XEE, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X07, 0X40,
                                               0X87, 0XC5, 0XFB, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X01, 0X5F, 0X07, 0XF5,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X02,
                                               0X42, 0X07, 0XED, 0XEF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X44, 0X07,
                                               0XED, 0XDF, 0XFF, 0XF9, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XFD, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFE, 0X06,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X3F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X41, 0X07, 0XFD, 0XF7, 0XFF,
                                               0XFF, 0X3F, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X07, 0X55, 0X07,
                                               0XC5, 0X57, 0XFF, 0X86, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X05, 0X55, 0X07, 0XD5, 0X57,
                                               0XFE, 0X7D, 0XFF, 0XC0, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X05, 0X55,
                                               0X07, 0XD5, 0X57, 0XFE, 0XFB,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X05, 0X7F, 0X87, 0XD4,
                                               0X03, 0XFF, 0XFF, 0X3F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X05,
                                               0X55, 0X07, 0XD5, 0X57, 0XFE,
                                               0X00, 0XFF, 0XC0, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X05, 0X55, 0X07,
                                               0XD5, 0X57, 0XFF, 0XFB, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X05, 0X55, 0X07, 0XD5, 0X57,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X05, 0X55,
                                               0X87, 0XD5, 0X53, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X01, 0XFF, 0XFF,
                                               0XFF, 0X07, 0X7E, 0X07, 0XC4,
                                               0X0F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X04, 0X07, 0XFF, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0XF7, 0XFF,
                                               0XC0, 0X01, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XF7, 0XBF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XF7,
                                               0X3F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X86, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XF7, 0X3F, 0XE0,
                                               0X03, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X92, 0X87, 0XFB, 0XCF, 0XFF,
                                               0XF6, 0XBF, 0XF6, 0XDB, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X56, 0X87,
                                               0XFB, 0X6B, 0XFE, 0X01, 0XBF,
                                               0XF7, 0X5B, 0XFF, 0XFF, 0XFF,
                                               0X04, 0XD6, 0X87, 0XFD, 0X4B,
                                               0XFE, 0XF7, 0XBF, 0XF0, 0X03,
                                               0XFF, 0XFF, 0XFF, 0X05, 0XA2,
                                               0X87, 0XD9, 0X4B, 0XFE, 0XF7,
                                               0XBF, 0XF7, 0X5B, 0XFF, 0XFF,
                                               0XFF, 0X02, 0XDF, 0X87, 0XD2,
                                               0XEB, 0XFF, 0XF7, 0XBF, 0XF6,
                                               0XDB, 0XFF, 0XFF, 0XFF, 0X02,
                                               0XA2, 0X87, 0XE9, 0X03, 0XFF,
                                               0XF7, 0XBF, 0XE0, 0X03, 0XFF,
                                               0XFF, 0XFF, 0X02, 0XD6, 0X87,
                                               0XEA, 0XEB, 0XFF, 0XF7, 0XFF,
                                               0XEA, 0XAB, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X56, 0X87, 0XE9, 0X4B,
                                               0XFF, 0XFF, 0XFF, 0XEC, 0XC1,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X92,
                                               0X87, 0XFD, 0X4B, 0XFF, 0XFF,
                                               0XFF, 0XEA, 0XAF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X86, 0X07, 0XFB,
                                               0X6B, 0XFF, 0XFF, 0XFF, 0XE0,
                                               0X23, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFB, 0XCF, 0XFE,
                                               0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFE, 0XC0, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFE, 0XC0, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFE, 0XDA,
                                               0XFF, 0XED, 0X45, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X08, 0X07, 0XFF,
                                               0XBF, 0XFE, 0XDA, 0XFF, 0XF0,
                                               0X11, 0XFF, 0XFF, 0XFF, 0X07,
                                               0XF4, 0X07, 0XC0, 0X5F, 0XFE,
                                               0XDA, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X02, 0X07,
                                               0XFF, 0XEF, 0XFF, 0X00, 0X1F,
                                               0XE5, 0X45, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X01, 0X87, 0XFF, 0XF3,
                                               0XFF, 0XDA, 0XFF, 0XED, 0X55,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X02,
                                               0X07, 0XFF, 0XEF, 0XFF, 0XDA,
                                               0XFF, 0XF0, 0X11, 0XFF, 0XFF,
                                               0XFF, 0X03, 0XF4, 0X07, 0XE0,
                                               0X5F, 0XFF, 0XDA, 0XFF, 0XFF,
                                               0XEB, 0XFF, 0XFF, 0XFF, 0X04,
                                               0X08, 0X07, 0XDF, 0XBF, 0XFF,
                                               0X80, 0XFF, 0XE0, 0X23, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X01, 0X87,
                                               0XFF, 0XF3, 0XFF, 0XFF, 0XFF,
                                               0XFA, 0XA9, 0XFF, 0XFF, 0XFF,
                                               0X07, 0XFE, 0X07, 0XC0, 0X0F,
                                               0XFF, 0XFF, 0XFF, 0XE0, 0X23,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X08,
                                               0X07, 0XFF, 0XBF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XEB, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X10, 0X07, 0XFF,
                                               0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFE,
                                               0X00, 0X3F, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFE, 0XED, 0XBF,
                                               0XF7, 0X2B, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XFF, 0XED, 0XBF, 0XED, 0X81,
                                               0XFF, 0XFF, 0XFF, 0X04, 0X10,
                                               0X07, 0XDF, 0X7F, 0XFF, 0XED,
                                               0XBF, 0XFB, 0XAB, 0XFF, 0XFF,
                                               0XFF, 0X02, 0X10, 0X07, 0XEF,
                                               0X7F, 0XFF, 0X02, 0X7F, 0XE8,
                                               0X83, 0XFF, 0XFF, 0XFF, 0X01,
                                               0X08, 0X07, 0XF7, 0XBF, 0XFE,
                                               0XFE, 0XFF, 0XE1, 0X29, 0XFF,
                                               0XFF, 0XFF, 0X00, 0XE4, 0X07,
                                               0XF8, 0XDF, 0XFF, 0XC2, 0XBF,
                                               0XEA, 0X97, 0XFF, 0XFF, 0XFF,
                                               0X01, 0X02, 0X07, 0XF7, 0XEF,
                                               0XFF, 0XEE, 0X7F, 0XE1, 0X01,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X01,
                                               0X87, 0XEF, 0XF3, 0XFF, 0X80,
                                               0XFF, 0XF9, 0X2B, 0XFF, 0XFF,
                                               0XFF, 0X05, 0X02, 0X07, 0XD7,
                                               0XEF, 0XFF, 0X6E, 0X3F, 0XF7,
                                               0XEF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0XE4, 0X07, 0XF8, 0XDF, 0XFE,
                                               0XE2, 0XFF, 0XF8, 0X01, 0XFF,
                                               0XFF, 0XFF, 0X01, 0X08, 0X07,
                                               0XF7, 0XBF, 0XFF, 0XFF, 0XFF,
                                               0XE7, 0X9F, 0XFF, 0XFF, 0XFF,
                                               0X02, 0X10, 0X07, 0XEF, 0X7F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X04, 0X10,
                                               0X07, 0XDF, 0X7F, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFE,
                                               0XFF, 0XFF, 0XE8, 0XC5, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFF, 0X7C, 0X3F,
                                               0XE2, 0X55, 0XFF, 0XFF, 0XFF,
                                               0X04, 0X00, 0X07, 0XDF, 0XFF,
                                               0XFF, 0X33, 0XBF, 0XFF, 0XD5,
                                               0XFF, 0XFF, 0XFF, 0X02, 0X00,
                                               0X07, 0XEF, 0XFF, 0XFF, 0X8F,
                                               0XBF, 0XE8, 0X91, 0XFF, 0XFF,
                                               0XFF, 0X01, 0X00, 0X07, 0XF7,
                                               0XFF, 0XFF, 0XA7, 0XBF, 0XE2,
                                               0X6F, 0XFF, 0XFF, 0XFF, 0X00,
                                               0XC0, 0X07, 0XF9, 0XFF, 0XFF,
                                               0X78, 0XBF, 0XFF, 0X8B, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X30, 0X07,
                                               0XFE, 0X7F, 0XFF, 0X7F, 0X3F,
                                               0XFD, 0X63, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X0F, 0X87, 0XFF, 0X83,
                                               0XFE, 0XFF, 0XBF, 0XE0, 0X69,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X30,
                                               0X07, 0XFE, 0X7F, 0XFE, 0XCF,
                                               0X7F, 0XF5, 0X63, 0XFF, 0XFF,
                                               0XFF, 0X00, 0XC0, 0X07, 0XF9,
                                               0XFF, 0XFF, 0X99, 0XBF, 0XE0,
                                               0X0B, 0XFF, 0XFF, 0XFF, 0X01,
                                               0X00, 0X07, 0XF7, 0XFF, 0XFE,
                                               0X7D, 0XFF, 0XFD, 0X6B, 0XFF,
                                               0XFF, 0XFF, 0X02, 0X00, 0X07,
                                               0XEF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0X04, 0X00, 0X07, 0XDF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0X00, 0X00,
                                               0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0X00, 0X00, 0X07, 0XFF,
                                               0XFF, 0XFE, 0X1E, 0X1E, 0X1E,
                                               0X1F, 0XFF, 0XFF, 0XFF, 0X00,
                                               0X00, 0X07, 0XFF, 0XFF, 0XFE,
                                               0X1E, 0X1E, 0X1E, 0X1F, 0XFF,
                                               0XFF, 0XFF, 0X00, 0X00, 0X07,
                                               0XFF, 0XFF, 0XFE, 0X1E, 0X1E,
                                               0X1E, 0X1F, 0XFF, 0XFF, 0XFF,
                                               0X00, 0X00, 0X07, 0XFF, 0XFF,
                                               0XF0, 0X00, 0X00, 0X00, 0X00,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XF0, 0X00,
                                               0X00, 0X00, 0X00, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XF0, 0X00, 0X00, 0X00,
                                               0X00, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XF0,
                                               0X00, 0X00, 0X00, 0X00, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFE, 0X1E, 0X1E,
                                               0X1E, 0X1F, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFE, 0X1E, 0X1E, 0X1E, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFE, 0X1E,
                                               0X1E, 0X1E, 0X1F, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFE, 0X1E, 0X1E, 0X1E,
                                               0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XF0,
                                               0X00, 0X00, 0X00, 0X00, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XF0, 0X00, 0X00,
                                               0X00, 0X00, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XE6, 0X66, 0X67,
                                               0XF0, 0X00, 0X00, 0X00, 0X00,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X00, 0X01, 0XF0, 0X00,
                                               0X00, 0X00, 0X00, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X00,
                                               0X01, 0XFE, 0X1E, 0X1E, 0X1E,
                                               0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XE6, 0X66, 0X67, 0XFE,
                                               0X1E, 0X1E, 0X1E, 0X1F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XE6,
                                               0X66, 0X67, 0XFE, 0X1E, 0X1E,
                                               0X1E, 0X1F, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X00, 0X01,
                                               0XFE, 0X1E, 0X1E, 0X1E, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XC0, 0X00, 0X01, 0XF0, 0X00,
                                               0X00, 0X00, 0X00, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XE6, 0X66,
                                               0X67, 0XF0, 0X00, 0X00, 0X00,
                                               0X00, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XE6, 0X66, 0X67, 0XF0,
                                               0X00, 0X00, 0X00, 0X00, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X00, 0X01, 0XF0, 0X00, 0X00,
                                               0X00, 0X00, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XC0, 0X00, 0X01,
                                               0XFE, 0X1E, 0X1E, 0X1E, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XE6, 0X66, 0X67, 0XFE, 0X1E,
                                               0X1E, 0X1E, 0X1F, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XE6, 0X66,
                                               0X67, 0XFE, 0X1E, 0X1E, 0X1E,
                                               0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X00, 0X01, 0XFE,
                                               0X1E, 0X1E, 0X1E, 0X1F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                               0X00, 0X01, 0XF0, 0X00, 0X00,
                                               0X00, 0X00, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XE6, 0X66, 0X67,
                                               0XF0, 0X00, 0X00, 0X00, 0X00,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XE6, 0X66, 0X67, 0XF0, 0X00,
                                               0X00, 0X00, 0X00, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XC0, 0X00,
                                               0X01, 0XF0, 0X00, 0X00, 0X00,
                                               0X00, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XC0, 0X00, 0X01, 0XFE,
                                               0X1E, 0X1E, 0X1E, 0X1F, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XE6,
                                               0X66, 0X67, 0XFE, 0X1E, 0X1E,
                                               0X1E, 0X1F, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XE6, 0X66, 0X67,
                                               0XFE, 0X1E, 0X1E, 0X1E, 0X1F,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                               0XFF };

const unsigned char gImage_HStest_red[2756] = { 0X0F, 0XF7, 0XFC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XF7,
                                                0X7C, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0E, 0XF6, 0X7C, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X97, 0X7C, 0X00, 0X00, 0X00,
                                                0X7E, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0X6B, 0X9C,
                                                0X00, 0X00, 0X00, 0XFF, 0X00,
                                                0X01, 0XFC, 0X00, 0X00, 0X00,
                                                0X0E, 0XE8, 0X7C, 0X00, 0X00,
                                                0X01, 0XC3, 0X80, 0X02, 0X02,
                                                0X00, 0X00, 0X00, 0X0E, 0X8C,
                                                0XBC, 0X00, 0X00, 0X01, 0X81,
                                                0X80, 0X02, 0X02, 0X00, 0X00,
                                                0X00, 0X0E, 0X68, 0XBC, 0X00,
                                                0X00, 0X01, 0XC3, 0X80, 0X02,
                                                0X02, 0X00, 0X00, 0X00, 0X0E,
                                                0XEA, 0X3C, 0X00, 0X00, 0X00,
                                                0XFF, 0X00, 0X01, 0XFC, 0X00,
                                                0X00, 0X00, 0X0E, 0X17, 0X9C,
                                                0X00, 0X00, 0X00, 0X7E, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XF7, 0X7C, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XEE,
                                                0XFC, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X7E, 0X00, 0X00,
                                                0XFC, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0XFF, 0X80, 0X01, 0X22, 0X00,
                                                0X00, 0X00, 0X0F, 0XFF, 0XFC,
                                                0X00, 0X00, 0X01, 0X91, 0X80,
                                                0X02, 0X42, 0X00, 0X00, 0X00,
                                                0X0F, 0XF9, 0XFC, 0X00, 0X00,
                                                0X01, 0XB1, 0X80, 0X02, 0X42,
                                                0X00, 0X00, 0X00, 0X0F, 0XFD,
                                                0XFC, 0X00, 0X00, 0X01, 0XBF,
                                                0X80, 0X03, 0X3C, 0X00, 0X00,
                                                0X00, 0X0E, 0X06, 0XFC, 0X00,
                                                0X00, 0X00, 0X1E, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0X3C, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XFF, 0X3C,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0X86, 0XFC, 0X00, 0X00,
                                                0X00, 0XE7, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0X7D,
                                                0XFC, 0X00, 0X00, 0X01, 0XFF,
                                                0X80, 0X01, 0XCC, 0X00, 0X00,
                                                0X00, 0X0E, 0XFB, 0XFC, 0X00,
                                                0X00, 0X01, 0X99, 0X80, 0X02,
                                                0X32, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0X3C, 0X00, 0X00, 0X01,
                                                0X99, 0X80, 0X02, 0X22, 0X00,
                                                0X00, 0X00, 0X0E, 0X00, 0XFC,
                                                0X00, 0X00, 0X01, 0XFF, 0X80,
                                                0X02, 0X32, 0X00, 0X00, 0X00,
                                                0X0F, 0XFB, 0XFC, 0X00, 0X00,
                                                0X00, 0XE7, 0X00, 0X01, 0XCC,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0X01, 0X80, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XFF, 0XFC,
                                                0X00, 0X00, 0X00, 0X0F, 0X80,
                                                0X00, 0X02, 0X00, 0X00, 0X00,
                                                0X0F, 0XF7, 0XFC, 0X00, 0X00,
                                                0X00, 0X7D, 0X80, 0X00, 0X1E,
                                                0X00, 0X00, 0X00, 0X0F, 0XF7,
                                                0XBC, 0X00, 0X00, 0X01, 0XE1,
                                                0X80, 0X03, 0XE2, 0X00, 0X00,
                                                0X00, 0X0F, 0XF7, 0X3C, 0X00,
                                                0X00, 0X01, 0X81, 0X80, 0X00,
                                                0X02, 0X00, 0X00, 0X00, 0X0F,
                                                0XF7, 0X3C, 0X00, 0X00, 0X00,
                                                0X01, 0X80, 0X00, 0X06, 0X00,
                                                0X00, 0X00, 0X0F, 0XF6, 0XBC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0X01, 0XBC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XF7,
                                                0XBC, 0X00, 0X00, 0X00, 0X78,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0E, 0XF7, 0XBC, 0X00,
                                                0X00, 0X00, 0XFD, 0X80, 0X01,
                                                0XE6, 0X00, 0X00, 0X00, 0X0F,
                                                0XF7, 0XBC, 0X00, 0X00, 0X01,
                                                0X8D, 0X80, 0X02, 0X12, 0X00,
                                                0X00, 0X00, 0X0F, 0XF7, 0XBC,
                                                0X00, 0X00, 0X01, 0X8D, 0X80,
                                                0X02, 0X12, 0X00, 0X00, 0X00,
                                                0X0F, 0XF7, 0XFC, 0X00, 0X00,
                                                0X01, 0X8B, 0X80, 0X02, 0X24,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X00, 0XFF,
                                                0X00, 0X01, 0XF8, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X7C, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0X7F, 0XFC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XC0, 0XFC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X01, 0XE2,
                                                0X00, 0X00, 0X00, 0X0E, 0XC0,
                                                0XFC, 0X00, 0X00, 0X00, 0XF1,
                                                0X80, 0X02, 0X12, 0X00, 0X00,
                                                0X00, 0X0E, 0XDA, 0XFC, 0X00,
                                                0X00, 0X00, 0XF9, 0X80, 0X02,
                                                0X12, 0X00, 0X00, 0X00, 0X0E,
                                                0XDA, 0XFC, 0X00, 0X00, 0X01,
                                                0X99, 0X80, 0X02, 0X12, 0X00,
                                                0X00, 0X00, 0X0E, 0XDA, 0XFC,
                                                0X00, 0X00, 0X01, 0X99, 0X80,
                                                0X01, 0XBE, 0X00, 0X00, 0X00,
                                                0X0F, 0X00, 0X1C, 0X00, 0X00,
                                                0X01, 0X9F, 0X80, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XDA,
                                                0XFC, 0X00, 0X00, 0X01, 0XDF,
                                                0X80, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XDA, 0XFC, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XDA, 0XFC, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0X80, 0XFC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X02, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X00, 0X20, 0X00, 0X03, 0XFE,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X01, 0XFF,
                                                0X80, 0X02, 0X84, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X01, 0XFF, 0X80, 0X00,
                                                0X88, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0X23, 0X80, 0X00, 0XB0, 0X00,
                                                0X00, 0X00, 0X0E, 0X00, 0X3C,
                                                0X00, 0X00, 0X00, 0X2E, 0X00,
                                                0X00, 0X40, 0X00, 0X00, 0X00,
                                                0X0E, 0XED, 0XBC, 0X00, 0X00,
                                                0X00, 0X3C, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XED,
                                                0XBC, 0X00, 0X00, 0X00, 0X30,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XED, 0XBC, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X01,
                                                0XEC, 0X00, 0X00, 0X00, 0X0F,
                                                0X02, 0X7C, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X02, 0X12, 0X00,
                                                0X00, 0X00, 0X0E, 0XFE, 0XFC,
                                                0X00, 0X00, 0X00, 0XE7, 0X00,
                                                0X02, 0X12, 0X00, 0X00, 0X00,
                                                0X0F, 0XC2, 0XBC, 0X00, 0X00,
                                                0X01, 0XFF, 0X80, 0X02, 0X02,
                                                0X00, 0X00, 0X00, 0X0F, 0XEE,
                                                0X7C, 0X00, 0X00, 0X01, 0X99,
                                                0X80, 0X01, 0X04, 0X00, 0X00,
                                                0X00, 0X0F, 0X80, 0XFC, 0X00,
                                                0X00, 0X01, 0X99, 0X80, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0X6E, 0X3C, 0X00, 0X00, 0X01,
                                                0X99, 0X80, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0XE2, 0XFC,
                                                0X00, 0X00, 0X01, 0XC3, 0X80,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X02, 0X1C,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X02, 0X22, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X01, 0X8F, 0X00, 0X02,
                                                0X42, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X01,
                                                0X9F, 0X80, 0X02, 0X82, 0X00,
                                                0X00, 0X00, 0X0E, 0XFF, 0XFC,
                                                0X00, 0X00, 0X01, 0XB1, 0X80,
                                                0X03, 0X0C, 0X00, 0X00, 0X00,
                                                0X0F, 0X7C, 0X3C, 0X00, 0X00,
                                                0X01, 0XE1, 0X80, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0X33,
                                                0XBC, 0X00, 0X00, 0X01, 0XC1,
                                                0X80, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0X8F, 0XBC, 0X00,
                                                0X00, 0X01, 0X83, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XA7, 0XBC, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0X78, 0XBC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X02, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0X7F, 0X3C, 0X00, 0X00,
                                                0X01, 0X80, 0X00, 0X03, 0XFE,
                                                0X00, 0X00, 0X00, 0X0E, 0XFF,
                                                0XBC, 0X00, 0X00, 0X01, 0X80,
                                                0X00, 0X02, 0X04, 0X00, 0X00,
                                                0X00, 0X0E, 0XCF, 0X7C, 0X00,
                                                0X00, 0X01, 0XFF, 0X80, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0X99, 0XBC, 0X00, 0X00, 0X01,
                                                0XFF, 0X80, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0X7D, 0XFC,
                                                0X00, 0X00, 0X01, 0X81, 0X80,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X01, 0X81, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XF7, 0XFC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XF7, 0X7C, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XF6,
                                                0X7C, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0E, 0X97, 0X7C, 0X00,
                                                0X00, 0X00, 0X25, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X6B, 0X9C, 0X00, 0X00, 0X02,
                                                0XAF, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0XE8, 0X7C,
                                                0X00, 0X00, 0X03, 0XE9, 0X80,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0X8C, 0XBC, 0X00, 0X00,
                                                0X01, 0XA9, 0XC0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0X68,
                                                0XBC, 0X00, 0X00, 0X01, 0XA7,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0E, 0XEA, 0X3C, 0X00,
                                                0X00, 0X03, 0XE3, 0XC0, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X17, 0X9C, 0X00, 0X00, 0X02,
                                                0X7A, 0XC0, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0XF7, 0X7C,
                                                0X00, 0X00, 0X02, 0X2A, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XEE, 0XFC, 0X00, 0X00,
                                                0X03, 0XE2, 0X40, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X03, 0XEF,
                                                0XC0, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0XAF, 0X80, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0XA3, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XF9, 0XFC,
                                                0X00, 0X00, 0X00, 0XA0, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFD, 0XFC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0X06,
                                                0XFC, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0X3C, 0X00,
                                                0X00, 0X00, 0X2A, 0X80, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0X3C, 0X00, 0X00, 0X03,
                                                0XEA, 0X80, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0X86, 0XFC,
                                                0X00, 0X00, 0X01, 0X2A, 0X80,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0X7D, 0XFC, 0X00, 0X00,
                                                0X01, 0X2A, 0X80, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XFB,
                                                0XFC, 0X00, 0X00, 0X01, 0X3F,
                                                0XC0, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0X3C, 0X00,
                                                0X00, 0X01, 0X3F, 0XC0, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X00, 0XFC, 0X00, 0X00, 0X01,
                                                0X2A, 0X80, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XFB, 0XFC,
                                                0X00, 0X00, 0X01, 0X2A, 0X80,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X01, 0X2A, 0XC0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X03, 0XFF,
                                                0XC0, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X07, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XF7, 0XFC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XF7, 0XBC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XF7,
                                                0X3C, 0X00, 0X00, 0X00, 0X20,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XF7, 0X3C, 0X00,
                                                0X00, 0X00, 0X27, 0X40, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XF6, 0XBC, 0X00, 0X00, 0X00,
                                                0X75, 0X40, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0X01, 0XBC,
                                                0X00, 0X00, 0X00, 0XF5, 0X40,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XF7, 0XBC, 0X00, 0X00,
                                                0X03, 0XDD, 0X40, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XF7,
                                                0XBC, 0X00, 0X00, 0X03, 0X49,
                                                0X40, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XF7, 0XBC, 0X00,
                                                0X00, 0X01, 0X7F, 0XC0, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0XF7, 0XBC, 0X00, 0X00, 0X01,
                                                0X4D, 0X40, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XF7, 0XFC,
                                                0X00, 0X00, 0X01, 0X5D, 0X40,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X00, 0XD5, 0X40, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X00, 0XF5,
                                                0X40, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X27, 0X40, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X7F, 0XFC, 0X00, 0X00, 0X00,
                                                0X20, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0XC0, 0XFC,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XC0, 0XFC, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XDA,
                                                0XFC, 0X00, 0X00, 0X00, 0X0C,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0E, 0XDA, 0XFC, 0X00,
                                                0X00, 0X03, 0XFF, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0XDA, 0XFC, 0X00, 0X00, 0X03,
                                                0XF9, 0X80, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0X00, 0X1C,
                                                0X00, 0X00, 0X00, 0X00, 0XC0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XDA, 0XFC, 0X00, 0X00,
                                                0X00, 0X01, 0XC0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XDA,
                                                0XFC, 0X00, 0X00, 0X01, 0XFB,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XDA, 0XFC, 0X00,
                                                0X00, 0X03, 0XE6, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0X80, 0XFC, 0X00, 0X00, 0X01,
                                                0X0C, 0X40, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XFF, 0XFC,
                                                0X00, 0X00, 0X03, 0XFF, 0XC0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X03, 0XFF, 0X80, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X00, 0X1E,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X18, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X00, 0X3C, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0E, 0XED, 0XBC,
                                                0X00, 0X00, 0X00, 0X08, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XED, 0XBC, 0X00, 0X00,
                                                0X03, 0X0C, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XED,
                                                0XBC, 0X00, 0X00, 0X01, 0X84,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0X02, 0X7C, 0X00,
                                                0X00, 0X00, 0XF6, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0XFE, 0XFC, 0X00, 0X00, 0X00,
                                                0X7F, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XC2, 0XBC,
                                                0X00, 0X00, 0X01, 0XE1, 0XC0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XEE, 0X7C, 0X00, 0X00,
                                                0X01, 0XC0, 0XC0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0X80,
                                                0XFC, 0X00, 0X00, 0X02, 0X61,
                                                0XC0, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0X6E, 0X3C, 0X00,
                                                0X00, 0X00, 0X3D, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0XE2, 0XFC, 0X00, 0X00, 0X00,
                                                0XFE, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XFF, 0XFC,
                                                0X00, 0X00, 0X01, 0XC4, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0XFF, 0XFC, 0X00, 0X00,
                                                0X03, 0X0C, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0XFF,
                                                0XFC, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XFF, 0XFC, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0XFF, 0XFC, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0X7C, 0X3C,
                                                0X00, 0X00, 0X02, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0F, 0X33, 0XBC, 0X00, 0X00,
                                                0X03, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0F, 0X8F,
                                                0XBC, 0X00, 0X00, 0X01, 0X80,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0XA7, 0XBC, 0X00,
                                                0X00, 0X00, 0XE0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0F,
                                                0X78, 0XBC, 0X00, 0X00, 0X00,
                                                0X3C, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0X7F, 0X3C,
                                                0X00, 0X00, 0X00, 0X0F, 0XC0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X0E, 0XFF, 0XBC, 0X00, 0X00,
                                                0X00, 0X3F, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X0E, 0XCF,
                                                0X7C, 0X00, 0X00, 0X00, 0XE0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X0F, 0X99, 0XBC, 0X00,
                                                0X00, 0X01, 0XC0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X0E,
                                                0X7D, 0XFC, 0X00, 0X00, 0X01,
                                                0X80, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X0F, 0XFF, 0XFC,
                                                0X00, 0X00, 0X03, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X1C, 0X71, 0XC7, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X1C,
                                                0X71, 0XC7, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X7F, 0XFF, 0XFF,
                                                0XE0, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X7F, 0XFF, 0XFF, 0XE0, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X7F, 0XFF,
                                                0XFF, 0XE0, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X1C, 0X71, 0XC7, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X1C,
                                                0X71, 0XC7, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X1C, 0X71, 0XC7,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X7F, 0XFF, 0XFF, 0XE0, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X7F, 0XFF,
                                                0XFF, 0XE0, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X7F, 0XFF, 0XFF, 0XE0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X1C,
                                                0X71, 0XC7, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X1C, 0X71, 0XC7,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X1C, 0X71, 0XC7, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X7F, 0XFF,
                                                0XFF, 0XE0, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X7F, 0XFF, 0XFF, 0XE0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X7F,
                                                0XFF, 0XFF, 0XE0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X1C, 0X71, 0XC7,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X1C, 0X71, 0XC7, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X1C, 0X71,
                                                0XC7, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X7F, 0XFF, 0XFF, 0XE0,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X7F,
                                                0XFF, 0XFF, 0XE0, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X7F, 0XFF, 0XFF,
                                                0XE0, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X1C, 0X71, 0XC7, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X1C, 0X71,
                                                0XC7, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00, 0X00, 0X00, 0X00, 0X00,
                                                0X00 };

const unsigned char gImage_LGTEST_BW1[2756] = { 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFA,
                                                0XAA, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFA, 0XAA, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFA, 0XAA, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFA, 0XAA, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE7, 0X87, 0XE1, 0XE6, 0X00,
                                                0X07, 0XFF, 0XFF, 0XFA, 0XAA,
                                                0XFF, 0XFF, 0XFF, 0XE7, 0X87,
                                                0XE1, 0XE6, 0X00, 0X07, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE6, 0X78, 0X66,
                                                0X7F, 0XE7, 0XFF, 0XFF, 0XFA,
                                                0XAA, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE6, 0X78, 0X66, 0X7F, 0XE7,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0X7E, 0X1E,
                                                0X06, 0X60, 0X67, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0X7E, 0X1E, 0X06, 0X60,
                                                0X67, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFE, 0X19,
                                                0X87, 0X86, 0X60, 0X67, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFE, 0X19, 0X87, 0X86,
                                                0X60, 0X67, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XE0,
                                                0X06, 0X00, 0X1E, 0X60, 0X67,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X06, 0X00,
                                                0X1E, 0X60, 0X67, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XF9, 0XE7, 0XE1, 0XE6, 0X7F,
                                                0XE7, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XF9, 0XE7,
                                                0XE1, 0XE6, 0X7F, 0XE7, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0X9F, 0X9F, 0XFE,
                                                0X00, 0X07, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0X9F, 0X9F, 0XFE, 0X00, 0X07,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X61, 0XF8,
                                                0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE0, 0X61, 0XF8, 0X1F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE1, 0XFF,
                                                0XF9, 0XF8, 0X1F, 0X9F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XE1, 0XFF, 0XF9, 0XF8,
                                                0X1F, 0X9F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XF8, 0X61, 0XE1, 0X9F, 0X87,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XF8, 0X61,
                                                0XE1, 0X9F, 0X87, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE6, 0X79, 0X81, 0X9E, 0X00,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE6, 0X79,
                                                0X81, 0X9E, 0X00, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XF8, 0X7F, 0X9E, 0X1F,
                                                0XE0, 0X67, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XF8,
                                                0X7F, 0X9E, 0X1F, 0XE0, 0X67,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE1, 0XE1, 0X81,
                                                0XFE, 0X18, 0X07, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE1, 0XE1, 0X81, 0XFE, 0X18,
                                                0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XF9, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XF9, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XE0,
                                                0X00, 0X66, 0X66, 0X00, 0X07,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X00, 0X66,
                                                0X66, 0X00, 0X07, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE7, 0XFE, 0X78, 0X66, 0X7F,
                                                0XE7, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE7, 0XFE,
                                                0X78, 0X66, 0X7F, 0XE7, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XE6, 0X06, 0X79, 0X86,
                                                0X60, 0X67, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XE6,
                                                0X06, 0X79, 0X86, 0X60, 0X67,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE6, 0X06, 0X61,
                                                0XFE, 0X60, 0X67, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE6, 0X06, 0X61, 0XFE, 0X60,
                                                0X67, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE6, 0X06,
                                                0X60, 0X1E, 0X60, 0X67, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XE6, 0X06, 0X60, 0X1E,
                                                0X60, 0X67, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XE7,
                                                0XFE, 0X79, 0X86, 0X7F, 0XE7,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE7, 0XFE, 0X79,
                                                0X86, 0X7F, 0XE7, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE0, 0X00, 0X7F, 0X86, 0X00,
                                                0X07, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE0, 0X00,
                                                0X7F, 0X86, 0X00, 0X07, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XDF,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XDF, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE0, 0X1F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0X7F,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XE7,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XDB, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XDC, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XD8,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XDB, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XDB, 0X7F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE0, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XDF,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XDF, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE0, 0X1F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0X7F,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0X00, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0X00, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0X3F,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFC, 0XBF, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XF3,
                                                0XBF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0X8F, 0XBF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XBF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XC3,
                                                0XBF, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XBD, 0XBF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0XF7, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XBD, 0XBF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0X03, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XBD, 0XBF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XF7, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XD8, 0X7F, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0X37, 0XFF, 0XBF,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0XB7, 0XFF, 0XBF, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0XC7, 0XFF,
                                                0XC0, 0X1F, 0XFF, 0X80, 0X3F,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0XEF, 0XFF, 0XFF, 0X7F,
                                                0XFF, 0XFF, 0X7F, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0XFF, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XDF, 0XFF, 0XCF,
                                                0X7F, 0XFF, 0XFE, 0XFF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0XA7, 0XFF, 0XB7, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0XB7, 0XFF,
                                                0XBB, 0X7F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0X0F, 0XFF, 0XBB, 0X7F,
                                                0XFF, 0XEF, 0XFF, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XBF,
                                                0XFF, 0XBC, 0XFF, 0XFF, 0X80,
                                                0X3F, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFE, 0XF7, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XEE, 0XFF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0X03, 0XFF, 0XB8, 0XFF, 0XFF,
                                                0XED, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XF7, 0XFF,
                                                0XBB, 0X7F, 0XFF, 0XE7, 0XFF,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XBB, 0X7F,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XBB, 0X7F, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XBB,
                                                0X7F, 0XFF, 0XC4, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XC0, 0XFF, 0XFF,
                                                0XBB, 0XBF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XBB, 0XBF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XBF, 0X7F,
                                                0XFF, 0XBB, 0XBF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XBF, 0X7F, 0XFF, 0XC4,
                                                0X7F, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XC0,
                                                0X1F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XC0, 0X7F,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XB7, 0XBF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XB7,
                                                0XBF, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XB7, 0XBF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XD8, 0X7F, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0X00, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XEF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0X80, 0X3E, 0X00,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XEE, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XED, 0XFF,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XE7, 0XFF, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XEF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0X80, 0X3F,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XEE, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XED,
                                                0XFF, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE7, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XBC, 0X7F, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XBB,
                                                0XBF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XB7, 0XBF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFE,
                                                0XEF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XAF, 0XBF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0XEF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0X9E, 0X7F,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0X03, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XEF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XE0, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XC4, 0X7F, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0X6F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XBB, 0XBF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0XAF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XBB, 0XBF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0XAF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XBF, 0XBF, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFE, 0XDF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XDF,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0X9F, 0XFF,
                                                0XBF, 0X7F, 0XFF, 0XC0, 0X7F,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0XAF, 0XFF, 0XBF, 0X7F,
                                                0XFF, 0XBF, 0XBF, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFE, 0XAF,
                                                0XFF, 0XC0, 0X1F, 0XFF, 0XBF,
                                                0XBF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0X1F, 0XFF, 0XFF,
                                                0X7F, 0XFF, 0XBF, 0XBF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XC0, 0X7F, 0XE0, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFE, 0XEF, 0XFF,
                                                0XCF, 0X7F, 0XFF, 0XFF, 0XFF,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFE, 0XEF, 0XFF, 0XB7, 0X7F,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0X03,
                                                0XFF, 0XBB, 0X7F, 0XFF, 0XC0,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XEF, 0XFF, 0XBB,
                                                0X7F, 0XFF, 0XB7, 0XBF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XBC, 0XFF, 0XFF,
                                                0XB7, 0XBF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XB7, 0XBF,
                                                0XE0, 0X01, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XD8, 0X7F, 0XE0, 0X01,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XB8, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XBB,
                                                0X7F, 0XFF, 0XFF, 0XFF, 0XE0,
                                                0X01, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XBB, 0X7F, 0XFF,
                                                0XC4, 0X7F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XBB, 0X7F, 0XFF, 0XBB, 0XBF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XBB, 0X7F,
                                                0XFF, 0XBB, 0XBF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XC0, 0XFF, 0XFF, 0XBB,
                                                0XBE, 0X00, 0X01, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XC4, 0X7F, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XBF, 0X7F, 0XFF,
                                                0XFF, 0XFE, 0X00, 0X01, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XBF, 0X7F, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XC0, 0X1F,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0X7F, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
                                                0XFF };

const unsigned char gImage_LGTEST_RED1[2756] = { 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X05, 0X55, 0X00,
                                                 0X00, 0X00, 0XDF, 0XDE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XCF, 0X9E, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X05, 0X55,
                                                 0X00, 0X00, 0X00, 0XC3, 0X1E,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XF0, 0X7E, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X05,
                                                 0X55, 0X00, 0X00, 0X00, 0XF8,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XC0, 0X1E, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X05, 0X55, 0X00, 0X00, 0X00,
                                                 0XC0, 0X1E, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X05, 0X55, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XC0,
                                                 0X1E, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X05, 0X55, 0X00,
                                                 0X00, 0X00, 0XC0, 0X1E, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XE3, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XF1, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFC, 0X7E, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFE,
                                                 0X1E, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XC0, 0X1E, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XC0, 0X1E, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XCF, 0X9E, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XC0,
                                                 0X1E, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XC0, 0X1E, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XCF, 0X9E, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XC0, 0X1E, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XC0,
                                                 0X1E, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFC, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFC, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFC, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X20, 0X80,
                                                 0X00, 0XFC, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X20, 0X80, 0X00, 0XC0,
                                                 0X1E, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X1F,
                                                 0XE0, 0X00, 0XC0, 0X1E, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X80, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X18, 0X80, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X24,
                                                 0X80, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X23, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X27, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X24,
                                                 0X80, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X24, 0X80, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X1F, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X20, 0X80, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X20,
                                                 0X80, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X1F, 0XE0, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X80, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0XFF, 0XFE,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFF, 0XFE, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XFF,
                                                 0XFE, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XFF, 0XFE, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFF, 0XFE, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0X08, 0X00, 0X40, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X01, 0XFC, 0X00,
                                                 0X40, 0X80, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X08, 0X00, 0X3F, 0XE0,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0XC8, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0X48, 0X00, 0X30, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X01, 0X38, 0X00,
                                                 0X48, 0X80, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X01, 0X10, 0X00, 0X44, 0X80,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X44, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X20, 0X00, 0X43,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0X58, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X01, 0X48, 0X00,
                                                 0X47, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X01, 0XF0, 0X00, 0X44, 0X80,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X40,
                                                 0X00, 0X44, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X01, 0X08, 0X00, 0X44,
                                                 0X80, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0XFC, 0X00, 0X44, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X08, 0X00,
                                                 0X3F, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X40, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X40,
                                                 0X80, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X3F, 0XE0, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X80, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X01, 0X10,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X01, 0X10, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0XFC, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X10, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X90, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0X50, 0X00, 0X40, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X01, 0X50, 0X00,
                                                 0X40, 0X80, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X01, 0X20, 0X00, 0X3F, 0XE0,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0X60, 0X00, 0X30, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X01, 0X50, 0X00,
                                                 0X48, 0X80, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X01, 0X50, 0X00, 0X44, 0X80,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0XE0,
                                                 0X00, 0X44, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X43,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X01,
                                                 0X10, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X01, 0X10, 0X00,
                                                 0X47, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0XFC, 0X00, 0X44, 0X80,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X10,
                                                 0X00, 0X44, 0X80, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X44,
                                                 0X80, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X44, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X3F, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X40,
                                                 0X80, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X40, 0X80, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X3F, 0XE0, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X80,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00, 0X00, 0X00, 0X00, 0X00,
                                                 0X00 };

const unsigned char init_data[] = { 0xA5, 0x89, 0x10, 0x00, 0x00, 0x00, 0x00,
                                    0xA5, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00,
                                    0xA5, 0xA9, 0x9B, 0x00, 0x00, 0x00, 0x00,
                                    0xA5, 0xA9, 0x9B, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x0F, 0x0F, 0x0F, 0x0F, 0x02, 0x10, 0x10,
                                    0x0A, 0x0A, 0x03, 0x08, 0x08, 0x09, 0x43,
                                    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void say_UART(char message[]){
    UART_write(uart, message, strlen(message));
}

void SDA_H()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_SDA, 1);
    PIN_setOutputValue(ledPinHandle, Board_SDA, 1);
}
void SDA_L()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_SDA, 0);
    PIN_setOutputValue(ledPinHandle, Board_SDA, 0);
}
void SCLK_H()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_SCL, 1);
    PIN_setOutputValue(ledPinHandle, Board_SCL, 1);
}
void SCLK_L()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_SCL, 0);
    PIN_setOutputValue(ledPinHandle, Board_SCL, 0);
}
void nCS_H()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_CS, Board_GPIO_LED_ON);
    PIN_setOutputValue(ledPinHandle, Board_CS, 1);
}
void nCS_L()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_CS, Board_GPIO_LED_OFF);
    PIN_setOutputValue(ledPinHandle, Board_CS, 0);
}
void nDC_H()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_GLED, Board_GPIO_LED_ON);
    PIN_setOutputValue(ledPinHandle, Board_DC, 1);
}
void nDC_L()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_GLED, Board_GPIO_LED_OFF);
    PIN_setOutputValue(ledPinHandle, Board_DC, 0);
}
void nRST_H()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_RE, Board_GPIO_LED_ON);
    PIN_setOutputValue(ledPinHandle, Board_RESET, 1);
}
void nRST_L()
{
    //GPIO_write(CC1310_LAUNCHXL_PIN_RE, Board_GPIO_LED_OFF);
    PIN_setOutputValue(ledPinHandle, Board_RESET, 0);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xx   DELAY xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void DELAY_100nS(int delaytime)                         // 30us
{
    int i, j;

    for (i = 0; i < delaytime; i++)
        for (j = 0; j < 1; j++)
            ;
}

void DELAY_mS(int delaytime)                            // 1ms
{
    int i, j;

    for (i = 0; i < delaytime; i++)
        for (j = 0; j < 200; j++)
            ;
}

void DELAY_S(int delaytime)                             // 1s
{
    int i, j, k;

    for (i = 0; i < delaytime; i++)
        for (j = 0; j < 1000; j++)
            for (k = 0; k < 200; k++)
                ;

    // for(;log==1;);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xx    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// RESET xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void RESET()
{
    //say_UART("Сброс\r\n");
    nRST_L();
    //GPIO_write(CC1310_LAUNCHXL_PIN_RE, 0);
    //say_UART("off\r\n");
    DELAY_S(10);
    //say_UART("sleep done\r\n");
    nRST_H();
    //GPIO_write(CC1310_LAUNCHXL_PIN_RE, 1);
    //say_UART("on\r\n");
    //sleep(1);
    //say_UART("sleep done\r\n");
}

// READ BUSY xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void READBUSY()
{
    // while (GPIO_read(CC1310_LAUNCHXL_BUSY) == 1) {}

    while (1)
    {
        //bus1 = PIN_getInputValue(Board_BUSY);
        //say_UART("busy= "+ bus1);
        // _nop_();
        if (!PIN_getInputValue(Board_BUSY))
        {
            break;
        }
    }
}

// WRITE COMMAND  xxxxxxxxxxxxxxxxxxx
void test(void){
    say_UART("test!\r\n");
}
void spiWritecom(unsigned char INITCOM){
    //say_UART("Команда!\r\n");
    unsigned char TEMPCOM;
    unsigned char scnt;

    //transaction.count = 1;
    //transaction.txBuf = (void *)INIT_COM;

    TEMPCOM = INITCOM;
    //say_UART("Команда!\r\n");
    nDC_L();
    //say_UART("DC_L\r\n");
    nCS_H();
    //say_UART("CS_H\r\n");
    nCS_L();
    //say_UART("CS_L\r\n");

    /*masterSpi = SPI_open(Board_SPI_MASTER, &spiParams);
     if (masterSpi == NULL) {
     test=20;
     while (1);
     }
     else {
     test=21;
     }
     test=10;
     //SPI_transfer(masterSpi, &transaction);

     transferOK = SPI_transfer(masterSpi, &transaction);
     if (transferOK) {
     test=99;
     }
     else {
     test=100;
     }

     test=11;
     //SPI_close(masterSpi);
     test=12;*/
    for (scnt = 0; scnt < 8; scnt++)
    {
        if (TEMPCOM & 0x80)
            SDA_H();
        else
            SDA_L();
        SCLK_L();
        SCLK_H();
        TEMPCOM = TEMPCOM << 1;
    }
    nCS_H();
}

// WRITE DATA  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void SPI4W_WRITEDATA(unsigned char INIT_DATA)
{
    unsigned char TEMPCOM;
    unsigned char scnt;

    TEMPCOM = INIT_DATA;
    SCLK_H();
    nDC_H();
    nCS_H();
    nCS_L();
    //transaction.count = 1;
    //transaction.txBuf = (void *)INIT_DATA;
    //SPI_open(Board_SPI_MASTER, &spiParams);
    //SPI_transfer(masterSpi, &transaction);
    // SPI_close(masterSpi);
    for (scnt = 0; scnt < 8; scnt++)
    {
        if (TEMPCOM & 0x80)
            SDA_H();
        else
            SDA_L();
        SCLK_L();
        SCLK_H();
        TEMPCOM = TEMPCOM << 1;
    }
    nCS_H();
}

// WRITE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void WRITE_LUT()
{
    unsigned char i;

    spiWritecom(0x32);           // write LUT register
    for (i = 0; i < 70; i++) // write LUT register with 29bytes instead of 30bytes 2D13
        SPI4W_WRITEDATA(init_data[i]);
}

//void WriteFlow()
//{
//  unsigned char i;
//
//  SPI4W_WRITECOM(0x24);       // write RAM
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data0[i]);
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data1[i]);
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data2[i]);
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data3[i]);
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data4[i]);
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data5[i]);
//  for(i=0;i<32;i++)
//        SPI4W_WRITEDATA(init_data6[i]);
//  for(i=0;i<18;i++)
//        SPI4W_WRITEDATA(init_data7[i]);
//}

// init    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void INIT_SSD1675()
{
    spiWritecom(0x74);
    SPI4W_WRITEDATA(0x54);
    spiWritecom(0x75);
    SPI4W_WRITEDATA(0x3b);
    spiWritecom(0x01);       // Set MUX as 212
    SPI4W_WRITEDATA(0xD3);
    SPI4W_WRITEDATA(0x00);
    SPI4W_WRITEDATA(0x00);
    spiWritecom(0x3A);       // Set 100Hz
    //SPI4W_WRITEDATA(0x25);         // Set 100Hz
    SPI4W_WRITEDATA(0x18);         // Set 120Hz
    spiWritecom(0x3B);       // Set 100Hz
    //SPI4W_WRITEDATA(0x06);         // Set 100Hz
    SPI4W_WRITEDATA(0x05);         // Set 120Hz
    spiWritecom(0x11);       // data enter mode
    SPI4W_WRITEDATA(0x03);      ///01
    spiWritecom(0x44);       // set RAM x address start/end, in page 36
    SPI4W_WRITEDATA(0x00);      // RAM x address start at 00h;
    SPI4W_WRITEDATA(0x0c);      // RAM x address end at 0fh(12+1)*8->104
    spiWritecom(0x45);       // set RAM y address start/end, in page 37
    SPI4W_WRITEDATA(0x00);
    SPI4W_WRITEDATA(0x00);      // RAM y address startat 00h;
    SPI4W_WRITEDATA(0xD3);              // RAM y address end at 212
    SPI4W_WRITEDATA(0x00);
    SPI4W_WRITEDATA(0x00);
    spiWritecom(0x04);       // set VSH,VSL value
    //SPI4W_WRITEDATA(0x50);        //      2D13  18v
    //SPI4W_WRITEDATA(0x41);        //      2D13  15v
    SPI4W_WRITEDATA(0x2D);      //      2D13  11v
    // SPI4W_WRITEDATA(0xbf);        //      2D13   7.3v
    //  SPI4W_WRITEDATA(0xb8);        //      2D13   6.6v
    //SPI4W_WRITEDATA(0xb4);     //      2D13   6.2v
    SPI4W_WRITEDATA(0xb2);      //      2D13   6v
    // SPI4W_WRITEDATA(0xAE);        //      2D13   5.6v
    // SPI4W_WRITEDATA(0xac);     //      2D13   5.4v
    //SPI4W_WRITEDATA(0xa8);        //      2D13   5v
    //SPI4W_WRITEDATA(0xaa);        //      2D13   5.2v
    //SPI4W_WRITEDATA(0xA4);        //      2D13   4.6v
    // SPI4W_WRITEDATA(0xA2);        //      2D13   4.4v
    // SPI4W_WRITEDATA(0xa0);      //      2D13   4.2v
    // SPI4W_WRITEDATA(0x9C);      //      2D13   3.8v
    //  SPI4W_WRITEDATA(0x96);        //      2D9   3.8v
    // SPI4W_WRITEDATA(0x9A);      //      2D13   3.6v
    SPI4W_WRITEDATA(0x22);     //      2D13  -11v
    // SPI4W_WRITEDATA(0x32);        //      2D13  -15v
    // SPI4W_WRITEDATA(0x3E);        //      2D13  -18v
    spiWritecom(0x2C);           // vcom
    // SPI4W_WRITEDATA(0x78);           //-3V
    // SPI4W_WRITEDATA(0x6f);           //-2.8V
    //  SPI4W_WRITEDATA(0x6c);           //-2.7V
    //SPI4W_WRITEDATA(0x68);           //-2.6V
    // SPI4W_WRITEDATA(0x5F);           //-2.4V
    // SPI4W_WRITEDATA(0x50);           //-2V
    SPI4W_WRITEDATA(0x3C);           //-1.5V
    spiWritecom(0x3C);       // board
    SPI4W_WRITEDATA(0x33);      //GS1-->GS1
    WRITE_LUT();
}

// ÈëÉî¶ÈË¯Ãß xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void enterdeepsleep()
{
    spiWritecom(0x10);
    SPI4W_WRITEDATA(0x01);
}

void set_xy_window(unsigned char xs, unsigned char xe, unsigned int ys,
                   unsigned int ye)
{

    spiWritecom(0x44);       // set RAM x address start/end, in page 36
    SPI4W_WRITEDATA(xs);        // RAM x address start at 00h;
    SPI4W_WRITEDATA(xe);        // RAM x address end at 0fh(12+1)*8->104
    spiWritecom(0x45);       // set RAM y address start/end, in page 37
    SPI4W_WRITEDATA(ys);        // RAM y address start at 0;
    SPI4W_WRITEDATA(ys >> 8);
    SPI4W_WRITEDATA(ye);        // RAM y address end at
    SPI4W_WRITEDATA(ye >> 8);     // RAM y address end at
}

void set_xy_counter(unsigned char x, unsigned int y)
{
    spiWritecom(0x4E);       // set RAM x address count
    SPI4W_WRITEDATA(x);
    spiWritecom(0x4F);       // set RAM y address count
    SPI4W_WRITEDATA(y);
    SPI4W_WRITEDATA(y >> 8);
}

void Display_update(void)
{
    spiWritecom(0x22);
    SPI4W_WRITEDATA(0xC7);      //Load LUT from MCU(0x32), Display update
    spiWritecom(0x20);
//  DELAY_mS(1);
    READBUSY();
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xx   Í¼Æ¬ÏÔÊ¾º¯Êý    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void dis_img(unsigned char num)
{
    unsigned int row, col;
    unsigned int pcnt;
    /***************************************************bw image****************************************************/
    set_xy_window(0, 12, 0, 211);
    set_xy_counter(0, 0);
    spiWritecom(0x24);

    pcnt = 0;
    for (col = 0; col < 212; col++)
    {
        for (row = 0; row < 13; row++)
        {
            switch (num)
            {
            case PIC_A:

                SPI4W_WRITEDATA(gImage_mb_bw[pcnt]);

                break;

            case PIC_B:
                SPI4W_WRITEDATA(gImage_HStest_bw[pcnt]);
                break;

            case PIC_C:
                //SPI4W_WRITEDATA(pic_black[pcnt]);
                break;

            case PIC_R:
                SPI4W_WRITEDATA(0x00);
                break;
            case PIC_VLINE:
                if (col > 148)
                    SPI4W_WRITEDATA(0xff);
                else
                    SPI4W_WRITEDATA(0x00);
                break;

            case PIC_HLINE:
                if (row > 8)
                    SPI4W_WRITEDATA(0xff);
                else if (row == 8)
                    SPI4W_WRITEDATA(0x0f);
                else
                    SPI4W_WRITEDATA(0x00);
                /* if((row%2)<1)
                 SPI4W_WRITEDATA(0xff);

                 else
                 SPI4W_WRITEDATA(0x00);*/
                break;

            case PIC_WHITE:
                SPI4W_WRITEDATA(0xff);
                break;

            case PIC_BLACK:
                SPI4W_WRITEDATA(0x00);
                break;
            default:
                break;
            }
            pcnt++;
        }
    }
    /* if(num==PIC_BLACK||num==PIC_WHITE||num==PIC_HLINE||num==PIC_VLINE)
     {
     //  SPI4W_WRITECOM(0x21);
     //   SPI4W_WRITEDATA(0x04);
     SPI4W_WRITECOM(0x22);
     SPI4W_WRITEDATA(0xc7);
     SPI4W_WRITECOM(0x20);
     DELAY_mS(1);
     READBUSY();
     return;
     }
     */

    /***************************************************bw image****************************************************/
    set_xy_window(0, 12, 0, 211);
    set_xy_counter(0, 0);
    spiWritecom(0x26);

    pcnt = 0;
    for (col = 0; col < 212; col++)
    {
        for (row = 0; row < 13; row++)
        {
            switch (num)
            {
            case PIC_A:

                SPI4W_WRITEDATA(gImage_mb_red[pcnt]);
                break;

            case PIC_B:
                SPI4W_WRITEDATA(gImage_HStest_red[pcnt]);
                break;

            case PIC_C:
                //SPI4W_WRITEDATA(pic_red[pcnt]);
                break;

            case PIC_R:
                SPI4W_WRITEDATA(0xFF);
                break;
            case PIC_VLINE:
                SPI4W_WRITEDATA(0x00);
                break;
            case PIC_HLINE:

                SPI4W_WRITEDATA(0x00);

                break;

            case PIC_WHITE:
                SPI4W_WRITEDATA(0x00);
                break;

            case PIC_BLACK:
                SPI4W_WRITEDATA(0x00);
                break;
            default:
                break;
            }
            pcnt++;
        }
    }

    Display_update();

}

void dis_block(unsigned char xs, unsigned char xe, unsigned int ys,
               unsigned int ye, unsigned char dat, unsigned char mode)

{
    unsigned int row, col;
    unsigned int pcnt;
    /***************************************************bw image****************************************************/

    set_xy_window(xs, xe, ys, ye);

    set_xy_counter(xs, ye - ys);
    spiWritecom(0x24);

    pcnt = 0;
    for (col = ys; col <= ye; col++)
    {
        for (row = xs; row <= xe; row++)
        {

            SPI4W_WRITEDATA(dat);

        }
        pcnt++;

    }

    set_xy_window(xs, xe, ys, ye);
    set_xy_counter(xs, ye - ys);
    spiWritecom(0x26);

    pcnt = 0;
    for (col = ys; col <= ye; col++)
    {
        for (row = xs; row <= xe; row++)
        {
            if (mode == 0x26)
                SPI4W_WRITEDATA(dat);
            else
                SPI4W_WRITEDATA(0x00);
        }
        pcnt++;

    }

    Display_update();

}



/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void heartBeatFxn(UArg arg0, UArg arg1)
{
    /*while (1) {
        Task_sleep((UInt)arg0);
        PIN_setOutputValue(ledPinHandle, Board_RESET,
                           !PIN_getOutputValue(Board_RESET));
    }*/
    /* Open LED pins */



        //int i;
        //test = 0;
        //GPIO_init();
        //UART_init();
        //SPI_init();

        UART_Params_init(&uartParams);
        uartParams.writeDataMode = UART_DATA_BINARY;
        uartParams.readDataMode = UART_DATA_BINARY;
        uartParams.readReturnMode = UART_RETURN_FULL;
        uartParams.readEcho = UART_ECHO_OFF;
        uartParams.baudRate = 115200;
        uart = UART_open(Board_UART0, &uartParams);

        /*if (uart == NULL)
        {
            while (1)
                ;
        }*/

        //say_UART("UART работает!\r\n");

        ledPinHandle = PIN_open(&ledPinState, ledPinTable);
        if(!ledPinHandle) {
            say_UART("Error initializing board LED pins\n");
        }

        buttonPinHandle = PIN_open(&buttonPinState, buttonPinTable);
            if(!buttonPinHandle) {
                System_abort("Error initializing button pins\n");
            }
       PIN_setOutputValue(ledPinHandle, Board_LED0, 1);


        say_UART("Пины инициализированы!\r\n");

        //SPI_Params_init(&spiParams);
        //spiParams.frameFormat = SPI_POL0_PHA1;
        //test = 2;
        /*masterSpi = SPI_open(Board_SPI_MASTER, &spiParams);
         if (masterSpi == NULL) {
         test=20;
         while (1);
         }
         else {
         test=21;
         }*/

        //bus = GPIO_read(CC1310_LAUNCHXL_BUSY);
        //if (bus==0){say_UART("первый\r\n");}
        RESET();
        System_printf("Starting the UART Echo example\nSystem provider is set to "
                          "SysMin. Halt the target to view any SysMin contents in "
                          "ROV.\n");
            /* SysMin will only print to the console when you call flush or exit */
            System_flush();
        //bus = GPIO_read(CC1310_LAUNCHXL_BUSY);
        //if (bus==0){say_UART("второй\r\n");}
        spiWritecom(0x12);
        //bus = GPIO_read(CC1310_LAUNCHXL_BUSY);
        //if (bus==0){say_UART("третий\r\n");}
        //else{say_UART("писец\r\n");
        //}
        READBUSY();
        //test = 5;
        say_UART("ресет прошел!\r\n");

        INIT_SSD1675();

        say_UART("Дисплей пошел!\r\n");

        while (1)
        {

            dis_img(PIC_C);
            DELAY_S(3);

            spiWritecom(0x21);
            SPI4W_WRITEDATA(0x08);

            Display_update();
            DELAY_S(3);

            spiWritecom(0x21);
            SPI4W_WRITEDATA(0x00);

            dis_img(PIC_R);
            DELAY_S(2);

            /*    dis_img(PIC_BLACK);
             DELAY_S(2);

             dis_block(4,9,0+5,40+5, 0xff, 0x24);
             DELAY_S(2);

             dis_block(4,9,0+5,40+5, 0xff, 0x26);
             DELAY_S(2);

             dis_img(PIC_WHITE);
             DELAY_S(2);


             dis_block(0,4,0,40, 0x00, 0x24);
             DELAY_S(2);

             dis_block(0,4,0,40, 0xff, 0x26);
             DELAY_S(2);



             dis_img(PIC_R);
             DELAY_S(2);

             dis_block(0,4,0,40,  0x00, 0x24);
             DELAY_S(2);

             dis_block(0,4,0,40, 0xff, 0x24);
             DELAY_S(2);

             */

    //      dis_img(PIC_VLINE);
    //      DELAY_S(2);
    //      dis_img(PIC_HLINE);
    //      DELAY_S(2);

            dis_img(PIC_A);
            DELAY_S(2);

            dis_img(PIC_B);
            DELAY_S(2);

        }
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    // Board_initI2C();
    // Board_initSPI();
    Board_initUART();
    // Board_initWatchdog();

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000000 / Clock_tickPeriod;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);





    /* Start BIOS */
    BIOS_start();

    return (0);
}
