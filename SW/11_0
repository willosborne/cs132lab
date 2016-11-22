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

#define PIO_PDSR    (ioreg *) 0xfffff43c

//#define RED_LED		0x00000004 // 00100
#define RED_LED     (1 << 4)
#define AMBER_LED   (1 << 3)
#define GREEN_LED   (1 << 2)

#define MOTOR_PULSE             (1 << 0)
#define MOTOR_DIRECTION         (1 << 1)

#define REDAMBER RED_LED|AMBER_LED
#define REDGREEN RED_LED|GREEN_LED
#define AMBERGREEN AMBER_LED|GREEN_LED

#define GREEN_SWITCH   (1 << 20)
#define RED_SWITCH     (1 << 30)
#define OPTO_DETECTOR  (1 << 25)

//#define AMBER_LED   0x00000008 // 01000
//#define GREEN_LED   0x00000010 // 10000
//#define REDAMBERGREEN    RED_LED|AMBER_LED|GREEN_LED
#define DELAY 		0x02800000	// Long enough to see a change in LED

#define PIO_IDENTIFIER 0x00000004

#define OUTPUTS RED_LED|AMBER_LED|GREEN_LED|MOTOR_PULSE|MOTOR_DIRECTION
#define INPUTS GREEN_SWITCH|RED_SWITCH|OPTO_DETECTOR
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

    int flipDirection = 0;
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
        delay(0x000F0000);

        //*PIO_SODR = MOTOR_DIRECTION;
        *PIO_SODR = MOTOR_PULSE;
        delay(0x000F0000);

        /*
         *  01000000  OPTO_DETECTOR (saying it's bit 7)
         *  01011011  PIO_PDSR    -  status of all the input pins
         * &
         *  01000000 = opto_detector. if this is true, you know the bit for the detector in the PDSR is on.
         *
         *  in this case opto_detector is active low so we check for the bit being NOT equal to opto_detector.
         *  i.e. if bit for opto_detector in PDSR is 0.
         */
        if ((*PIO_PDSR & OPTO_DETECTOR) != OPTO_DETECTOR || (*PIO_PDSR & RED_SWITCH) != RED_SWITCH) {
            if (flipDirection) {
                *PIO_CODR = MOTOR_DIRECTION;
            }

            else {
                *PIO_SODR = MOTOR_DIRECTION;
            }

            flipDirection = !flipDirection;                
        }

    }
}

void delay(int count)
{
    register int i;
    for (i=count;i>0;i--)
        ;
}
