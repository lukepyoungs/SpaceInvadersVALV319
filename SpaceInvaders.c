// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EdX Lab 15
// Cole Lutz and Luke Youngs
// Last Modified: 8/11/2020 
// for images and sounds, see http://users.ece.utexas.edu/~valvano/Volume1/Lab10Files
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2020

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2020

 Copyright 2020 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

////////////////////////////////////////////////////////////////////////////////
//  Lines   |                  Reference Index                                //
//  76-86    - Includes and Defines                                           //
//  91-98    - Psuedo Ops                                                     //
//  99-101   - Declaring ADC                                                  //
//  103-113  - Ascii Conversion and Print                                     //
//  114-135  - Port Initialization                                            //
//  136-157  - Systick                                                        //
//  158-160  - Random Number                                                  //
//  162-177  - Button Checks                                                  //
//  179-196  - Create Start Screen                                            //
//  197-205  - Create Language Screen                                         //
//  206-250  - Initial Screen Logic                                           //
//  251-258  - Player Ship Data Type Declaration                              //
//  261-270  - Enemy Ship Data Type Declaration                               //
//  272-280  - Laser Sprite                                                   //
//  282-285  -                                                                      //
//                                                                       //
//                                                                       //
//                                                                       //
////////////////////////////////////////////////////////////////////////////////


#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Sound.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "DAC.h"
#include "Images.h"
#include "Timer0.h"
#include "Timer1.h"
#define PF1       (*((volatile uint32_t *)0x40025008))	
#define PF2       (*((volatile uint32_t *)0x40025010))	
#define PF3       (*((volatile uint32_t *)0x40025020))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void GameOver(void);	// function prototype for GameOver
void Victory(void);
void Create(void);
void LaserLogicP(void);
void LaserLogicE(void);
void Loss(void);
void DAC_Init(void);
void UpdateScore(int currentScore);
void HardInit(void);
void Timer2_Init(void(*task)(void), unsigned long period);
unsigned long ADCMail;
unsigned long ADCStatus;
signed long ADCData, Pos, Buf;

extern unsigned char String[12];
void ConvertUDec(unsigned long n);
void ConvertDistance(unsigned long n);
void LCD_OutUDec(unsigned long n){
  ConvertUDec(n);     // convert using your function
  ST7735_OutString((char *)String);  // output using your function
}
void LCD_OutDistance(unsigned long n){
  ConvertDistance(n);      // convert using your function
  ST7735_OutString((char *)String);  // output using your function
}
void PortF_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x20;
	delay = SYSCTL_RCGC2_R;         //    allow time for clock to stabilize
	delay = SYSCTL_RCGC2_R;
	delay = SYSCTL_RCGC2_R;
	delay = SYSCTL_RCGC2_R;
	SYSCTL_RCGCGPIO_R |= 0x20;
	while((SYSCTL_PRGPIO_R&0x20) == 0){};
	GPIO_PORTF_DIR_R &= ~0x07; // Pf 123 input
	GPIO_PORTF_AFSEL_R &= ~0x07;
	GPIO_PORTF_AMSEL_R &= ~0x07;
	GPIO_PORTF_DEN_R |= 0x07;
}
void PortE_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x10;
	delay = SYSCTL_RCGC2_R;         //    allow time for clock to stabilize
	delay = SYSCTL_RCGC2_R;
	delay = SYSCTL_RCGC2_R;
	delay = SYSCTL_RCGC2_R;
	SYSCTL_RCGCGPIO_R |= 0x10;
	while((SYSCTL_PRGPIO_R&0x10) == 0){};
	GPIO_PORTE_DIR_R &= ~0x03; // PE0 PE1 input
	GPIO_PORTE_AFSEL_R &= ~0x03;
	GPIO_PORTE_AMSEL_R &= ~0x03;
	GPIO_PORTE_DEN_R |= 0x03;
}
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;    					// disable SysTick during setup
  NVIC_ST_CTRL_R = 0x07;
  NVIC_ST_RELOAD_R = 2000000;			// reload value --> 40 Hz
  NVIC_ST_CURRENT_R = 0;      		// any write to current clears it
}
int IntCounter;

void SysTick_Handler(void){
if(Button1Press()){
	PlayerFire();
}
}
uint32_t RandomNumber(void){
 return ((Random32()>>24)%5)+1;  // returns 1, 2, 3, 4, or 5
}

int OnePressed=0;
int Button1Press (void){
	OnePressed=(GPIO_PORTE_DATA_R&0x01);
	return OnePressed;
}
int TwoPressed=0;
int Button2Press (void){
	TwoPressed=(GPIO_PORTE_DATA_R&0x02);
	return TwoPressed;
}
void SetButton1 (unsigned long state){
	OnePressed=state;
}
void SetButton2 (unsigned long state){
	TwoPressed=state;
}

void CreateStartScn(void){
	ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Welcome to Space Invaders");
  ST7735_SetCursor(1, 4);
  ST7735_OutString("Your help is needed to");
  ST7735_SetCursor(1, 6);
  ST7735_OutString("destroy the dreaded alien");
	ST7735_SetCursor(1, 8);
	ST7735_OutString("fleet. Fail and humanity");
	ST7735_SetCursor(1, 10);
	ST7735_OutString("will face dire consequences!");
	ST7735_SetCursor(1, 12);
	ST7735_OutString("Press B1 to continue");
	ST7735_SetCursor(1, 14);
	ST7735_OutString("Press B2 for Language");



// we will still need to change the spacing

  //void CreateStartScnSpan(void){
  	// ST7735_FillScreen(0x0000);            // set screen to black
    // ST7735_SetCursor(1, 2);
    // ST7735_OutString("Bienvenido a Space Invaders");
    // ST7735_SetCursor(1, 4);
    // ST7735_OutString("Your help is needed to");
    // ST7735_SetCursor(1, 6);
    // ST7735_OutString("destroy the dreaded alien");
  	// ST7735_SetCursor(1, 8);
  	// ST7735_OutString("fleet. Fail and humanity");
  	// ST7735_SetCursor(1, 10);
  	// ST7735_OutString("will face dire consequences!");
  	// ST7735_SetCursor(1, 12);
  	// ST7735_OutString("Press B1 to continue");
  	// ST7735_SetCursor(1, 14);
  	// ST7735_OutString("Press B2 for Language");
  
}
void CreateLangScn(void){
	ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Choose your language:");
  ST7735_SetCursor(1, 4);
  ST7735_OutString("Press B1 for English");
  ST7735_SetCursor(1, 6);
  ST7735_OutString("Press B2 for Espa\xA4ol");
}
int GoFlag=0;
int LangHold;
int LFlag; //loss flag
uint32_t score = 0;					  // player's score
uint32_t LaserNumP = 0;		    // number of player lasers on screen
uint32_t LaserNumE = 0;			  // number of enemy lasers on screen
uint32_t LivingE = 0;				  // number of alive enemies
uint32_t DeadE = 0;				    // number of dead enemies
uint32_t EDelay = 0;					// enemy fire delay
uint32_t FrameCount = 0;			// current frame
uint32_t FireDelay = 0;			  // delay that the enemy fires at
uint32_t CounterX = 0;		    // enemy horizontal movement counter
uint32_t CounterY = 0;			  // enemy vertical movement counter
uint32_t GameStat = 0;			  // check if game is running
uint32_t stage = 0;					  // current stage of game

int StartCheck(void){
	return GoFlag;
}
void StartScn(void){
	int LangCheck;									// lang falg
	LangCheck = Button2Press();		// check  lang butt
	if(LangCheck){
		CreateLangScn();
		SetButton2(0);
	}

	while(LangCheck){
		//	if(Button1Press()){
	//	LangHold=1;
//}
	//if(Button2Press()){
	//	LangHold=2;
	//}
		LangCheck = Button2Press(); 	// polls controls button state while in controls
		SetButton1(0);
		if(!LangCheck){
			CreateStartScn();
			SetButton2(0);
		}
	}

	if(Button1Press()){
		GoFlag = 1;
	}
}
struct PlayerShip{
	uint32_t xP;									// x-axis position
	uint32_t yP;									// y-axis position
	uint32_t HP;										// health of player
	int deathstate;												// status of recent death
	const unsigned short *image; 		// pointer to bitmaps
};
typedef struct PlayerShip PlayerShip_t;	// player sprite datatype

// Sprites used for enemies
struct EnemyShip{
	uint32_t xP;									// x-axis position
	uint32_t yP;									// y-axis position
	uint32_t HP;										// health of enemy
	uint32_t pointval;								// point value of enemy
	int deathstate;												// status of recent death
	const unsigned short *image[2]; // pointer to bitmaps
	/* 2 images emulate animation */
};
typedef struct EnemyShip EnemyShip_t;		// enemy sprite datatype

struct LaserSprite{
	int xP;											// x-axis position
	int yP;											// y-axis position
	int deathstate;											// status of laser (alive/dead)
	const unsigned short *image; 	// pointer to bitmaps
	unsigned short velocity;								// vertical speed of laser
};
typedef struct LaserSprite PlayerLaser_t;		// player laser sprite datatype
typedef struct LaserSprite EnemyLaser_t;		// enemy laser sprite datatype

PlayerShip_t Player;					
EnemyShip_t Enemies[18];			
PlayerLaser_t PlayerLasers[10];	// maximum # of player lasers on screen is 4
EnemyLaser_t EnemyLasers[10];		// maximum # of enemy lasers on screen is 4

void GameInit(void){
  unsigned short ind = 0;															// index
	 
	GameStat = 1;														// game is running
	score = 0;																	// score reset
	FrameCount = 0;																	// current frame state
	LaserNumP = 0;												// player lasers reset
	LaserNumE = 0;												// enemy lasers reset

	LivingE = 18;													// enemies are all alive
	DeadE = 0;													// no enemies are dead
	FireDelay = RandomNumber();					// random rate of fire for enemy 
  Player.xP = 55;														// player ship starts at middle bottom
	Player.yP = 158;
	Player.HP = 1;															// player starts with 1 health
	Player.deathstate = 0;														// player hasn't recently died
	Player.image = PlayerShip0;			// player starts with default ship
	PlayerLasers[0].image = PlayerLaser1;
	PlayerLasers[1].image = PlayerLaser1;
	PlayerLasers[2].image = PlayerLaser1;
	PlayerLasers[3].image = PlayerLaser1;
	EnemyLasers[0].image = EnemyLaser1;
	EnemyLasers[1].image = EnemyLaser1;
	EnemyLasers[2].image = EnemyLaser1;
	EnemyLasers[3].image = EnemyLaser1;
	for(; ind < 18; ind++){													// initializes aliens
		Enemies[ind].HP = 1;													// enemy starting health
		Enemies[ind].deathstate = 0;												// enemies hasn't recently died
		if(ind < 6){																	// first row of 6 enemies in array are 10 pt.
			Enemies[ind].xP = (16 * ind) + 4;
			Enemies[ind].yP = 10 + 9 + 10; 								// 19
			Enemies[ind].image[0] = SmallEnemy30pointA;	// arms down
			Enemies[ind].image[1] = SmallEnemy30pointB;	// arms up
			Enemies[ind].pointval = 30;										// enemy is worth 30 points
		}
		else if(ind < 12){														// second row of 6 enemies in array are 20 pts.
			Enemies[ind].xP = (16 * (ind - 6)) + 4;
			Enemies[ind].yP = 20 + 9 + 10; 								// 29
			Enemies[ind].image[0] = SmallEnemy20pointA;	// arms down
			Enemies[ind].image[1] = SmallEnemy20pointB;	// arms up
			Enemies[ind].pointval = 20;										// enemy is worth 20 points
		}
		else{																				// third row of 6 enemies in array are 30 pts.
			Enemies[ind].xP = (16 * (ind - 12) + 4);
			Enemies[ind].yP = 30 + 9 + 10; 								// 39
			Enemies[ind].image[0] = SmallEnemy10pointA;	// arms down
			Enemies[ind].image[1] = SmallEnemy10pointB;	// arms up
			Enemies[ind].pointval = 10;										// enemy is worth 10 points
		}
	}
  ind=0;
  for(; ind < 1; ind++){													// initializes player lasers
		PlayerLasers[ind].deathstate = 1;				// no player lasers at beginning
	}
	
	ind = 0;
	for(; ind < 4; ind++){													// initializes enemy lasers
		EnemyLasers[ind].deathstate = 1;									// no enemy lasers at beginning
	}
	UpdateScore(score);															// update socre
	Create();																			// draw initial frame
}
int	ind = 0;
void UpdateScore(int currentScore){
	ST7735_SetCursor(7, 1);
	ST7735_OutString("Score:");
	LCD_OutUDec(currentScore);
}
void CreateP(void){
  if(Player.HP > 0)
		ST7735_DrawBitmap(Player.xP, Player.yP, Player.image, 12, 8); 	// draw player spaceship
	else{
		if(Player.deathstate == 1){
			ST7735_DrawBitmap(Player.xP, Player.yP, Explosion, 16, 10);		// draw explosion upon death
			Player.deathstate = 0;																									// reset recent death flag
		}
		else
			ST7735_DrawBitmap(Player.xP, Player.yP, BlackEnemy, 16, 10);	// draw black space after explosion
	}
}
void CreateE(void){
  unsigned short L = 0;
	for(; L < 18; L++){
		if(Enemies[L].HP > 0)																																		// check if enemy is alive
			ST7735_DrawBitmap(Enemies[L].xP, Enemies[L].yP, Enemies[L].image[FrameCount], 16, 10);	// draw enemy bitmap depending on frame state
		else{
			if(Enemies[L].deathstate == 1){																															// check recent death flag
				ST7735_DrawBitmap(Enemies[L].xP, Enemies[L].yP, Explosion, 16, 10);							// draw explosion
				Enemies[L].deathstate = 0;																																// reset recent death flag
			}
			else
				ST7735_DrawBitmap(Enemies[L].xP, Enemies[L].yP, BlackEnemy, 16, 10);						// draw black space over enemy after death
		}
	}
}
void ELasers(void){
  unsigned short in=0;
  for(; in < 4; in++){
		if(EnemyLasers[in].deathstate == 0){																																// check if enemy lasers are alive
			ST7735_DrawBitmap(EnemyLasers[in].xP, EnemyLasers[in].yP, EnemyLasers[in].image, 2, 10);	// if enemy laser is alive, draw it
			Sound_Shoot();	// play shoot sound
		}
	}
}
void PLasers(void){
  unsigned short in=0;
  for(;in<4;in++){
    if(PlayerLasers[in].deathstate == 0){
      ST7735_DrawBitmap(PlayerLasers[in].xP, PlayerLasers[in].yP, PlayerLasers[in].image, 2, 10);
			Sound_Shoot();
    }
  }
}

void Create(void){
  CreateP();
  CreateE();
  ELasers();
  PLasers();
}
void MoveHor(void){
  ST7735_DrawBitmap(Player.xP, Player.yP, BlackPlayer, 12, 8); // draw black placeholder in place of ship to avoid double images
	uint32_t Aninput;																	// holds ADC data
	
	Aninput = ADC0_In();																// reads in ADC data
	if((Aninput < 1100) && (Player.xP > 5)){					
		Player.xP -= 1;																	// move left
	}
	else if((Aninput < 1100) && (Player.xP <= 5)){		
		Player.xP = 0;																	// minimum left position
	}
	else if((Aninput > 2800) && (Player.xP< 115)){	
		Player.xP += 1;																	// move right
	}
	else if((Aninput > 2800) && (Player.xP >= 115)){	
		Player.xP = 115;																// maximum right position
	}
}
void MoveEnemy(void){
  if(CounterX<2){
    int8_t j = 0;
	for(; j < 18; j++){
		Enemies[j].xP -= 1;				// move every enemy right 1 pixel
	 }
  CounterX++;
  FrameCount ^=1;
  }
  if(CounterX<2){
    int8_t j = 0;
  for(; j < 18; j++){
    Enemies[j].xP += 1;				// move every enemy lrft 1 pixel
    }
  CounterX++;
  FrameCount ^=1;
  }
  if(CounterX==2){
    int8_t j = 0;
  for(; j < 18; j++){
    Enemies[j].xP += 1;				// move every enemy left 1 pixel
    }
  CounterX=0;
  CounterY++;
  FrameCount ^=1;
  }
  if(CounterY==1){
    int8_t j = 0;
    CounterY=0;
  for(; j < 18; j++){
    Enemies[j].yP += 1;				// move every enemy up 1 pixel
    }
  }
	
 char  j=0;
  for(; j < 18; j++){
  if(Enemies[j].yP == Player.yP){
    Loss();
  }
}
	}
void LaserLogicP(void)
	{
  short z = 0;
	for(;z < 1; z++){		
		if(PlayerLasers[z].deathstate == 0){											// checks if each player laser is alive
			if(PlayerLasers[z].yP > 30){											// if laser has not reached top of screen
				PlayerLasers[z].yP -= PlayerLasers[z].velocity;	// move player laser up screen by its speed
			}
			else{
				PlayerLasers[z].deathstate = 1;												// laser is removed
				ST7735_DrawBitmap(PlayerLasers[z].xP, PlayerLasers[z].yP, BlackLaser, 2, 10);	
				LaserNumP--;															// decrease player laser count
			}
		}
	}
}
void LaserLogicE(void){
  char z = 0;
	for(;z < 4; z++){																		// checks state of every enemy laser
		if(EnemyLasers[z].deathstate == 0){											// if enemy laser is alive
			if((EnemyLasers[z].yP < 178) && (EnemyLasers[z].yP > 30)){									// checks if enemy laser has reached bottom of screen
				//ST7735_DrawBitmap(EnemyLasers[k].xPos, EnemyLasers[k].yPos, BlackLaser, 2, 8); // draw black space at original position of laser
				EnemyLasers[z].yP += EnemyLasers[z].velocity;	// moves enemy laser down screen
			}
			else{
				EnemyLasers[z].deathstate = 1;											// removes enemy laser
				ST7735_DrawBitmap(EnemyLasers[z].xP, EnemyLasers[z].yP, BlackLaser, 2, 10);
				LaserNumE--;														// decrease enemy laser count
			}
		}
	}
}

void MoveAll(void){
  MoveHor();
  LaserLogicP();
  LaserLogicE();
  //ISR FOR ENEMY MOVEMENT
}
void PausedScn(void){
  ST7735_FillScreen(0x0000);            
  ST7735_SetCursor(1, 2);
  ST7735_OutString("GAME PAUSED");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("B2 TO RESUME");
}
void LossScn(void){
  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(6, 1);
	ST7735_OutString("SCORE:");
	LCD_OutUDec(score);
	ST7735_SetCursor(5, 5);
  ST7735_OutString("GAME OVER");
}
void WinScn(void){
  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(6, 1);
	ST7735_OutString("SCORE:");
	LCD_OutUDec(score);
	ST7735_SetCursor(7, 5);
  ST7735_OutString("VICTORY");
}
void Loss(void){
  LossScn();
  GameStat=0;
  while(1)
  {
    
  }
}
void Win(void){
  if(LivingE==0){
    GameStat=0;
  }
  
}
void Pause(void){
	int SemaPause;								// flag for game pause

	SemaPause = Button2Press();		// checks if pause button was pressed
	if(SemaPause){
		if(GameStat){
			PausedScn();
		}
	}
	while(SemaPause){
		SemaPause = Button2Press(); // polls pause button state while paused
		if(GameStat){	
			if(!SemaPause){
				ST7735_FillScreen(0x0000);// set screen to black
				UpdateScore(score);
			}
		}
	}
}
int GameStatus(void){
  return GameStat;
}
void PlayerFire(void){
	if(LaserNumP < 1){																		// does not exceed max # of player lasers
		PlayerLasers[LaserNumP].xP = (Player.xP + 5);	// laser is 2 x 10, places laser at center of player ship
		PlayerLasers[LaserNumP].yP = (Player.yP - 8);	// places laser at front of player ship
		PlayerLasers[LaserNumP].image = PlayerLaser1;				// shoots player laser
		PlayerLasers[LaserNumP].velocity = 2;									// speed that player laser moves at
		PlayerLasers[LaserNumP].deathstate = 0;									// player laser is live
		LaserNumP++;																				// increment count of player lasers
	}
}
void EnemyFire(void){
	if(FireDelay > 0){						// random delay randomizes enemy firing scheme
		FireDelay--;								// decrement delay and return
		return;
	}
	else{
		short check;									// new randomized delay
		check = RandomNumber();		// create new delay using a random number to randomize firing
		check*=4;
		FireDelay = check;					
	}
	uint8_t k = 0;								// index for aliens
	for(; k < 18; k++){
		uint8_t fire = 0;						// allows alien to fire
		
		fire = (RandomNumber()%2); 		// boolean result decides if enemy fires
		
		if((Enemies[k].HP > 0) && (LaserNumE < 4) && (fire == 1)){	// fire if enemy is alive, there are less than max # of enemy lasers on screen, and boolean allows fire 
			EnemyLasers[k].xP = (Enemies[k].xP + (16 / 2));							// places laser at center of width of alien
			EnemyLasers[k].yP = Enemies[k].yP + 10;											// places laser at bottom of alien
			EnemyLasers[k].image = EnemyLaser1;															// assigns sprite of alien laser
			EnemyLasers[k].velocity = 2;																				// sets speed of laser
			EnemyLasers[k].deathstate = 0;																				// enemy laser is live
			LaserNumE++;																							// increment laser count
			return;
		}
	}
}
void PlayerHit(void){
	if(Player.HP > 0){
		char q = 0;										// index for enemy lasers
		for(;q < 4; q++){
			if((EnemyLasers[q].deathstate == 0)&&		
				 ((EnemyLasers[q].xP >= Player.xP) && (EnemyLasers[q].xP < (Player.xP + 12))) &&
			   ((EnemyLasers[q].yP >= (Player.yP + 18)))){ // check if enemy laser is live, if enemy laser touches player model
				if(Player.HP > 0){				
					Player.HP--;						// if player is alive, decrement health
					if(Player.HP == 0){			
						Player.deathstate = 1;			// if player health reaches 0, set recent death flag
						LFlag = 1;					// set gameOver flag to true
						Loss();						// run game over
						Sound_Explosion();		// play explosion sound
					}
				}
				
				ST7735_DrawBitmap(EnemyLasers[q].xP, EnemyLasers[q].yP, BlackLaser, 2, 10); // draw black space at laser after hit
				LaserNumE--;				// decrement enemy laser count
				EnemyLasers[q].deathstate = 1;	// kill enemy laser
				
			}
		}
	}
}
void EnemyHit(void){
	char p = 0;															// enemy array index
	char v = 0;															// player laser array index
	for(; p < 18; p++){
		if(Enemies[p].HP > 0){								// checks if enemy is alive
			v = 0;															// reset player laser index
			for(;v < 1; v++){										// checks all player lasers
				if((PlayerLasers[v].deathstate == 0)&&
					((PlayerLasers[v].xP >= Enemies[p].xP) && (PlayerLasers[v].xP < (Enemies[p].xP + 16))) &&
					((PlayerLasers[v].yP <= (Enemies[p].yP + 10)))){ // if player laser is live and touching enemy model
					if(Enemies[p].HP > 0){			
						Enemies[p].HP--;							// if enemy is alive, decrement enemy health
						if(Enemies[p].HP == 0){		
							Enemies[p].deathstate = 1;				// if enemy has just died, set recent death flag
							score += Enemies[p].pointval; // add point value of enemy to score
							UpdateScore(score);					// update score
							LivingE--;							// decrease number of alive enemies
							DeadE++;						// increase number of dead enemies
							Sound_Killed();			// play enemy destroyed sound
						}
					}
					
					LaserNumP--;							// decrement player laser counter
					PlayerLasers[v].deathstate = 1;				// player laser is dead
					ST7735_DrawBitmap(PlayerLasers[v].xP, PlayerLasers[v].yP, BlackLaser, 2, 10); // draw black space at laser after hit													
				}	
			}
		}
	}
}
void Hits(void){
	PlayerHit();
	EnemyHit();
}
void GameFlow(void){
  MoveAll();
	Hits();
  Pause();
	Create();
  Win();
}


int main(void){
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz
	DAC_Init();
  Sound_Init();
	ADC0_Init();
	PortF_Init();
	PortE_Init();
	SysTick_Init();
	Random_Init(NVIC_ST_CURRENT_R);
  Output_Init();
	
	unsigned long state=0; //state holder
	CreateStartScn();
	while(state==0){
		StartScn();
		state=StartCheck();
	}
	while(1){
		if(state==1){
			
				ST7735_FillScreen(0x0000);
				GameInit();
				SetButton1(0);
				SetButton2(0);
				Timer1_Init(MoveEnemy, 8000000);	// enemy movement at around 10 Hz
				Timer0_Init(EnemyFire, 8000000);
				while(state){
					GameFlow();
					state=GameStatus();
			}
				WinScn();
			while(1){
			}
			
			
		}
	}

	//typedef enum {English, Spanish} Language_t;
	//Language_t myLanguage=English;
	//typedef enum {HELLO,SELECT,LANGUAGE,GAME,OVER} phrase_t;
	//const char Hello_English[]="HELLO";
	//const char Hello_Spanish[]="\xADHola!";
	//const char Select_English[]="SELECT";
	//const char Select_Spanish[]="ESCOGER";
	//const char Game_English[]="GAME";
	//const char Game_Spanish[]="JUEGO";
	//const char Over_Spanish[]="TERMINADO";
//	const char Over_English[]="OVER";
	//onst char Language_Spanish[]="Espa\xA4ol";
	//const char Language_English[]="English";
//	const char *Phrases[5][2]={
//		{Hello_English,Hello_Spanish},
//		{Select_English,Select_Spanish},
//		{Language_English, Language_Spanish},
//		{Game_English, Game_Spanish},
//		{Over_English, Over_Spanish}
//	};

}
// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays

void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
