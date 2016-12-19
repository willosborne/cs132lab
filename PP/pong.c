// save us writing volatile unsigned int* for every register
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

// pin data status register - we aren't using pins, but define it anyway just in case we need it
#define PIO_PDSR    (ioreg *) 0xfffff43c


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


// vim code folding marker
// {{{

// we're going to set PMC_PCER to this on init.
// set bits 2, 4 and 5; this means we want to use Parallel I/O controller A, the Analog-to-Digital converter and the Serial Peripheral Interface respectively.
#define PIO_IDENTIFIER 0x34 // 0b110100

// These two constants set bits 12-15 of the message we send to the chip
#define WRITE_B  0x4000 //0b0100000000000000; - R1 0, SPD 1, PWR 0, R0 0 (i.e. write to port B + buffer)
#define WRITE_A  0xc000 //0b1100000000000000; - R1 1, SPD 1, PWR 0, R0 0 (i.e. write to port A + update B with buffer)

/*
 * FUNCTION SIGNATURES: see definitions for details on each function
 */

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

void drawChar (int, int, char);
void changeState (int);

void drawPowerup(void);
void spawnPowerup(void);
int intersectsPowerup(void);
void hitPowerup(int);
void expirePowerup(void);

// code folding, ignore
// }}}

/*
 * GLOBAL VARIABLES
 */

// information about ball position and velocity. Stored as floats to make life easier, then casted to ints for rendering.
float ballX = 512;
float ballY = 512;
float ballvX = 5;
float ballvY = 2;

// hold the values from the two knobs
int player1Input = 0;
int player2Input = 0;

// the default paddle size
int paddleSize = 200;

// keep track of how big each player's paddle should be, and how big it is at the moment (for use with the powerup)
int player1PaddleSize;
int player2PaddleSize;
int player1BaseSize;
int player2BaseSize;

// scores
int player1Score = 0;
int player2Score = 0;

// for the "random number generator"
int seed = 3;

// animation information for the fireworks
int fireworksFrame = 0;
int fireworkCounter = 0;
int fireworkFinished = 0;

// holds the time value - when this goes over a certain value it'll advance the animation by 1 frame
int accumulator = 0;
int frameDelay = 0x130000;

// how long to wait for after every pause
int restartDelay = 0x300000;
int restartPaused = 1;

// what state the game's in
int state;
const int GAME = 0;
const int FIREWORKS = 1;

// x of powerup on screen
int powerupX;
// y of powerup on screen
int powerupY;
// side length of powerup
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
// how large to make the paddles when under the effects of the powerup
int powerupPaddleSize = 300;


// entry point
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
     * SETUP FUNCTIONS
     */

    LowLevelInit(); //setup, we don't need to know about this
    *PMC_PCER = PIO_IDENTIFIER; // set up what mode the chip should be in - so we want to be using PIO controller A and the Analog-Digital converter
    *PIO_PDR = 0x7800; // disable bits 11-14
    *PIO_ASR = 0x7800;  // peripheral A is using 11-14
    *SPI_CR = 0x80; // reset serial interface to ensure no errors occur
    *SPI_CR = 0x1; // enable it

    /* 
     * CONFIGURE SPI
     */
    *SPI_MR = 0x11; // spi to master mode, turn off fault detection, need this for it to work
    *SPI_CSR0 = 0x183; //clock polarity = 1, clock phase = 1, bits/transfer 16, baud raite = 1
    *SPI_TDR = 0xd002; // reference voltage to 2
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

    // wait until SPI is ready
    waitForSPI();

    //INPUT SETUP

    *ADC_CR = 0x1; //reset ADC
    *ADC_CHER = 0x30; // enable channels 4 and 5

    *ADC_MR = 0x030b0400; // sample+holdtime = 3, startup = b, prescale = 4


    //set seed for RNG
    seed = getInputA() * getInputB();

    // game setup
    state = GAME;

    // set everything to default values
    player1PaddleSize = paddleSize;
    player2PaddleSize = paddleSize;
    player1BaseSize = paddleSize;
    player2BaseSize = paddleSize;

    powerupState = 0; // no powerup at the moment
    powerupTimer = powerupBaseDelay;

    // main loop
    while(1) {
        // FSM - while it's in game state
        if (state == GAME) {
            //draw bounding box
            drawVLine(50, 50, 950);
            drawVLine(950, 50, 950);

            drawHLine(50, 50, 950);
            drawHLine(950, 50, 950);


            //get input from players and adjust it - we take 190 away to get the offsets working properly (otherwise it renders the line in the wrong place)
            player1Input = getInputA();
            player1Input = adjustInput(player1Input);
            player1Input -= 190;

            player2Input = getInputB();
            player2Input = adjustInput(player2Input);
            player2Input -= 190;

            // make sure player paddles are on the screen 
            if (player1Input + player1PaddleSize > 950)
                player1Input = 950 - player1PaddleSize;
            if (player2Input + player2PaddleSize > 950)
                player2Input = 950 - player2PaddleSize;
            if (player1Input < 50)
                player1Input = 50;
            if (player2Input < 50)
                player2Input = 50;

            //draw the paddles
            drawVLine(75, player1Input, player1Input + player1PaddleSize);
            drawVLine(925, player2Input, player2Input + player2PaddleSize);

            // if the game is paused between points, stop the game logic but keep the rendering going
            if (!restartPaused) {

                //update ball position
                ballX += ballvX;
                ballY += ballvY;

                // if the powerup timer needs to be counting, make it count down
                if (powerupState != 1)
                    powerupTimer -= 0x8000;

                // when it hits zero, trigger an effect based on its state
                if (powerupTimer <= 0) {

                    // no powerup at the moment, spawn one
                    if (powerupState == 0) {
                        spawnPowerup();
                    }
                    // powerup currently spawned, do nothing
                    else if (powerupState == 1) {
                        // do nothing ; we don't want our powerups to expire
                    }
                    // one of the players has the powerup, make it wear off
                    else {
                        // make powerup effect wear off
                        expirePowerup();
                    }
                }
            }

            // if the powerup is on screen, draw it and check if someone has hit it
            if (powerupState == 1) {
                drawPowerup();

                // if someone has hit it, check based on which way the ball is moving who it was
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
                // calculate the difference between the centre of the paddle and the ball, then use this to set the new velocity for the ball - allows for player control over the ball
                // this is based on the original Pong game (uses slightly different numbers, though)
                float intersection = ballY - (player1Input + player1PaddleSize / 2);
                if (ballX < 75 && ballX > 50){
                    // flip ball X velocity and increase it slightly
                    ballvX = -ballvX;
                    ballvX += 0.75f;

                    // clamp X position to stop it going off screen when it's really fast
                    ballX = 75;

                    ballvY = (intersection / (player1PaddleSize/2)) * 7;
                }
            }

            // ball hits player 2 paddle - this is exactly the same as for player 1 but with tweaked values
            if (ballY - player2Input <= player2PaddleSize && ballY > player2Input) {
                float intersection = ballY - (player1Input + player2PaddleSize / 2);

                if (ballX > 925 && ballX < 950){
                    ballvX = -ballvX;
                    ballvY -= 0.75f;
                    ballX = 925;

                    ballvY = (intersection / (player2PaddleSize/2)) * 7;
                }
            }

            // clamp ball vertical speed
            if (ballvY > 7)
                ballvY = 7;
            if (ballvY < -7)
                ballvY = -7;

            // bounce on top+bottom of screen
            if(ballY > 950 || ballY < 50)
                ballvY = -ballvY;

            // detect game over
            if (player1Score > 9) 
                changeState(FIREWORKS);
            if (player2Score > 9)
                changeState(FIREWORKS);

            // draw scores - add the score on to the char '0' to give us a number (this means we can't have a score greater than 9)
            drawChar(300, 700, '0'+player1Score);
            drawChar(600, 700, '0'+player2Score);

            // draw the ball LAST to ensure it's brightest, and clamp the position to integers. This might cause issues, but they're pretty minor considering the range is 0-1023
            setX((int)ballX);
            setY((int)ballY);

            // if we are paused, update all the timers to make sure we unpause at the right time
            if (restartPaused) {
                accumulator += 0x8000;
                // also reset the powerup
                powerupState = 0;
                powerupTimer = powerupBaseDelay;

                // start game when time is up
                if (accumulator > restartDelay) {
                    accumulator = 0;
                    restartPaused = 0;
                }
            }

            // player 1 has scored
            if(ballX > 950) {
                player1Score++;
                player1BaseSize -= 10;
                // resetBall takes one argument, 1 or -1, which tells it which way to make the ball move when it restarts
                resetBall(1);
            }

            // player 2 has scored
            if(ballX < 50) {
                player2Score++;
                player2BaseSize -= 10;
                // resetBall takes one argument, 1 or -1, which tells it which way to make the ball move when it restarts
                resetBall(-1);
            }

            // delay and end loop
            delay(0x8000);

            /* if ((*PIO_PDSR & RED_SWITCH) != RED_SWITCH) { */
            /*     continue; */
            /* } */    
        }

        // if we're in firework mode
        else if (state == FIREWORKS) {
            // draw the win messages for player 1 or two based on the score
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

            // draw fireworks one after another
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
            // when we're done, go back to the game
            if (fireworkCounter > 5)
                changeState(GAME);
        }
    }
}

// draw a firework animation at the X and Y co-ordinates given. 
void drawFirework(int x, int y){
    // this basically works by drawing lots and lots of lines in the shape of a basic firework.
    // we tweak the values for each frame to make it grow
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

    // this stuff handles the animation
    // add the time we've delayed for to the accumulator every frame, and when it reaches the delay threshold, we advance the frame and reset the accumulator.
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

// switch state
void changeState(int newState) {
    state = newState;
    // basically, we reset everything whenever we enter a new state.
    if (newState == FIREWORKS) {
        fireworksFrame = 0;
        accumulator = 0;
        fireworkCounter = 0;
        fireworkFinished = 0;
        accumulator = 0;
    }
    if (newState == GAME){
        // ball always goes the same way on a new game for simplicity
        resetBall(1);
        // we pause at the start of a new game
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

// random number generator - uses a Linear Congruential Generator to generate very simple random numbers from 0 to 10.
int rand() {
    seed = (2 * seed + 3) % 10;
    return seed;
}

// reset the ball - the player argument is either 1 or -1 and decides which X direction the ball will move in.
void resetBall(int player) {
    // reset position
    ballX = 512;
    ballY = 512;
    // reset velocity on x axis to a constant * the player factor
    ballvX = 5 * player;
    // use our RNG to get the Y velocity, but subtract 5 to make it from -5 to 5
    ballvY = rand() - 5;
    while (ballvY == 0)
        ballvY = rand() - 5;
    // pause afterwards and reset accumulator
    restartPaused = 1;
    accumulator = 0;

    // reset paddle sizes
    player1PaddleSize = player1BaseSize;
    player2PaddleSize = player2BaseSize;

    // reset powerup states
    powerupState = 0;
    powerupTimer = powerupBaseDelay;
    //delay(0x90000);
}

// draws a box at the powerup location
void drawPowerup() {
    drawVLine(powerupX, powerupY, powerupY + powerupSize);
    drawVLine(powerupX + powerupSize, powerupY, powerupY + powerupSize);

    drawHLine(powerupY, powerupX, powerupX + powerupSize);
    drawHLine(powerupY + powerupSize, powerupX, powerupX + powerupSize);
}

// returns 1 if the ball intersects the powerup. Could be shorter but wrote it like this for simplicity
int intersectsPowerup() {
    if (ballX > powerupX && ballX < powerupX + powerupSize && ballY > powerupY && ballY < powerupY + powerupSize)
        return 1;
    else
        return 0;
}

// spawn the powerup
void spawnPowerup() {
    // change the state
    powerupState = 1;
    // set an initial position of the middle of the screen
    powerupX = 500;
    powerupY = 500;

    // adjust the Y position by a random value from -250 to 250
    powerupY += (rand() - 5) * 50;
}

// called when the ball hits the
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
    // set the timer to duration (as state 2 or 3 means a player has an active powerup)
    powerupTimer = powerupDuration;
}

// called when the powerup runs out
void expirePowerup() {
    // reset the paddles to their base sizes
    player1PaddleSize = player1BaseSize;
    player2PaddleSize = player2BaseSize;
    // change state and set timer accordingly
    powerupState = 0;
    powerupTimer = powerupBaseDelay;
}

// vim comment folding marker, use :set foldmethod=marker to hide this extremely large function
// {{{
// draws a single character at the x and y co-ords given.
// supported chars: '0' to '9', 'p', 'w' and 'n'
void drawChar (int x, int y, char toDraw) {
    // height and width of a single segment of the character.
    int height = 100;
    int width = 80;

    // works by drawing a large number of vertical or horizontal lines, and occasionally a diagonal for the letters.
    switch (toDraw) {
        case '0':
            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            break;
        case '1':

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            break;
        case '2':
            drawVLine(x, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '3':

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '4':
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);


            drawHLine(y+height/2, x, x+width);
            break;
        case '5':
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '6':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);

            drawHLine(y, x, x+width);

            drawHLine(y+height, x, x+width);

            drawHLine(y+height/2, x, x+width);
            break;
        case '7':


            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);


            drawHLine(y+height, x, x+width);

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

            drawVLine(x+width, y+height/2, y+height);


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
            break;
        case 'n':

            drawVLine(x, y, y+height/2);
            drawVLine(x, y+height/2, y+height);

            drawVLine(x+width, y, y+height/2);
            drawVLine(x+width, y+height/2, y+height);

            drawLine(x, y+height, x+width, y);
            break;
    }
}
// }}}

// get input from the first player
int getInputA() {
    *ADC_CR = 0x2;
    while (*ADC_SR & 0x10 == 0);
    return *ADC_CDR4;
}

// get input from the second player
int getInputB() {
    *ADC_CR = 0x2;
    while (*ADC_SR & 0x10 == 0);
    return *ADC_CDR5;
}

// tweak an input to be in the range 0-1023
int adjustInput(int input) {
    int val = input * 2;
    if (val > 1023)
        val = 1022;
    if (val < 0)
        val = 0;
    return val;
}

//set the Y of the Oscilloscope
void setY(int y) {
    // first two bits of the data payload are ignored, so shift our value left by two places (y << 2)
    // append our data to WRITE_A, defined at the top - this basically tells the processor to write to output A in fast mode without powering down.
    *SPI_TDR = WRITE_A | (y << 2);
    // wait for the SPI to be ready for another input.
    waitForSPI();
}

// set the x of the oscilloscope
void setX(int x) {
    // first two bits of the data payload are ignored, so shift our value left by two places (y << 2)
    // append our data to WRITE_A, defined at the top - this basically tells the processor to write to output A in fast mode without powering down.
    *SPI_TDR = WRITE_B | (x << 2);
    // wait for the SPI to be ready for another input.
    waitForSPI();
}

// draw a vertical line at x, from starty to endy
void drawVLine(int x, int startY, int endY) {
    register int i;
    setX(x);
    for (i = startY; i < endY; i++) {
        setY(i);
    }
}

// draw a horizontal line at y, from startX to endX
void drawHLine(int y, int startX, int endX) {
    register int i;
    setY(y);
    for (i = startX; i < endX; i++) {
        setX(i);
    }
}

// draw a line between the two points - can be diagonal
void drawLine(int startX, int startY, int endX, int endY) {
    float dX = endX - startX;
    float dY = endY - startY;

    // might be overkill, but we had enough time and didn't want it looking dotty
    int interval = 100;

    dX /= interval;
    dY /= interval;

    // divide the difference between the two points into _interval_ pieces and move the dot to each location. 

    register int i = 0;
    for (i = 0; i <= interval; i++) {
        setX(startX + (int)(i * dX));
        setY(startY + (int)(i * dY));
    }
}

// simple delay function. do nothing for _count_ frames.
void delay(int count)
{
    register int i;
    for (i=count;i>0;i--)
        ;
}

// wait until the serial interface is ready to get more data.
void waitForSPI() {
    // specifically, wait until bit 1 of the SPI status register is 1 - this indicates that the conversion has finished and SPI_TDR is ready to receive more data.
    while (*SPI_SR & 2 != 2)
        continue;
}

