//#include <stdlib.h>
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

void drawFirework(int, int);

void delay(int);
int getInputA(void);
int getInputB(void);
int adjustInput(int);
void resetBall(int);
int rand(void);

void drawLine(int, int, int, int);
void drawVLine(int, int, int);
void drawHLine(int, int, int);

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
int player1Score = 0;
int player2Score = 0;
int seed = 3;

int fireworksFrame = 0;
int fireworkCounter = 0;
int fireworkFinished = 0;
int accumulator = 0;
int frameDelay = 0x130000;


int state;

const int GAME = 0;
const int FIREWORKS = 1;

void drawChar (int, int, char);
void changeState (int);

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

    seed = getInputA() * getInputB();
    state = GAME;

    while(1) {
        if (state == GAME) {
            //draw bounding box
            register int i = 0;

            /* setX(50); */
            /* for (i = 0; i < 1023; i++) { */
            /*     setY(i); */
            /* } */
            /* setX(950); */
            /* for (i = 0; i < 1023; i++) { */
            /*     setY(i); */
            /* } */
            /* setY(50); */
            /* for (i = 0; i < 1023; i++) { */
            /*     setX(i); */
            /* } */
            /* setY(950); */
            /* for (i = 0; i < 1023; i++) { */
            /*     setX(i); */
            /* } */

            drawVLine(50, 50, 950);
            drawVLine(950, 50, 950);

            drawHLine(50, 50, 950);
            drawHLine(950, 50, 950);
            //get input from players
            player1Input = getInputA();
            player1Input = adjustInput(player1Input);

            player1Input -= 190;

            player2Input = getInputB();
            player2Input = adjustInput(player2Input);

            player2Input -= 190;
            if (player1Input + paddleSize > 950)
                player1Input = 950 - paddleSize;
            if (player2Input + paddleSize > 950)
                player2Input = 950 - paddleSize;

            if (player1Input < 50)
                player1Input = 50;

            if (player2Input < 50)
                player2Input = 50;

            drawVLine(75, player1Input, player1Input + paddleSize);
            drawVLine(925, player2Input, player2Input + paddleSize);



            //draw paddles

            //setX(75);


            /* drawLine(100, 200, 700, 900); */
            //setY()



            // move ball by velocity
            ballX += ballvX;
            ballY += ballvY;

            if (ballY - player1Input <= paddleSize && ballY > player1Input) {
                if (ballX < 75 && ballX > 50){
                    ballvX = -ballvX;
                    ballX = 75;
                }
            }
            if (ballY - player2Input <= paddleSize && ballY > player2Input) {
                if (ballX > 925 && ballX < 950){
                    ballvX = -ballvX;
                    ballX = 925;
                }
            }

            if(ballY > 950 || ballY < 50)
                ballvY = -ballvY;
            if(ballX > 950) {
                resetBall(1);
                player1Score++;
            }
            if(ballX < 50) {
                resetBall(-1);
                player2Score++;
            }

            if (player1Score > 9) 
                changeState(FIREWORKS);
            if (player2Score > 9)
                changeState(FIREWORKS);

            //y *= 2;
            drawChar(300, 700, '0'+player1Score);
            drawChar(600, 700, '0'+player2Score);

            setX(ballX);
            setY(ballY);



            delay(0x8000);

            /* if ((*PIO_PDSR & RED_SWITCH) != RED_SWITCH) { */
            /*     continue; */
            /* } */    
        }

        else if (state == FIREWORKS) {
            
            if (player1Score > player2Score){
                drawChar(200, 300, 'p');
                drawChar(250, 300, '1');
                drawChar(450, 300, 'w');
                drawChar(530, 300, '1');
                drawChar(650, 300, 'n');
                drawChar(750, 300, '5');
            }
            if (player2Score > player1Score){
                drawChar(200, 300, 'p');
                drawChar(250, 300, '1');
                drawChar(450, 300, 'w');
                drawChar(530, 300, '1');
                drawChar(650, 300, 'n');
                drawChar(750, 300, '5');
            }

            if (fireworkCounter == 0)
                drawFirework(512, 512);
            else if (fireworkCounter == 1)
                drawFirework(200, 300);
            else if (fireworkCounter == 2)
                drawFirework(800, 600);
            else if (fireworkCounter == 3)
                drawFirework(300, 800);
            else if (fireworkCounter == 4)
                drawFirework(400, 600);
            else if (fireworkCounter == 5)
                drawFirework(700, 200);
            ///fireworkCounter++;
            if (fireworkCounter > 5)
                changeState(GAME);
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
}

void drawFirework(int x, int y){

    int width = 150;
    int diagonal = 110;

    int length = 100;

    if (fireworksFrame == 0) {
        width = 50;
        diagonal = 30;
        length = 50;
        drawLine(x + width, y, x + width + length, y);
        drawLine(x - width, y, x - width - length, y);

        drawLine(x, y + width, x, y + width + length);            
        drawLine(x, y - width, x, y - width - length);

        drawLine(x + diagonal, y + diagonal, x + diagonal + diagonal, y + diagonal +  diagonal);
        drawLine(x - diagonal, y - diagonal, x - diagonal - diagonal, y - diagonal -  diagonal);
        drawLine(x + diagonal, y - diagonal, x + diagonal + diagonal, y - diagonal -  diagonal);
        drawLine(x - diagonal, y + diagonal, x - diagonal - diagonal, y + diagonal +  diagonal);
        delay(0x8000);

    }
    else if (fireworksFrame == 1) {
        width = 70;
        diagonal = 50;
        length = 70;
        drawLine(x + width, y, x + width + length, y);
        drawLine(x - width, y, x - width - length, y);

        drawLine(x, y + width, x, y + width + length);            
        drawLine(x, y - width, x, y - width - length);

        drawLine(x + diagonal, y + diagonal, x + diagonal + diagonal, y + diagonal +  diagonal);
        drawLine(x - diagonal, y - diagonal, x - diagonal - diagonal, y - diagonal -  diagonal);
        drawLine(x + diagonal, y - diagonal, x + diagonal + diagonal, y - diagonal -  diagonal);
        drawLine(x - diagonal, y + diagonal, x - diagonal - diagonal, y + diagonal +  diagonal);
        delay(0x8000);
    }
    else if (fireworksFrame == 2) {
        width = 100;
        diagonal = 70;
        length = 100;
        int tweak = 0;
        drawLine(x + width, y, x + width + length, y);
        drawLine(x - width, y, x - width - length, y);

        drawLine(x, y + width, x, y + width + length);            
        drawLine(x, y - width, x, y - width - length);

        drawLine(x + diagonal, y + diagonal, x + diagonal + diagonal-tweak, y + diagonal +  diagonal-tweak);
        drawLine(x - diagonal, y - diagonal, x - diagonal - diagonal+tweak, y - diagonal -  diagonal+tweak);
        drawLine(x + diagonal, y - diagonal, x + diagonal + diagonal-tweak, y - diagonal -  diagonal+tweak);
        drawLine(x - diagonal, y + diagonal, x - diagonal - diagonal+tweak, y + diagonal +  diagonal-tweak);
        delay(0x8000);
    }
    accumulator += 0x8000;
    if (accumulator > frameDelay) {
        fireworksFrame++;
        accumulator = 0;
    }
    if (fireworksFrame > 2) {
        fireworksFrame = 0;

        fireworkCounter++;
    }

}

void changeState(int newState) {
    state = newState;
    if (newState == FIREWORKS) {
        fireworksFrame = 0;
        accumulator = 0;
        fireworkCounter = 0;
        fireworkFinished = 0;
    }
    if (newState == GAME){
        resetBall(1);
        player1Score = 0;
        player2Score = 0;
    }
}

int rand() {
    seed = (2 * seed + 3) % 10;
    return seed;
}

void resetBall(int player) {
    ballX = 512;
    ballY = 512;
    ballvX = 5 * player;
    ballvY = rand() - 5;
    while (ballvY == 0)
        ballvY = rand() - 5;
    //delay(0x90000);
}

/// {{{
void drawChar (int x, int y, char toDraw) {
    int height = 100;
    int width = 80;
    switch (toDraw) {
        case '0':
            //drawLine(x, y, x, y+height);
            //drawLine(x, y+height, x+width, y+height);
            //drawLine(x+width, y, x+width, y+height);
            //drawLine(x, y, x+width, y);
            //
            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            //drawHLine(y+height/2, x, x+width);
            break;
        case '1':
            //drawVLine(x, y, y+height/2);
            //drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            //drawHLine(y, x, x+width/2);
            //drawHLine(y, x+width/2, x+width);

            //drawHLine(y+height, x, x+width/2);
            //drawHLine(y+height, x+width/2, x+width);

            //drawHLine(y+height/2, x, x+width);
            break;
        case '2':
            drawVLine(x, y, y+height/2);
            //drawVLine(x, y+height/2, y+height);

            //drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);
            //drawHLine(y, x+width/2, x+width);

            drawHLine(y+height, x, x+width);
            //drawHLine(y+height, x+width/2, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '3':
            //drawVLine(x, y, y+height/2);
            //drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);
            //drawHLine(y, x+width/2, x+width);

            drawHLine(y+height, x, x+width);
            //drawHLine(y+height, x+width/2, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '4':
            //drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            //drawHLine(y, x, x+width/2);
            //drawHLine(y, x+width/2, x+width);

            //drawHLine(y+height, x, x+width);
            //drawHLine(y+height, x+width/2, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '5':
            //drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            //drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '6':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            //drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '7':

            //drawVLine(x, y, y+height/2);
            //drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            //drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            //drawHLine(y+height/2, x, x+width);
            break;
        case '8':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '9':

            //drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case 'p':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            //drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            //drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case 'w':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width*1.5, y, y+height/2);
            drawVLine(x+width*1.5, y+height/2, y+height);
            
            drawLine(x, y, x+width*0.75, y+height);

            drawLine(x+width*0.75, y+height, x+width*1.5, y);
            //drawHLine(y, x, x+width);

            //drawHLine(y+height, x, x+width);

            //drawHLine(y+height/2, x, x+width);
            break;
        case 'n':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);
            
            drawLine(x, y+height, x+width, y);
            //drawHLine(y, x, x+width);

            //drawHLine(y+height, x, x+width);

            //drawHLine(y+height/2, x, x+width);
            break;
    }
}
// }}}

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

int adjustInput(int input) {
    int val = input * 2;
    if (val > 1023)
        val = 1022;
    if (val < 0)
        val = 0;
    return val;
}

//RANGE IS 0-1023
void setY(int y) {
    *SPI_TDR = WRITE_A | (y << 2);
    waitForSPI();
    //delay(0x0001);
}

//RANGE IS 0-1023
void setX(int x) {
    *SPI_TDR = WRITE_B | (x << 2);
    waitForSPI();
    //delay(0x0001);
}

void drawVLine(int x, int startY, int endY) {
    register int i;
    setX(x);
    for (i = startY; i < endY; i++) {
        setY(i);
    }
}

void drawHLine(int y, int startX, int endX) {
    register int i;
    setY(y);
    for (i = startX; i < endX; i++) {
        setX(i);
    }
}

void drawLine(int startX, int startY, int endX, int endY) {
    float dX = endX - startX;
    float dY = endY - startY;

    int interval = 100;

    dX /= interval;
    dY /= interval;

    register int i = 0;
    for (i = 0; i <= interval; i++) {
        setX(startX + (int)(i * dX));
        setY(startY + (int)(i * dY));
    }
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

