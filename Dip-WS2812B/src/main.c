
#include "SolderSplashLpc.h"
#include "main.h"

#define LED_1		0xF8
#define LED_0		0xC0

// Using a transistor to level shift, it also inverts, so we invert the bit patterns
//#define INVERTED_COMS
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

/*
uint32_t FrameBuffer[] = { \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
		0, 0, 0, 0, 0, 0, 0, 0};
*/


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

uint32_t DecreaseBrightness ( uint32_t pixel, uint8_t brightness )
{
uint8_t *outColour;
uint8_t i;

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

void SpiNibble ( uint8_t nibble )
{
	// wait while the FIFO is full
	while ((LPC_SSP1->SR & SSPSR_TNF) != SSPSR_TNF);

	LPC_SSP1->DR = LedBitPattern[ 0x0f & nibble ];
}


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


void StepMode ( void )
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

void SingleStripRainbow(void)
{
uint8_t i = 0;
static uint8_t brightness = 1;

	for ( i=0; i<NO_OF_RGB_LEDS; i++ )
	{
		FrameBuffer[i] = ColourWheel(i, brightness);

	}

	brightness++;
	if ( brightness > 3 ) brightness = 1;
}

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
				StepMode();
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

// For my example I use 4 buttons
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

	// SSE - Enabled, Master
	LPC_SSP1->CR1 = 0 | SSPCR1_SSE;

	InitButtons();

	// Set up the System Tick for a 1ms interrupt
	SysTick_Config(SYSTICK);
}

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
