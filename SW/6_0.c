typedef volatile unsigned int ioreg;

// register containing which pins we want to enable
#define	PIO_PER		(ioreg *) 0xfffff400	// PIO Enable Register

// register containing which pins are to be outputs
#define	PIO_OER	  	(ioreg *) 0xfffff410	// Output Enable Register

// register containing which pins are to be filtered for input (noise elimination)
#define PIO_IFER    (ioreg *) 0xFFFFF420

// clock enable
#define PMC_PCER    (ioreg *) 0xFFFFFC10

// register containing which outputs we want to switch on
#define	PIO_SODR  	(ioreg *) 0xfffff430	// Set Output Data Register

// register containing which outputs we want to switch off
#define	PIO_CODR  	(ioreg *) 0xfffff434	// Clear Output Data Register

//#define RED_LED		0x00000004 // 00100
#define RED_LED     (1 << 4)
#define AMBER_LED   (1 << 3)
#define GREEN_LED   (1 << 2)

#define MOTOR_PULSE             (1 << 0)
#define MOTOR_DIRECTION         (1 << 1)

#define REDAMBER RED_LED|AMBER_LED
#define REDGREEN RED_LED|GREEN_LED
#define AMBERGREEN AMBER_LED|GREEN_LED


//#define AMBER_LED   0x00000008 // 01000
//#define GREEN_LED   0x00000010 // 10000
//#define REDAMBERGREEN    RED_LED|AMBER_LED|GREEN_LED
#define DELAY 		0x02800000	// Long enough to see a change in LED

#define PIO_IDENTIFIER 0x00000004

#define OUTPUTS RED_LED|AMBER_LED|GREEN_LED|MOTOR_PULSE|MOTOR_DIRECTION
#define INPUTS 0
/*
 * ||
 * | 
 *
 * 1 || 0 == 1
 *
 * 0000 | 0101 = 0101
 * 1101 | 0010 == 1111
 * 0100 | 1000 == 1100
 */

/*
 * traffic lights:
 * 1. red
 * 2. red-amber
 * 3. green
 * 4. amber
 */

void delay(int);

int main(void)
{
    //int INPUTS = 0;

    //*PIO_PER = RED_LED; // Enable control of I/O pin from PIO Controller
    //*PIO_OER = RED_LED; // Enable output driver for pin
    *PIO_PER = OUTPUTS|INPUTS; // Enable control of I/O pin from PIO Controller
    *PIO_OER = OUTPUTS; // Enable output driver for pin
    *PIO_IFER = INPUTS; // Turn on filter for inputs
    *PMC_PCER = PIO_IDENTIFIER; //enable the clock (regularly check if the inputs have changed -i.e. enable inputs)

    *PIO_CODR = OUTPUTS; //clear all outputs

    //int PIN_6 = (1 << 5); // take 00000001, shift it left 5 times -> 0100000

    *PIO_CODR = MOTOR_DIRECTION;
    //blink red and green leds
    while (1) {
        // turn on red, turn off amber and green
        /* *PIO_CODR = AMBERGREEN; */
        /* *PIO_SODR = RED_LED; */
        /* delay(DELAY); */

        /* // turn on red and amber, turn off green */
        /* *PIO_SODR = REDAMBER; */
        /* *PIO_CODR = GREEN_LED; */
        /* delay(DELAY); */

        /* // turn off red and amber, turn on green */
        /* *PIO_CODR = RED_LED | AMBER_LED; */
        /* *PIO_SODR = GREEN_LED; */
        /* delay(DELAY); */

        /* // turn off green and red, turn on amber */
        /* *PIO_CODR = GREEN_LED | RED_LED; */
        /* *PIO_SODR = AMBER_LED; */
        /* delay(DELAY); */
        //*PIO_CODR = MOTOR_DIRECTION;
        *PIO_CODR = MOTOR_PULSE;
        delay(0x00880000);

        //*PIO_SODR = MOTOR_DIRECTION;
        *PIO_SODR = MOTOR_PULSE;
        delay(0x00880000);

    }
}

void delay(int count)
{
    register int i;
    for (i=count;i>0;i--)
        ;
}
