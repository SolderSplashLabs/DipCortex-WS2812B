//*****************************************************************************
// LPC13Uxx Microcontroller Startup code for use with LPCXpresso IDE
//
// Version : 130808
//*****************************************************************************
//
// Copyright(C) NXP Semiconductors, 2013
// All rights reserved.
//
// Software that is described herein is for illustrative purposes only
// which provides customers with programming information regarding the
// LPC products.  This software is supplied "AS IS" without any warranties of
// any kind, and NXP Semiconductors and its licensor disclaim any and
// all warranties, express or implied, including all implied warranties of
// merchantability, fitness for a particular purpose and non-infringement of
// intellectual property rights.  NXP Semiconductors assumes no responsibility
// or liability for the use of the software, conveys no license or rights under any
// patent, copyright, mask work right, or any other intellectual property rights in
// or to any products. NXP Semiconductors reserves the right to make changes
// in the software without notification. NXP Semiconductors also makes no
// representation or warranty that such application will be suitable for the
// specified use without further testing or modification.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation is hereby granted, under NXP Semiconductors' and its
// licensor's relevant copyrights in the software, without fee, provided that it
// is used in conjunction with NXP Semiconductors microcontrollers.  This
// copyright, permission, and disclaimer notice must appear in all copies of
// this code.
//*****************************************************************************

#if defined (__cplusplus)
#ifdef __REDLIB__
#error Redlib does not support C++
#else
//*****************************************************************************
//
// The entry point for the C++ library startup
//
//*****************************************************************************
extern "C" {
    extern void __libc_init_array(void);
}
#endif
#endif

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

#if defined (__USE_CMSIS) || defined (__USE_LPCOPEN)
void SystemInit(void);
#endif

//*****************************************************************************
#if defined (__cplusplus)
extern "C" {
#endif

//*****************************************************************************
//
// Forward declaration of the default handlers. These are aliased.
// When the application defines a handler (with the same name), this will 
// automatically take precedence over these weak definitions
//
//*****************************************************************************
void ResetISR(void);
WEAK void NMI_Handler(void);
WEAK void HardFault_Handler(void);
WEAK void MemManage_Handler(void);
WEAK void BusFault_Handler(void);
WEAK void UsageFault_Handler(void);
WEAK void SVC_Handler(void);
WEAK void DebugMon_Handler(void);
WEAK void PendSV_Handler(void);
WEAK void SysTick_Handler(void);
WEAK void IntDefaultHandler(void);
//*****************************************************************************
//
// Forward declaration of the specific IRQ handlers. These are aliased
// to the IntDefaultHandler, which is a 'forever' loop. When the application
// defines a handler (with the same name), this will automatically take 
// precedence over these weak definitions
//
//*****************************************************************************
void PIN_INT0_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT2_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT3_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT4_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT5_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT6_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIN_INT7_IRQHandler(void) ALIAS(IntDefaultHandler);
void GINT0_IRQHandler(void) ALIAS(IntDefaultHandler);
void GINT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP1_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP0_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB_IRQHandler(void) ALIAS(IntDefaultHandler);
void USB_FIQHandler(void) ALIAS(IntDefaultHandler);
void ADC_IRQHandler(void) ALIAS(IntDefaultHandler);
void WDT_IRQHandler(void) ALIAS(IntDefaultHandler);
void BOD_IRQHandler(void) ALIAS(IntDefaultHandler);
void FMC_IRQHandler(void) ALIAS(IntDefaultHandler);
void OSCFAIL_IRQHandler(void) ALIAS(IntDefaultHandler);
void PVTCIRCUIT_IRQHandler(void) ALIAS(IntDefaultHandler);
void USBWakeup_IRQHandler(void) ALIAS(IntDefaultHandler);
// NOTE
//   Standard interrupt handler names used within LPCOpen are slightly
//   different to NXP codebundle / peripheral driver library style names.
//   Hence use LPCOpen names if appropriate for following...
#if defined (__USE_LPCOPEN)
void RIT_IRQHandler (void) ALIAS(IntDefaultHandler);
void TIMER16_0_IRQHandler (void) ALIAS(IntDefaultHandler);
void TIMER16_1_IRQHandler (void) ALIAS(IntDefaultHandler);
void TIMER32_0_IRQHandler (void) ALIAS(IntDefaultHandler);
void TIMER32_1_IRQHandler (void) ALIAS(IntDefaultHandler);
void UART_IRQHandler (void) ALIAS(IntDefaultHandler);
#else
void OSTIMER_IRQHandler(void) ALIAS(IntDefaultHandler);
void CT16B0_IRQHandler(void) ALIAS(IntDefaultHandler);
void CT16B1_IRQHandler(void) ALIAS(IntDefaultHandler);
void CT32B0_IRQHandler(void) ALIAS(IntDefaultHandler);
void CT32B1_IRQHandler(void) ALIAS(IntDefaultHandler);
void USART_IRQHandler(void) ALIAS(IntDefaultHandler);
#endif
//*****************************************************************************
//
// The entry point for the application.
// __main() is the entry point for Redlib based applications
// main() is the entry point for Newlib based applications
//
//*****************************************************************************
#if defined (__REDLIB__)
extern void __main(void);
#else
extern int main(void);
#endif
//*****************************************************************************
//
// External declaration for the pointer to the stack top from the Linker Script
//
//*****************************************************************************
extern void _vStackTop(void);

//*****************************************************************************
#if defined (__cplusplus)
} // extern "C"
#endif
//*****************************************************************************
//
// The vector table.  Note that the proper constructs must be placed on this to
// ensure that it ends up at physical address 0x0000.0000.
//
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    // Core Level - CM3
        &_vStackTop,            //  initial stack pointer
        ResetISR,               // The reset handler
        NMI_Handler,            // The NMI handler
        HardFault_Handler,      // The hard fault handler
        MemManage_Handler,      // The MPU fault handler
        BusFault_Handler,       // The bus fault handler
        UsageFault_Handler,     // The usage fault handler
        0,                      // Reserved
        0,                      // Reserved
        0,                      // Reserved
        0,                      // Reserved
        SVC_Handler,            // SVCall handler
        DebugMon_Handler,       // Debug monitor handler
        0,                      // Reserved
        PendSV_Handler,         // The PendSV handler
        SysTick_Handler,        // The SysTick handler

        // LPC13U External Interrupts
        PIN_INT0_IRQHandler,    // All GPIO pin can be routed ...
        PIN_INT1_IRQHandler,    // ... to PIN_INTx
        PIN_INT2_IRQHandler,
        PIN_INT3_IRQHandler,
        PIN_INT4_IRQHandler,
        PIN_INT5_IRQHandler,
        PIN_INT6_IRQHandler,
        PIN_INT7_IRQHandler,
        GINT0_IRQHandler,
        GINT1_IRQHandler,       // PIO0 (0:7)
        0,
        0,

#if defined (__USE_LPCOPEN)
        RIT_IRQHandler,
#else
        OSTIMER_IRQHandler,
#endif

        0,
        SSP1_IRQHandler,        // SSP1
        I2C_IRQHandler,         //  I2C

#if defined (__USE_LPCOPEN)
        TIMER16_0_IRQHandler,   // 16-bit Counter-Timer 0
	      TIMER16_1_IRQHandler,   // 16-bit Counter-Timer 1
	      TIMER32_0_IRQHandler,   // 32-bit Counter-Timer 0
	      TIMER32_1_IRQHandler,   // 32-bit Counter-Timer 1
#else
        CT16B0_IRQHandler,      // 16-bit Counter-Timer 0
        CT16B1_IRQHandler,      // 16-bit Counter-Timer 0
        CT32B0_IRQHandler,      // 16-bit Counter-Timer 0
        CT32B1_IRQHandler,      // 16-bit Counter-Timer 0
#endif

        SSP0_IRQHandler,        // SSP0

#if defined (__USE_LPCOPEN)
        UART_IRQHandler,        // UART
#else
        USART_IRQHandler,       // USART
#endif

        USB_IRQHandler,         // USB IRQ
        USB_FIQHandler,         // USB FIQ
        ADC_IRQHandler,         // A/D Converter
        WDT_IRQHandler,         // Watchdog timer
        BOD_IRQHandler,         // Brown Out Detect
        FMC_IRQHandler,         // IP2111 Flash Memory Controller
        OSCFAIL_IRQHandler,     // OSC FAIL
        PVTCIRCUIT_IRQHandler,  // PVT CIRCUIT
        USBWakeup_IRQHandler,   // USB wake up
        0,
    };

//*****************************************************************************
// Functions to carry out the initialization of RW and BSS data sections. These
// are written as separate functions rather than being inlined within the
// ResetISR() function in order to cope with MCUs with multiple banks of
// memory.
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void data_init(unsigned int romstart, unsigned int start, unsigned int len) {
    unsigned int *pulDest = (unsigned int*) start;
    unsigned int *pulSrc = (unsigned int*) romstart;
    unsigned int loop;
    for (loop = 0; loop < len; loop = loop + 4)
        *pulDest++ = *pulSrc++;
}

__attribute__ ((section(".after_vectors")))
void bss_init(unsigned int start, unsigned int len) {
    unsigned int *pulDest = (unsigned int*) start;
    unsigned int loop;
    for (loop = 0; loop < len; loop = loop + 4)
        *pulDest++ = 0;
}

//*****************************************************************************
// The following symbols are constructs generated by the linker, indicating
// the location of various points in the "Global Section Table". This table is
// created by the linker via the Code Red managed linker script mechanism. It
// contains the load address, execution address and length of each RW data
// section and the execution and length of each BSS (zero initialized) section.
//*****************************************************************************
extern unsigned int __data_section_table;
extern unsigned int __data_section_table_end;
extern unsigned int __bss_section_table;
extern unsigned int __bss_section_table_end;

//*****************************************************************************
// Reset entry point for your code.
// Sets up a simple runtime environment and initializes the C/C++
// library.
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void
ResetISR(void) {

    //
    // Copy the data sections from flash to SRAM.
    //
    unsigned int LoadAddr, ExeAddr, SectionLen;
    unsigned int *SectionTableAddr;

    // Load base address of Global Section Table
    SectionTableAddr = &__data_section_table;

    // Copy the data sections from flash to SRAM.
    while (SectionTableAddr < &__data_section_table_end) {
        LoadAddr = *SectionTableAddr++;
        ExeAddr = *SectionTableAddr++;
        SectionLen = *SectionTableAddr++;
        data_init(LoadAddr, ExeAddr, SectionLen);
    }
    // At this point, SectionTableAddr = &__bss_section_table;
    // Zero fill the bss segment
    while (SectionTableAddr < &__bss_section_table_end) {
        ExeAddr = *SectionTableAddr++;
        SectionLen = *SectionTableAddr++;
        bss_init(ExeAddr, SectionLen);
    }

#if defined (__USE_CMSIS) || defined (__USE_LPCOPEN)
    SystemInit();
#endif

#if defined (__cplusplus)
    //
    // Call C++ library initialisation
    //
    __libc_init_array();
#endif

#if defined (__REDLIB__)
    // Call the Redlib library, which in turn calls main()
    __main() ;
#else
    main();
#endif
    //
    // main() shouldn't return, but if it does, we'll just enter an infinite loop
    //
    while (1) {
        ;
    }
}

//*****************************************************************************
// Default exception handlers. Override the ones here by defining your own
// handler routines in your application code.
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void NMI_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void HardFault_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void MemManage_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void BusFault_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void UsageFault_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void SVC_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void DebugMon_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void PendSV_Handler(void) {
    while (1) {
    }
}
__attribute__ ((section(".after_vectors")))
void SysTick_Handler(void) {
    while (1) {
    }
}

//*****************************************************************************
//
// Processor ends up here if an unexpected interrupt occurs or a handler
// is not present in the application code.
//
//*****************************************************************************
__attribute__ ((section(".after_vectors")))
void IntDefaultHandler(void) {
    //
    // Go into an infinite loop.
    //
    while (1) {
    }
}
