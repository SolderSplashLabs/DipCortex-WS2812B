/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     main.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     12/12/2013

    @section LICENSE

	Software License Agreement (BSD License)

    Copyright (c) 2013, C. Matthews - R. Steel (soldersplash.co.uk)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	@section DESCRIPTION

*/

#include "SolderSplashLpc.h"
#include "main.h"


// IF you use a transistor to level shift the coms, it will also invert it, so we invert the bit patterns

//#define INVERTED_COMS
#ifdef INVERTED_COMS

// These SPI bit patterns allow us to pack 4 bits for LED communication in to each 16bit SPI message
// This lets the SPI periperial handle the time sensitive stuff while we maintain it's FIFO
// Unfortunatly there is no DMA periperhal which we remove the processor overhead of filling the SPI
const uint16_t	LedBitPattern[16] =
{
		0x7777,
		0x7773,
		0x7737,
		0x7733,
		0x7377,
		0x7373,
		0x7337,
		0x7333,
		0x3777,
		0x3773,
		0x3737,
		0x3733,
		0x3377,
		0x3373,
		0x3337,
		0x3333
};

#else

// These SPI bit patterns allow us to pack 4 bits for LED communication in to each 16bit SPI message
// This lets the SPI peripheral handle the time sensitive stuff while we maintain it's FIFO
// Unfortunately there is no DMA peripheral which we remove the processor overhead of filling the SPI
const uint16_t	LedBitPattern[16] =
{
		0x8888,
		0x888C,
		0x88C8,
		0x88CC,
		0x8C88,
		0x8C8C,
		0x8CC8,
		0x8CCC,
		0xC888,
		0xC88C,
		0xC8C8,
		0xC8CC,
		0xCC88,
		0xCC8C,
		0xCCC8,
		0xCCCC
};

#endif

#define NO_OF_RGB_LEDS		120

uint32_t FrameBuffer[NO_OF_RGB_LEDS];


enum COLOUR_MODE_TYPE
{
	COLOUR_RAINBOW,
	COLOUR_BUTTON,
	COLOUR_CHRISTMAS,
	COLOUR_BUTTON_CENTRE,
	COLOUR_LOOPY,
	MODE_COUNT
};

uint8_t CurrentMode = COLOUR_RAINBOW;

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Generate a colour
*/
// ------------------------------------------------------------------------------------------------------------
uint32_t ColourWheel ( uint8_t wheelPos, uint8_t brightness )
{
uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 0;
uint32_t result = 0;

	// Divide the radius into thirds, 85 counts for each
	if(wheelPos < 85)
	{
		red = wheelPos * 3;
		green = 255 - wheelPos * 3;
		blue = 0;
	}
	else if( wheelPos < 170 )
	{
		wheelPos -= 85;
		red = 255 - wheelPos * 3;
		green = 0;
		blue = wheelPos * 3;
	}
	else
	{
		wheelPos -= 170;
		red = 0;
		green = wheelPos * 3;
		blue = 255 - wheelPos * 3;
	}

	green -= (green / brightness);
	red -= (red / brightness);
	blue -= (blue /brightness);

	result = green << 16;
	result += red << 8;
	result += blue;

	return ( result );
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Reduce the brightness of the supplied Pixel value
*/
// ------------------------------------------------------------------------------------------------------------
uint32_t DecreaseBrightness ( uint32_t pixel, uint8_t brightness )
{
uint8_t *outColour;
uint8_t i;

	// TODO : if I was doing it properly the frame buffer would use a structure datatype to allow easier access to each member/byte
	outColour = (uint8_t *)&pixel;

	for (i=0; i<3; i++)
	{
		if (outColour[i] > brightness)
		{
			outColour[i] -= brightness;
		}
		else
		{
			outColour[i] = 0;
		}
	}

	return(pixel);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Rotates a rainbow around the LED strip
*/
// ------------------------------------------------------------------------------------------------------------
void RainBowStep ( void )
{
static uint8_t step = 0;
static uint8_t brightnessLvl = 1;
uint8_t i = 0;

	if (!( LPC_GPIO->PIN[1] & (1<<28)))
	{
		if (brightnessLvl>1) brightnessLvl --;
	}


	if (!( LPC_GPIO->PIN[0] & (1<<7)))
	{
		if (brightnessLvl < 25) brightnessLvl ++;
	}

	for ( i=0; i<NO_OF_RGB_LEDS; i++ )
	{
		FrameBuffer[i] = ColourWheel(i+step, brightnessLvl);
	}

	step ++;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Make sure the SPI FIFO is not full, and add the bit pattern that matches the supplied 4bit nibble
*/
// ------------------------------------------------------------------------------------------------------------
void SpiNibble ( uint8_t nibble )
{
	// wait while the FIFO is full
	while ((LPC_SSP1->SR & SSPSR_TNF) != SSPSR_TNF);

	LPC_SSP1->DR = LedBitPattern[ 0x0f & nibble ];
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief  Loops around the frame buffer sending each nibble to be the SPI bus
*/
// ------------------------------------------------------------------------------------------------------------
void UpdateLedStrip_NibbleMode ( void )
{
uint32_t x = 0;
uint32_t currentColour = 0;

	for ( x=0; x<NO_OF_RGB_LEDS; x++ )
	{
		currentColour = FrameBuffer[x];

		// Masking off unwanted bits is handled in the nibble function

		// Top Nibble - Green
		SpiNibble( currentColour >> 20 );

		// Bottom Nibble - Green
		SpiNibble( currentColour >> 16 );

		// Top Nibble - Red
		SpiNibble( currentColour >> 12 );

		// Bottom Nibble - Red
		SpiNibble( currentColour >> 8 );

		// Top Nibble - Blue
		SpiNibble( currentColour >> 4 );

		// Bottom Nibble - Blue
		SpiNibble( currentColour );
	}

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief  Each button press add's that colour to the start of the strip, each call the framebuffer data is moved
    		from the start towards the end
*/
// ------------------------------------------------------------------------------------------------------------
void Pulse ( void )
{
uint8_t i = 0;

	for ( i=NO_OF_RGB_LEDS-1; i>0; i-- )
	{
		FrameBuffer[i] = FrameBuffer[i-1];
	}

	LPC_GPIO->DIR[1] &= ~(1<<28);
	LPC_GPIO->DIR[0] &= ~(1<<7);
	LPC_GPIO->DIR[1] &= ~(1<<23);
	LPC_GPIO->DIR[1] &= ~(1<<20);

	FrameBuffer[0] = 0;

	if (!( LPC_GPIO->PIN[1] & (1<<28)))
	{
		FrameBuffer[0] += 0x00FF00;
	}

	if (!( LPC_GPIO->PIN[0] & (1<<7)))
	{
		FrameBuffer[0] += 0xFF0000;
	}

	if (!( LPC_GPIO->PIN[1] & (1<<23)))
	{
		FrameBuffer[0] += 0xFF;
	}

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief  Each button press add's that colour to the centre of the strip, each call the framebuffer data is moved
    		from the middle to each end
*/
// ------------------------------------------------------------------------------------------------------------
void CentrePulse ( void )
{
uint8_t i = 0;
uint32_t newColour = 0;

	for ( i=0; i<(NO_OF_RGB_LEDS/2); i++ )
	{
		FrameBuffer[i] = FrameBuffer[i+1];
	}

	for ( i=NO_OF_RGB_LEDS-1; i>(NO_OF_RGB_LEDS/2); i-- )
	{
		FrameBuffer[i] = FrameBuffer[i-1];
	}

	LPC_GPIO->DIR[1] &= ~(1<<28);
	LPC_GPIO->DIR[0] &= ~(1<<7);
	LPC_GPIO->DIR[1] &= ~(1<<23);
	LPC_GPIO->DIR[1] &= ~(1<<20);

	newColour = 0;

	if (!( LPC_GPIO->PIN[1] & (1<<28)))
	{
		newColour += 0x00FF00;
	}

	if (!( LPC_GPIO->PIN[0] & (1<<7)))
	{
		newColour += 0xFF0000;
	}

	if (!( LPC_GPIO->PIN[1] & (1<<23)))
	{
		newColour += 0xFF;
	}

	FrameBuffer[(NO_OF_RGB_LEDS/2)] = newColour;
	FrameBuffer[(NO_OF_RGB_LEDS/2)-1] = newColour;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief  Button press adds that colour to the LED Strip once the LED value is about to be shifted of the end
    		it's sent back the beginning with it's brightness reduced.
*/
// ------------------------------------------------------------------------------------------------------------
void Loopy ( void )
{
uint8_t i = 0;
uint32_t nextColour = 0;

	nextColour = FrameBuffer[NO_OF_RGB_LEDS-1];

	for ( i=NO_OF_RGB_LEDS-1; i>0; i-- )
	{
		FrameBuffer[i] = FrameBuffer[i-1];
	}

	FrameBuffer[0] = DecreaseBrightness(nextColour, 25);

	if (!( LPC_GPIO->PIN[1] & (1<<28)))
	{
		FrameBuffer[0] += 0x00FF00;
	}

	if (!( LPC_GPIO->PIN[0] & (1<<7)))
	{
		FrameBuffer[0] += 0xFF0000;
	}

	if (!( LPC_GPIO->PIN[1] & (1<<23)))
	{
		FrameBuffer[0] += 0xFF;
	}

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief  Alternate LEDs Red and Green, fading up and down
*/
// ------------------------------------------------------------------------------------------------------------
void Christmas (void)
{
static uint8_t green = 255;
static uint8_t red = 0;
static bool greenUp = false;
static bool redUp = true;

	if (redUp)
	{
		red ++;
		if ( red == 0xff )
		{
			redUp = false;
		}
	}
	else
	{
		red --;
		if ( red == 0 )
		{
			redUp = true;
		}
	}

	if (greenUp)
	{
		green ++;
		if ( green == 0xff )
		{
			greenUp = false;
		}
	}
	else
	{
		green --;
		if ( green == 0 )
		{
			greenUp = true;
		}
	}

	uint8_t i = 0;

	for ( i=0; i<NO_OF_RGB_LEDS; i += 2 )
	{
		FrameBuffer[i] = red << 8;
		FrameBuffer[i+1] = green << 16;
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief  Each lighting mode gets called here to update the framebuffer, the buffer then gets sent to the
    		LEDs
*/
// ------------------------------------------------------------------------------------------------------------
void SysTick_Handler(void)
{
static uint32_t msCounter = 0;
static bool buttonWasPressed = false;

	msCounter += SYSTICKMS;

	if ( LPC_GPIO->PIN[1] & BIT20 )
	{
		// Button up
		if ( buttonWasPressed )
		{
			buttonWasPressed = false;
			CurrentMode ++;
			if ( CurrentMode > MODE_COUNT-1 )
			{
				CurrentMode = 0;
			}
		}
	}
	else
	{
		// Button down
		buttonWasPressed = true;
	}

	if (msCounter >= 50)
	{
		msCounter = 0;

		switch ( CurrentMode )
		{
			case COLOUR_RAINBOW :
				RainBowStep();
			break;

			case COLOUR_BUTTON :
				Pulse();
			break;

			case COLOUR_CHRISTMAS :
				Christmas();
			break;

			case COLOUR_BUTTON_CENTRE :
				CentrePulse();
			break;

			case COLOUR_LOOPY :
				Loopy();
			break;
		}

		//UpdateLedStrip();
		UpdateLedStrip_NibbleMode();
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief For my example I use 4 buttons
*/
// ------------------------------------------------------------------------------------------------------------
void InitButtons ( void )
{
	// Enable internal Pull Ups on all of the buttons
	LPC_IOCON->PIO1_28 = 0x30;
	LPC_IOCON->PIO0_7 = 0x30;
	LPC_IOCON->PIO1_23 = 0x30;
	LPC_IOCON->PIO1_20 = 0x30;

	LPC_GPIO->DIR[1] &= ~(1<<28);
	LPC_GPIO->DIR[0] &= ~(1<<7);
	LPC_GPIO->DIR[1] &= ~(1<<23);
	LPC_GPIO->DIR[1] &= ~(1<<20);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Set up SPI and Button inputs
*/
// ------------------------------------------------------------------------------------------------------------
void Init ( void )
{
	LpcLowPowerIoInit();

	SystemCoreClockUpdate();

	// Enable AHB clock to the GPIO domain.
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	// Enable AHB clock to the PinInt, GroupedInt domain and RAM1
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1<<19) | (1<<23) | (1<<24) | (1<<26));

	// SPI ----------------------------
	// Enable the SPI1 Clock
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<18);

	// Reset the SPI peripheral
	LPC_SYSCON->PRESETCTRL |= (0x1<<2);

	// Divide by 1 = Peripheral clock enabled
	LPC_SYSCON->SSP1CLKDIV = 1;

	// MOSI Enable - IO Config
	LPC_IOCON->PIO0_21 = 2;


	// SPI Clock Speed = Peripheral Clock / ( CPSR * (SCR+1) )
	// Clock Prescale Register - Must be even
	LPC_SSP1->CPSR = 2;

	// DSS  = 0x7, 8-bit
	// FRF  = 0, Frame format SPI
	// CPHA = 1, Data captured on falling edge of the clock
	// CPOL = 0, Clock Low between frames, and SCR is 3 ( 4clocks - 1 )
	// SCR = 2, 36Mhz/(2+1) = 12Mhz Clock rate
	LPC_SSP1->CR0 = 0 | 0xf | SSPCR0_SPH | 0x0D00;

#ifdef INVERTED_COMS
	// Set the SPI Bus to idle high, as it will be inverted to idle low
	LPC_SSP1->CR0 |= 1<<6;
#endif

	// SSE - Enabled, Master
	LPC_SSP1->CR1 = 0 | SSPCR1_SSE;

	InitButtons();

	// Set up the System Tick for a 1ms interrupt
	SysTick_Config(SYSTICK);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Main Loop - Nothing here
*/
// ------------------------------------------------------------------------------------------------------------
int main(void)
{
volatile static int i = 0;

	Init();

    while(1)
    {
        i++ ;
    }
    return 0 ;
}
