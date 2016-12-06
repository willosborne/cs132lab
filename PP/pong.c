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

float ballX = 512;
float ballY = 512;
float ballvX = 5;
float ballvY = 2;
int player1Input = 0;
int player2Input = 0;
int paddleSize = 200;

int player1PaddleSize;
int player2PaddleSize;
int player1BaseSize;
int player2BaseSize;

int player1Score = 0;
int player2Score = 0;
int seed = 3;

int fireworksFrame = 0;
int fireworkCounter = 0;
int fireworkFinished = 0;
int accumulator = 0;

int frameDelay = 0x130000;
int restartDelay = 0x300000;
int restartPaused = 1;

int state;

const int GAME = 0;
const int FIREWORKS = 1;

void drawChar (int, int, char);
void changeState (int);

int powerupX;
int powerupY;
int powerupSize = 100;

/*
 * 0 = does not exist
 * 1 = exists, but has not been activated
 * 2 = does not exist, but effect is active for player 1
 * 3 = does not exist, but effect is active for player 2
 */
int powerupState = 0;

/*
 * either: (depends on powerupState)
 * 0 = how long it's been since the start, or since the last one ended
 * 1 = how long it's been sitting there for
 * 2 or 3 = how long it's been active for
 */
int powerupTimer = 0;

// how long a powerup should last for
int powerupDuration = 0xE00000;

// how long to wait before spawning a powerup
int powerupBaseDelay = 0x1900000;

int powerupPaddleSize = 300;


void drawPowerup(void);
void spawnPowerup(void);
int intersectsPowerup(void);
void hitPowerup(int);
void expirePowerup(void);

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


    seed = getInputA() * getInputB();
    state = GAME;

    player1PaddleSize = paddleSize;
    player2PaddleSize = paddleSize;
    player1BaseSize = paddleSize;
    player2BaseSize = paddleSize;

    powerupState = 0; // no powerup
    powerupTimer = powerupBaseDelay;

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
            if (player1Input + player1PaddleSize > 950)
                player1Input = 950 - player1PaddleSize;
            if (player2Input + player2PaddleSize > 950)
                player2Input = 950 - player2PaddleSize;

            if (player1Input < 50)
                player1Input = 50;

            if (player2Input < 50)
                player2Input = 50;

            drawVLine(75, player1Input, player1Input + player1PaddleSize);
            drawVLine(925, player2Input, player2Input + player2PaddleSize);



            //draw paddles

            //setX(75);


            /* drawLine(100, 200, 700, 900); */
            //setY()



            // move ball by velocity
            if (!restartPaused) {
                ballX += ballvX;
                ballY += ballvY;

                if (powerupState != 1)
                    powerupTimer -= 0x8000;
                
                if (powerupTimer <= 0) {
                    // powerup timer has expired:
                    if (powerupState == 0) {
                        spawnPowerup();
                    }
                    else if (powerupState == 1) {
                        // do nothing ; we don't want our powerups to expire
                    }
                    else {
                        // make powerup effect wear off
                        expirePowerup();
                    }
                }
            }

            if (powerupState == 1) {
                drawPowerup();
                if (intersectsPowerup()) {
                    if (ballvX < 0) {
                        // player 2 hit the powerup
                        hitPowerup(2);
                    }
                    else if (ballvX > 0) {
                        // player 1 hit the powerup
                        hitPowerup(1);
                    }
                }
            }

            // ball hits player 1 paddle
            if (ballY - player1Input <= player1PaddleSize && ballY > player1Input) {
                float intersection = ballY - (player1Input + player1PaddleSize / 2);
                if (ballX < 75 && ballX > 50){
                    ballvX = -ballvX;
                    ballvX += 0.75f;
                    ballX = 75;

                    ballvY = (intersection / (player1PaddleSize/2)) * 7;
                }
            }

            // ball hits player 2 paddle
            if (ballY - player2Input <= player2PaddleSize && ballY > player2Input) {
                float intersection = ballY - (player1Input + player2PaddleSize / 2);

                if (ballX > 925 && ballX < 950){
                    ballvX = -ballvX;
                    ballvY -= 0.75f;
                    ballX = 925;

                    ballvY = (intersection / (player2PaddleSize/2)) * 7;
                }
            }
            if (ballvY > 7)
                ballvY = 7;
            if (ballvY < -7)
                ballvY = -7;

            if(ballY > 950 || ballY < 50)
                ballvY = -ballvY;


            if (player1Score > 9) 
                changeState(FIREWORKS);
            if (player2Score > 9)
                changeState(FIREWORKS);

            //y *= 2;
            drawChar(300, 700, '0'+player1Score);
            drawChar(600, 700, '0'+player2Score);

            setX((int)ballX);
            setY((int)ballY);

            if (restartPaused) {
                accumulator += 0x8000;
                powerupState = 0;
                powerupTimer = powerupBaseDelay;
                if (accumulator > restartDelay) {
                    accumulator = 0;
                    restartPaused = 0;
                }
            }
            if(ballX > 950) {
                player1Score++;
                player1BaseSize -= 10;
                resetBall(1);
                //previousPaddleSize = player1PaddleSize;
            }
            if(ballX < 50) {
                player2Score++;
                player2BaseSize -= 10;
                resetBall(-1);
                //previousPaddleSize = player2PaddleSize;
            }


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
                drawChar(180, 300, 'p');
                drawChar(290, 300, '2');
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
        accumulator = 0;
    }
    if (newState == GAME){
        resetBall(1);
        restartPaused = 1;
        accumulator = 0;
        player1Score = 0;
        player2Score = 0;
        player1PaddleSize = paddleSize;
        player2PaddleSize = paddleSize;
        player1BaseSize = paddleSize;
        player2BaseSize = paddleSize;
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
    restartPaused = 1;
    accumulator = 0;

    player1PaddleSize = player1BaseSize;
    player2PaddleSize = player2BaseSize;

    /* if (powerupState == 2) */
    /*     player1PaddleSize = previousPaddleSize; */
    /* else if (powerupState = 3) */
    /*     player2PaddleSize = previousPaddleSize; */

    powerupState = 0;
    powerupTimer = powerupBaseDelay;
    //delay(0x90000);
}

void drawPowerup() {
    drawVLine(powerupX, powerupY, powerupY + powerupSize);
    drawVLine(powerupX + powerupSize, powerupY, powerupY + powerupSize);

    drawHLine(powerupY, powerupX, powerupX + powerupSize);
    drawHLine(powerupY + powerupSize, powerupX, powerupX + powerupSize);
}

int intersectsPowerup() {
    if (ballX > powerupX && ballX < powerupX + powerupSize && ballY > powerupY && ballY < powerupY + powerupSize)
        return 1;
    else
        return 0;
}

void spawnPowerup() {
    powerupState = 1;
    powerupX = 500;
    powerupY = 500;

    powerupY += (rand() - 5) * 50;
}

void hitPowerup(int player) {
    // set powerup state based on who hit the powerup
    if (player == 1) {
        powerupState = 2;
        //previousPaddleSize = player1PaddleSize;
        player1PaddleSize = powerupPaddleSize;
    }
    else if (player == 2) {
        powerupState = 3;
        //previousPaddleSize = player2PaddleSize;
        player2PaddleSize = powerupPaddleSize;
    }
    powerupTimer = powerupDuration;
}

void expirePowerup() {
    //if (powerupState == 2){
        player1PaddleSize = player1BaseSize;
    //}
    //if (powerupSize == 3) {
        player2PaddleSize = player2BaseSize;
    //}
    powerupState = 0;
    powerupTimer = powerupBaseDelay;
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

