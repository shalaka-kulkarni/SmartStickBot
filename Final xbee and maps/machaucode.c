/*****************************************************************
 * SmartStickBot.c
 *
 * Created: 11-03-2015 19:01:09
 *  Author: ARKA, SHALAKA
 *****************************************************************/ 

#include "fbird.h"
#include <math.h>


/**Sharp sensor operational variables*/

unsigned char sharp;			//stores the value input from sharp sensor
unsigned int distance;			//stores the distance measured by sharp sensor


/** Ground traversal operational variables (Distances in mm)  */

double xFinal = 1000, yFinal = 1000; 	//destination coordinates, initialized to (1m,1m)
double x = 0, y = 0; 					//current position
float epsilon = 50;						//permissible error in final coordinates	
double angle = 0; 						//orientation of bot w.r.t. x-axis
double theta;							//angle made by line joining the bot to the destination, with x-axis
float rotate = 20;	 					//angle to rotated every time an obstacle is detected
float step = 10;						//distance covered per forward function call 
float backstep = 50;					//distance covered in backtracking from an obstacle 
int counterfordestination = 0;			//takes value i for i'th step point


/** X-Bee operational variables */

unsigned char data;
int counterforxb = 1; 			//value of the counter decides whether input is taken for x or y
int shouldistart = 0;			//flag to determine if the bot should start moving (1) or not (0) 


/** Function declarations */

void positive_obstacle();			//evasion of positive obstacle
void negative_obstacle();			//evasion of negative obstacle
int p_obs();						//checks for presence of positive obstacle
int n_obs();						//checks for presence of negative obstacle
void align();						//aligning the bot towards the destination from the current position
void changeCoordinatesBy(float);	//changes the co-ordinates of bot position by distance covered at the given angle 
int reachedDestination();			//checks if destination is reached
void getDestinationX();				//receives and stores x-coordinate sent from PC via XBee 
void getDestinationY();				//receives and stores y-coordinate sent from PC via XBee 


/** Function definitions */

void uart0_init(void)
{
	UCSR0B = 0x00; 			//disable while setting baud rate
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	UBRR0L = 0x5F; 			// 14745600 Hz set baud rate low
	UBRR0H = 0x00;			//set baud rate high
	UCSR0B = 0x98;
}


void port_init()
{
	cli();						//clear the interrupts
	motion_pin_config ();		//initialize the motion pins
	
	timer5_init();				//for pwm
	
// 	lcd_port_config();			//initialize the lcd ports
// 	lcd_init();				
	
	adc_pin_config();		    
	adc_init();		
	
	position_encode();
	
	uart0_init();
	
	DDRH |= 1<<PINH3;
	PORTH |= 1<<PINH3;
	
	DDRH |= 1<<PINH2;
	PORTH |= 1<<PINH2;
	
	
	sei();
}


void getDestinationX()
{
	xFinal = 1000*data;
	UDR0 = 1+data;
}

void getDestinationY()
{
	yFinal = 1000*data;
	UDR0 = 2+data;
}

int  main()
{
	port_init();
	;
}

void move()
{
	while(1)
	{
	   	if(shouldistart==1)
		{
			sei();								//enable the global interrupt
			align();							//align the bot to the destination (xFinal, yFinal)
				
			while( !reachedDestination() )	
			{
				
				//Detect and evade obstacles
				negative_obstacle();
				positive_obstacle();			
				
				_delay_ms(10);
			
				//Move forward
				forward_mm(step);
				changeCoordinatesBy(step);
						
			}
			
			stop();
		}
	}

}

void positive_obstacle()
{
	int p_obs_found = 0;
	
	while(p_obs()==1)
	{
		left_degrees(rotate);
		angle += rotate;
		_delay_ms(10);
		
		p_obs_found=1;
	}
	
	if(p_obs_found==1)
	{
		//Turning buffer to completely evade the obstacle once out of the line of sight
		left_degrees(3*rotate);	
		angle+=3*rotate;
		
		forward_mm(50);		
		changeCoordinatesBy(50);
	}

	align();
}

void negative_obstacle()
{
	int n_obs_found = 0;
	
	while(n_obs()==1)
	{
		back_mm(backstep);
		changeCoordinatesBy(-backstep);
		
		left_degrees(rotate);
		angle += rotate;
		
		n_obs_found=1;
	}
	
	if(n_obs_found==1)
	{
		//Turning buffer to completely evade the obstacle once out of the line of sight
		back_mm(2*backstep);	
		changeCoordinatesBy(-2*backstep);
		
		left_degrees(3*rotate);
		angle+=3*rotate;
	}

	align();
}

//ISR for right position encoder
ISR(INT5_vect)
{
	ShaftCountRight++;  //increment right shaft position count
}


//ISR for left position encoder
ISR(INT4_vect)
{
	ShaftCountLeft++;  //increment left shaft position count
}

SIGNAL(SIG_USART0_RECV) 		// ISR for receive complete interrupt
{
	data = UDR0; 				//making copy of data from UDR0 in 'data' variable
	
	//UDR0 = data;				//echo data back to PC
	if (counterforxb==1)
	{
		 getDestinationX();
		 counterforxb = 2;		//this is to allow input of y-coordinate only if x-cordinate has been received
		 shouldistart = 0;
	}
	else if (counterforxb==2)
	{
		getDestinationY();
		counterforxb = 1;		//counter is reset to 1 to allow input of next x-coordinate
		
		counterfordestination ++;
		
		shouldistart = 1;		//bot is allowed to start moving only after both x and y coordinates have been received
		move();
	}	
}

void align()
{		  
	theta = atan2(yFinal-y,xFinal-x)*180/M_PI; 
	
	//PRECONDITION : the bot should be aligned to the first destination
	
	if(counterfordestination == 1) //first step point
	{
		angle = theta;
	}
	
	else
	{
	
		//left_degrees(unsigned int) does not take a negative angle argument
	
		if(theta>(angle+1))
		{
			left_degrees(theta-angle);
		}
	
		else
		{
			right_degrees(angle-theta);
		}
	
		angle = theta;
	}
}

void changeCoordinatesBy(float distanceCovered)
{
	x += distanceCovered*cos(angle*M_PI/180);
	y += distanceCovered*sin(angle*M_PI/180);
}

int abs(int a)
{
	if(a>0)
		return a;
	else
		return -a;
}

int reachedDestination()
{
	//distance of current position from final destination
	double dx = abs(xFinal - x);	 
	double dy = abs(yFinal - y);
	
	if(dx<epsilon && dy<epsilon)
	{
		//Preparing for next lap by initializing start point to (0,0)
		x = 0;
		y = 0;
		
		return 1;
	}		
	else
	{
		return 0;
	}		
}


int p_obs()
{
	unsigned char sharp;			//sharp sensor analog value
	unsigned char p_channel = 11;   //channel number of front sharp sensor
	unsigned int distance;			//distance measured by sharp sensor
	
	sharp = ADC_Conversion(p_channel);
	distance = Sharp_GP2D12_estimation(sharp);

	if (distance<350)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int n_obs()
{
	unsigned char sharp;			//sharp sensor analog value
	unsigned char n_channel = 12;   //channel number of top sharp sensor
	unsigned int distance;			//distance measured by sharp sensor
	
	sharp = ADC_Conversion(n_channel);
	distance = Sharp_GP2D12_estimation(sharp);

	if (distance>250)				//calibrated
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

