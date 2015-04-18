/*****************************************************************
 * SmartStickBot.c
 *
 * Created: 11-03-2015 19:01:09
 *  Author: ARKA
 *****************************************************************/ 

#include <firebird/fbird.h>				
#include <math.h>

int counterforxb=1;
int shouldistart =0;
unsigned char sharp;
unsigned int distance;			//distance measured by sharp sensor
double xFinal = 1000, yFinal = 1000; //destination
double x=0, y=0; //current position
float epsilon = 50;						//error in final coordinates	(may need to increase this a bit)
double angle=0; //orientation of bot w.r.t. x-axis
double theta ;	//angle to be rotated for reaching final destination
float rotate=90;	 //angle to rotated when obstacle detected
float step = 10;	//distance covered (in mm) per forward function call 
float backstep=50;	//go back 50 cm

unsigned char data;

void positive_obstacle();			//evasion of positive obstacle
void negative_obstacle();			//evasion of negative obstacle
int p_obs();						//returns 1 if there is a positive obstacle
int n_obs();						//returns 1 if there is a negative obstacle
void align();					//aligns itself to the destination
void changeCoordinatesBy(float);	//changes the co-ordinates
int reachedDestination();		//actually a bool, returns true if destination is reached

void uart0_init(void)				/**nice code
@author Arka*/
{	
	UCSR0B = 0x00; //disable while setting baud rate
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	// UBRR0L = 0x47; //11059200 Hz
	UBRR0L = 0x5F; // 14745600 Hzset baud rate lo
	UBRR0H = 0x00; //set baud rate hi
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

void duplicatemain()
{
	if(shouldistart==1)
	{
		sei();
		align();
				
		while( !reachedDestination() )
		{
			positive_obstacle();
			negative_obstacle();
			_delay_ms(10);
			
			forward_mm(step);
			changeCoordinatesBy(step);
					
		}
		stop();
	}
}
int main(void)
{
	
	port_init();
	while(1)
	{
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
		//30degrees turning buffer
		left_degrees(3*rotate);	
		angle+=3*rotate;
		
		forward_mm(50);		
		changeCoordinatesBy(50);
	}


	int d=100;
	forward_mm(d);changeCoordinatesBy(d);

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
		 counterforxb=2;
		 shouldistart = 0;
	}
	else if (counterforxb==2)
	{
		getDestinationY();
		counterforxb=1;
		shouldistart =1;
		duplicatemain();
	}	
}

void align()
{		  
	theta = atan2(yFinal-y,xFinal-x)*180/M_PI; //angle made by line joining current position to destination with x-axis
	
	if(theta>(angle+1))
	{
		left_degrees(theta-angle);
	}
	else
	{
		right_degrees(angle-theta);
	}
	
	angle=theta;
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
	double dx = abs(xFinal - x);
	double dy = abs(yFinal - y);
	
	if(dx<epsilon && dy<epsilon)
	{
		//UDR0 = 0xFF;	
		return 1;
	}		
	else
	{
		//UDR0 = 0x00;
		return 0;
	}		
}


int p_obs()
{
	/*unsigned char sharp;			//sharp sensor analog value
	unsigned char p_channel = 11;   //channel number of front sharp sensor
	unsigned int distance;			//distance measured by sharp sensor
	
	sharp = ADC_Conversion(p_channel);
	distance = Sharp_GP2D12_estimation(sharp);

	if (distance<200)
	{
		return 1;
	}
	else
	{
		return 0;
	}*/

	sharp = ADC_Conversion(11);
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
	unsigned char n_channel = 12;   //channel number of front sharp sensor
	unsigned int distance;			//distance measured by sharp sensor
	
	sharp = ADC_Conversion(n_channel);
	distance = Sharp_GP2D12_estimation(sharp);

	if (distance>250)		//calibrated
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

