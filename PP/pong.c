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


/*
 * ---------------- NEW DEFINITIONS -----------------------
 */

#define PIO_PDR (ioreg *) 0xfffff404 // PIO disable register
#define PIO_ASR (ioreg *) 0xfffff470 // PIO A select register

#define SPI_CR   (ioreg *) 0xfffe0000 // SPI Control Register
#define SPI_MR   (ioreg *) 0xfffe0004 // SPI Mode Register
#define SPI_SR   (ioreg *) 0xfffe0010 // SPI status register
#define SPI_TDR  (ioreg *) 0xfffe000c // SPI Transmit Data Register
#define SPI_CSR0 (ioreg *) 0xfffe0030 // SPI Chip Select Register 0



#define ADC_CR   (ioreg *) 0xfffd8000 // ADC control register
#define ADC_MR   (ioreg *) 0xfffd8004 // ADC mode register
#define ADC_CHER (ioreg *) 0xfffd8010 // ADC channel enable register
#define ADC_SR   (ioreg *) 0xfffd801c // ADC status register
#define ADC_CDR4 (ioreg *) 0xfffd8040 // ADC channel 4 data register
#define ADC_CDR5 (ioreg *) 0xfffd8044 // ADC channel 5 data register

/*
 * -----------------------------------------------------------
 *
 */


// {{{
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

#define PIO_IDENTIFIER 0x34 // 0b110100


#define WRITE_B  0x4000 //0b0100000000000000;
#define WRITE_A  0xc000 //0b1100000000000000;

void waitForSPI(void);
void setX(int);
void setY(int);

int getInputA(void);
int getInputB(void);

//#define OUTPUTS RED_LED|AMBER_LED|GREEN_LED|MOTOR_PULSE|MOTOR_DIRECTION
#define INPUTS RED_SWITCH


// }}}

int ballX = 512;
int ballY = 512;
int ballvX = 5;
int ballvY = 2;
int player1Input = 0;
int player2Input = 0;
int paddleSize = 100;

int main(void)
{
    /*
     * NOTES:
     * 
     * DAC chip has two outputs, A and B, which store 10 bits of data.
     * It also has a 10-bit buffer for storage.
     * A and B go to X and Y respectively.
     *
     *
     */

    /*
     * SETUP
     */

    LowLevelInit(); //setup
    *PMC_PCER = PIO_IDENTIFIER;
    *PIO_PDR = 0x7800; // disable bits 11-14
    *PIO_ASR = 0x7800;  // peripheral A is using 11-14
    *SPI_CR = 0x80; // reset serial interface
    *SPI_CR = 0x1; // enable it

    /* 
     * CONFIGURE SPI
     */

    *SPI_MR = 0x11; // spi to master mode, turn off fault detection
    *SPI_CSR0 = 0x183; //clock polarity = 1, clock phase = 1, bits/transfer 16, baud raite = 1

    *SPI_TDR = 0xd002; // reference voltage to 2

    waitForSPI();


    /*
     * 0xd002 in binary is 1101 0000 0000 0010
     * Format is R1, SPD, PWR, R0, [12 bits of data].
     * i.e. R1 = 1,
     * SPD = 1 (fast mode)
     * PWR = 0 (don't switch off)
     * R0 = 1
     *
     * R0, R1 = 1 means "write to control register."
     * So we write 2 to control register meaning set ref. voltage to 2.
     */

    //INPUT SETUP

    *ADC_CR = 0x1; //reset ADC
    *ADC_CHER = 0x30; // enable channels 4 and 5

    *ADC_MR = 0x030b0400; // sample+holdtime = 3, startup = b, prescale = 4

    //*PIO_PER = INPUTS; // Enable control of I/O pin from PIO Controller
    //*PIO_OER = OUTPUTS; // Enable output driver for pin
    //*PIO_IFER = INPUTS; // Turn on filter for inputs

    
    //int WRITE_B = 0x4000; //0b0100000000000000;
    //int WRITE_A = 0xc000; //0b1100000000000000;
    //*SPI_TDR = WRITE_TO_B_AND_BUFFER; // write 32 to buffer and B
    //*SPI_TDR = WRITE_TO_A_UPDATE_B; // write 32 to A

    //*SPI_TDR = WRITE_A | (0 << 2);

    //waitForSPI();

    while(1) {
        //draw bounding box
        register int i = 0;
        setX(50);
        for (i = 0; i < 1023; i++) {
            setY(i);
        }
        setX(950);
        for (i = 0; i < 1023; i++) {
            setY(i);
        }
        setY(50);
        for (i = 0; i < 1023; i++) {
            setX(i);
        }
        setY(950);
        for (i = 0; i < 1023; i++) {
            setX(i);
        }

        //get input from players
        player1Input = getInputA();
        player2Input = getInputB();

        //draw paddles

        setX(75);
        //setY()
        
        

        // move ball by velocity
        ballX += ballvX;
        ballY += ballvY;

        if(ballY > 950 || ballY < 50)
            ballvY = -ballvY;
        if(ballX > 950)
            ballvX = -ballvX;
        if(ballX < 50)
            ballvX = -ballvX;



        //y *= 2;
        
        setX(ballX);
        setY(ballY);

        delay(0x8000);

        /* if ((*PIO_PDSR & RED_SWITCH) != RED_SWITCH) { */
        /*     continue; */
        /* } */    
    }
    // 0xd### writes the bits contained in ### to the control register in fast mode.

    //int INPUTS = 0;

    //*PIO_PER = RED_LED; // Enable control of I/O pin from PIO Controller
    //*PIO_OER = RED_LED; // Enable output driver for pin
    /* *PIO_PER = OUTPUTS|INPUTS; // Enable control of I/O pin from PIO Controller */
    //*PIO_OER = OUTPUTS; // Enable output driver for pin
    /* *PIO_IFER = INPUTS; // Turn on filter for inputs */
    //*PMC_PCER = PIO_IDENTIFIER; //enable the clock (regularly check if the inputs have changed -i.e. enable inputs)

    //*PIO_CODR = OUTPUTS; //clear all outputs

    //int PIN_6 = (1 << 5); // take 00000001, shift it left 5 times -> 0100000

}

int getInputA() {
    *ADC_CR = 0x2;
    while (*ADC_SR & 0x10 == 0);
    return *ADC_CDR4;
}

int getInputB() {
    *ADC_CR = 0x2;
    while (*ADC_SR & 0x10 == 0);
    return *ADC_CDR5;
}

//RANGE IS 0-1023
void setY(int y) {
    *SPI_TDR = WRITE_A | (y << 2);
    waitForSPI();
}

//RANGE IS 0-1023
void setX(int x) {
    *SPI_TDR = WRITE_B | (x << 2);
    waitForSPI();
}

void delay(int count)
{
    register int i;
    for (i=count;i>0;i--)
        ;
}

void waitForSPI() {
    while (*SPI_SR & 2 != 2)
        continue;
}

