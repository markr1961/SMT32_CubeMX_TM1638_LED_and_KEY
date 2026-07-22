/*  Save this file using UTF-8 encoding

original author: Golinskiy Konstantin e-mail: golinskiy.konstantin@gmail.com

//---------------------------------------
CubeMX settings:

  Enable SPI in Half - Duplex Master mode (single wire for both receive and transmit)
  Change bit order :   FirstBit -> LSB First
  Set speed to low (max 1 Mbit / s)
  Set CPOL -> High
  Set CPHA -> 2 Edge

//---------------------------------------

  Pinout and connections :

  Interface : Half - Duplex SPI (Master) (single wire for both receive and transmit)

    VCC - +5V (tolerance : max 10 % )
    GND - GND
    STB - Chip Select (device selection)
    CLK - Clock line (SCK)
    DIO - Data In / Out (MOSI)

Module operating principle :
    This chip is a controller for a 7 - segment display (up to 10 digits) and a keypad (up to 8x3).
    It features 16 bytes of internal memory.
    Each byte controls the display of a single character or LED (depending on the PCB layout).
    In my case, the 8 odd - indexed bytes controlled the digits on the LED displays,
    while the least significant bit of each even - indexed byte controlled 8 individual LEDs.
    + There is a readable 32 - bit register storing the button states.
    + There is a configuration byte.

The chip is quite easy to control. There are three types of commands:
    control commands  0x8X
    data commands     0x4X
    address commands  0xCX

You can control two parameters here: brightness and display activation.
The lower nibble (lower 4 bits) of the byte is used for this purpose.
The first three bits are allocated for brightness, and the fourth for display control.
Consequently, there are 8 brightness levels.
To turn on the chip, send any of the following commands:
    0x88  Display ON, brightness PWM width 1 / 16
    0x89  Display ON, brightness PWM width 2 / 16
    0x8A  Display ON, brightness PWM width 4 / 16
    0x8B  Display ON, brightness PWM width 10 / 16
    0x8C  Display ON, brightness PWM width 11 / 16
    0x8D  Display ON, brightness PWM width 12 / 16
    0x8E  Display ON, brightness PWM width 13 / 16
    0x8F  Display ON, brightness PWM width 14 / 16
To turn it off, simply send any of the commands listed above with the 3rd bit cleared (set to 0).

Once powered on, you can write data to and read data from the chip's registers.
The 0x4X command is used to select the desired register operation;
As with the previous command, the lower nibble—specifically bits 1 and 2—controls the function.
    Bit 1: 1 - Read / 0 - Write
    Bit 2: 1 - Fixed address / 0 - Auto - increment address

    0x40  Write, auto - increment address
    0x44  Write, fixed address
    0x42  Read

Writing to the chip is possible in the following modes:
    1) Fixed address (command 0x44): the memory cell address is transmitted first, followed by the data byte.
    2) Auto - increment address (command 0x40): the memory cell address is transmitted first, followed by multiple data bytes (up to 16). Each subsequent byte is written to the next memory cell.

On / off control and brightness are set via command 0x80 using specific bits:
    1) On / off status is set by setting / clearing the 3rd bit.
    2) Brightness (range 0–7) is set using the 3 least significant bits.

A write operation to the chip consists of at least 2 bytes:
    1) Specifies the write address ( or the start address if auto - increment mode is used): 0xC0 + address (0–15).
       To conveniently iterate from 0 to 16, we need even addresses in one case and odd addresses in the other;
       therefore, the start address for LEDs is 0xC1 (0xC0 + 1), while for seven - segment displays it is 0xC0.
    2) The actual data byte.

Reading the keypad consists of 2 stages :
    1) Sending the keypad read command : 0x42.
    2) Reading 32 bits of data.

*/

//##########  SETUP  ##########################################################

// Specify the configuration settings in the LED_KEY_TM1638.h file

//==== Specify the SPI port ===================================================
#define   TM1638_SPI_HAL    hspi1

//=== Specify the ports (if you named them STB in Cube, no need to specify anything here)
#if defined (STB_GPIO_Port)
#else
#define STB_GPIO_Port   GPIOA
#define STB_Pin         GPIO_PIN_11
#endif
//=============================================================================

#include "LED_KEY_TM1638.h"

// Variable to store button data (each bit corresponds to a pressed key)
uint8_t keyPressed;

void main(void)
{

    //--------------------------------------------------
    // display initialization
    // backlight brightness from 0 to 7 (for LEDs and digits)
    TM1638_init( 0 );
    //--------------------------------------------------

    // clear (turn off all seven-segment displays)------
    //TM1638_Clear_SevenSegment();
    //--------------------------------------------------

    // clear (turn off all LEDs)------------------------
    //TM1638_Clear_Led();
    //--------------------------------------------------

    // clear everything completely: both LEDs and seven-segment displays
    TM1638_Clear_All();
    //--------------------------------------------------

    HAL_Delay (500);

    //--------------------------------------------------
    // turn on all LEDs one by one
    for( uint8_t i = 1; i <= 8; i++)
    {
        // turn LEDs on and off
        // first parameter: LED number from 1 to 8
        // second parameter: state LED_OFF or LED_ON
        TM1638_Led_OnOff( i, LED_ON );

        HAL_Delay (200);
    }
    //--------------------------------------------------

    //--------------------------------------------------
    // turn off all LEDs one by one
    for( uint8_t i = 1; i <= 8; i++)
    {

        // turn LEDs on and off
        // first parameter: LED number from 1 to 8
        // second parameter: state LED_OFF or LED_ON
        TM1638_Led_OnOff( i, LED_OFF );

        HAL_Delay (200);
    }

    //--------------------------------------------------
    // function to display a single-digit number (1 digit)
    // at the specified position (decimal point can also be enabled)
    // 1 - position (digit slot) to display the value (1 to 8)
    // 2 - the digit itself (0 to 9); use 11 for a minus sign
    // 3 - decimal point (1 to enable, 0 to disable)
    TM1638_sendOneDigit(1, 8, 0);
    //--------------------------------------------------

    HAL_Delay (2000);
    // clear (turn off all seven-segment displays)
    TM1638_Clear_SevenSegment();

    //--------------------------------------------------
    // function to display any number
    // string to be displayed:
    // digits 0 to 9
    // minus sign (multiple allowed)
    // decimal point (multiple allowed)
    // Examples: "-10.56", "0.56", "-2 -2.3", "1.2.3.4.", "-23-", etc.
    // TM1638_sendNumber("-2435.67");
    //--------------------------------------------------

    HAL_Delay (2000);
    // clear (turn off all seven-segment displays)
    TM1638_Clear_SevenSegment();

    // display numbers 0 to 9 in a loop
    for( int32_t i = 0; i < 10; i++ )
    {

        TM1638_Led_OnOff( 1, LED_ON );

        // clear (turn off all seven-segment displays)
        TM1638_Clear_SevenSegment();
        //------------------------------------------------
        // function to display an integer (INT) number
        // integer value:
        // range from -9,999,999 to 99,999,999
        TM1638_sendNumberInt( i );
        //------------------------------------------------

        HAL_Delay (500);

        TM1638_Led_OnOff( 1, LED_OFF );

        HAL_Delay (500);

    }

    HAL_Delay (2000);
    // clear display (turn off all seven-segment indicators)
    TM1638_Clear_SevenSegment();

///////////////////////////////////////////////////////////////////////////////

    while(1)
    {
        ///////////////////////////////////////////////////////////////////////////

        //-- turn on the LED corresponding to the pressed button ------------------
        // continuously read the button states
        // (bit corresponds to the pressed key)
        keyPressed = TM1638_readKey();

        // variable used as a mask to check the bits
        uint8_t num_key = 1;

        // iterate through all 8 buttons; if a button is pressed,
        // turn on the corresponding LED; otherwise, turn it off
        for( uint8_t i = 1; i <= 8; i++)
        {

            if( keyPressed & num_key )
            {
                TM1638_Led_OnOff( i, LED_ON );
            }
            else
            {
                TM1638_Led_OnOff( i, LED_OFF );
            }

            // shift to the next bit
            num_key = num_key << 1;
        }
        //------------------------------------------------

        // Simultaneously display the number of seconds elapsed since microcontroller startup

        // Clear (turn off all seven-segment displays)
        //TM1638_Clear_SevenSegment();

        //------------------------------------------------
        // Function to display an integer value
        // Integer range:
        // -9,999,999 to 99,999,999
        TM1638_sendNumberInt( HAL_GetTick() / 1000 );
        //------------------------------------------------

        //////////////////////////////////////////////////////////////////////////////////////////////

    } //== = end while(1) = ==

} //  == = end main() = ==
