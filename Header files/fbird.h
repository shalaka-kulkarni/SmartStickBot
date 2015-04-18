/*
 * fbird.h
 * Author: ARKA
 */ 

#ifndef FBIRD_H_INCLUDED
#define FBIRD_H_INCLUDED
#endif // FBIRD_H_INCLUDED

#define F_CPU 14745600
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>	//for pow function

	volatile unsigned long int ShaftCountLeft = 0; //to keep track of left position encoder
	volatile unsigned long int ShaftCountRight = 0; //to keep track of right position encoder
	volatile unsigned int Degrees; //to accept angle in degrees for turning		  


/*
Functions listed by order:
	>>Buzzer:
		>void buzzer_pin_config (void): to initialize Buzzer   
		>void buzzer_pin_config (void): to switch the buzzer on
		>void buzzer_off (void): to switch the buzzer on 
	>>Motion:
		>void motion_pin_config (void): to initialize motor pins
		>void motion_set (unsigned char Direction) :to move the bot in the specific direction
		>directions are: forward, back, left, right, soft_left, soft_right, soft_left_2, soft_right_2, stop
	>>Velocity (pwm mode):
		>void timer5_init()	: function to initialize the timer5 register
								// Timer 5 initialized in PWM mode for velocity control
								// Prescale:256
								// PWM 8bit fast, TOP=0x00FF
								// Timer Frequency:225.000Hz

		>void velocity (unsigned char left_motor, unsigned char right_motor): for speed control
					//uses non-inverting mode;
	>>LCD: connections
			 LCD Connections:
			 LCD	  Microcontroller Pins
			 RS  --> PC0
			 RW  --> PC1
			 EN  --> PC2
			 DB7 --> PC7
			 DB6 --> PC6
			 DB5 --> PC5
			 DB4 --> PC4
	
	>>ADC conncections:
 			  ACD CH.	PORT	Sensor
 			  0			PF0		Battery Voltage
 			  1			PF1		White line sensor 3
 			  2			PF2		White line sensor 2
 			  3			PF3		White line sensor 1
 			  4			PF4		IR Proximity analog sensor 1*****
 			  5			PF5		IR Proximity analog sensor 2*****
 			  6			PF6		IR Proximity analog sensor 3*****
 			  7			PF7		IR Proximity analog sensor 4*****
 			  8			PK0		IR Proximity analog sensor 5
 			  9			PK1		Sharp IR range sensor 1
 			  10		PK2		Sharp IR range sensor 2
 			  11		PK3		Sharp IR range sensor 3
 			  12		PK4		Sharp IR range sensor 4
 			  13		PK5		Sharp IR range sensor 5
 			  14		PK6		Servo Pod 1
 			  15		PK7		Servo Pod 2
			  ***** For using Analog IR proximity (1, 2, 3 and 4) sensors short the jumper J2.
			  To use JTAG via expansion slot of the microcontroller socket remove these jumpers.
			  
			  
		>void adc_pin_config (void) : initializes portf,k to input, for adc
		>void adc_init() :initializes the adc registers ADCSRA, ADCSRB, ADMUX, ACSR
		>unsigned char ADC_Conversion(unsigned char Ch) : takes in the channel number and prints out the analog value
		>unsigned int Sharp_GP2D12_estimation(unsigned char adc_reading): returns the distance in mm of the sharp sensor.
		
	>>Position encoder
		
		for the code to run properly: 
		1>initially define the following as global variables
		****
				volatile unsigned long int ShaftCountLeft = 0; //to keep track of left position encoder
				volatile unsigned long int ShaftCountRight = 0; //to keep track of right position encoder
				volatile unsigned int Degrees; //to accept angle in degrees for turning		  
		****
		2>And include the interrupt for the two ports:
		****
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
		****
		>void position_encode(): initializes the four functions given below
		>void left_encoder_pin_config (void): initializes PORTE, pin4 as input
		>void right_encoder_pin_config (void): initializes PORTE, pin5 as input
		>void left_position_encoder_interrupt_init (void): Interrupt 4 enable
		>void right_position_encoder_interrupt_init (void): Interrupt 5 enable
		>void angle_rotate(unsigned int Degrees): to rotate a particular angle
		>void linear_distance_mm(unsigned int DistanceInMM): to move a given distance
		>to move a certain distance/rotate by a particular degrees: use forward_mm, back_mm, left_degrees...
		
*/

//Buzzer functions

		//Function to initialize Buzzer
		void buzzer_pin_config (void)
		{
			DDRC = DDRC | 0x08;		//Setting PORTC 3 as output
			PORTC = PORTC & 0xF7;		//Setting PORTC 3 logic low to turnoff buzzer
		}

		void buzzer_on (void)
		{
			unsigned char port_restore = 0;
			port_restore = PINC;
			port_restore = port_restore | 0x08;
			PORTC = port_restore;
		}

		void buzzer_off (void)
		{
			unsigned char port_restore = 0;
			port_restore = PINC;
			port_restore = port_restore & 0xF7;
			PORTC = port_restore;
		}

//Motion functions
			void motion_pin_config (void)
			{
				DDRA = DDRA | 0x0F; //set direction of the PORTA 3 to PORTA 0 pins as output
				PORTA = PORTA & 0xF0; // set initial value of the PORTA 3 to PORTA 0 pins to logic 0
				DDRL = DDRL | 0x18;   //Setting PL3 and PL4 pins as output for PWM generation
				PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM
			}

			void motion_set (unsigned char Direction)
			{
				unsigned char PortARestore = 0;

				Direction &= 0x0F; 			// removing upper nibbel as it is not needed
				PortARestore = PORTA; 			// reading the PORTA's original status
				PortARestore &= 0xF0; 			// setting lower direction nibbel to 0
				PortARestore |= Direction; 	// adding lower nibbel for direction command and restoring the PORTA status
				PORTA = PortARestore; 			// setting the command to the port
			}


					void forward (void) //both wheels forward
					{
						motion_set(0x06);
					}

					void back (void) //both wheels backward
					{
						motion_set(0x09);
					}

					void left (void) //Left wheel backward, Right wheel forward
					{
						motion_set(0x05);
					}

					void right (void) //Left wheel forward, Right wheel backward
					{
						motion_set(0x0A);
					}

					void soft_left (void) //Left wheel stationary, Right wheel forward
					{
						motion_set(0x04);
					}

					void soft_right (void) //Left wheel forward, Right wheel is stationary
					{
						motion_set(0x02);
					}

					void soft_left_2 (void) //Left wheel backward, right wheel stationary
					{
						motion_set(0x01);
					}

					void soft_right_2 (void) //Left wheel stationary, Right wheel backward
					{
						motion_set(0x08);
					}

					void stop (void) //hard stop
					{
						motion_set(0x00);
					}	
					

//Velocity: pwm (fast) 8 bit mode
		// Timer 5 initialized in PWM mode for velocity control
		// Prescale:256
		// PWM 8bit fast, TOP=0x00FF
		// Timer Frequency:225.000Hz
		void timer5_init()
		{
			TCCR5B = 0x00;	//Stop
			TCNT5H = 0xFF;	//Counter higher 8-bit value to which OCR5xH value is compared with
			TCNT5L = 0x01;	//Counter lower 8-bit value to which OCR5xH value is compared with
			OCR5AH = 0x00;	//Output compare register high value for Left Motor
			OCR5AL = 0xFF;	//Output compare register low value for Left Motor
			OCR5BH = 0x00;	//Output compare register high value for Right Motor
			OCR5BL = 0xFF;	//Output compare register low value for Right Motor
			OCR5CH = 0x00;	//Output compare register high value for Motor C1
			OCR5CL = 0xFF;	//Output compare register low value for Motor C1
			TCCR5A = 0xA9;	/*{COM5A1=1, COM5A0=0; COM5B1=1, COM5B0=0; COM5C1=1 COM5C0=0}
 							  For Overriding normal port functionality to OCRnA outputs.
				  			  {WGM51=0, WGM50=1} Along With WGM52 in TCCR5B for Selecting FAST PWM 8-bit Mode*/
	
			TCCR5B = 0x0B;	//WGM12=1; CS12=0, CS11=1, CS10=1 (Prescaler=64)
		}
		
		//function for velocity
		void velocity (unsigned char left_motor, unsigned char right_motor)
		{
			OCR5AL = (unsigned char)left_motor;
			OCR5BL = (unsigned char)right_motor;
		}

//ADC:
		//ADC pin configuration
		void adc_pin_config (void)
		{
			DDRF = 0x00; //set PORTF direction as input
			PORTF = 0x00; //set PORTF pins floating
			DDRK = 0x00; //set PORTK direction as input
			PORTK = 0x00; //set PORTK pins floating
		}
		//initializing the adc registers
		//prescalar 64
		void adc_init()
		{
			ADCSRA = 0x00;
			ADCSRB = 0x00;		//MUX5 = 0
			ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
			ACSR = 0x80;
			ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
		}
		
		
	//This Function accepts the Channel Number and returns the corresponding Analog Value
	unsigned char ADC_Conversion(unsigned char Ch)
	{
		unsigned char a;
		if(Ch>7)
		{
			ADCSRB = 0x08;
		}
		Ch = Ch & 0x07;
		ADMUX= 0x20| Ch;
		ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
		while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
		a=ADCH;
		ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
		ADCSRB = 0x00;
		return a;
	}
	
	// This Function calculates the actual distance in millimeters(mm) from the input
	// analog value of Sharp Sensor.
	unsigned int Sharp_GP2D12_estimation(unsigned char adc_reading)
	{
		float distance;
		unsigned int distanceInt;
		distance = (int)(10.00*(2799.6*(1.00/(pow(adc_reading,1.1546)))));
		distanceInt = (int)distance;
		if(distanceInt>800)
		{
			distanceInt=800;
		}
		return distanceInt;
	}
	  
//Position encoder

		void left_encoder_pin_config (void)
		{
			DDRE  = DDRE & 0xEF;  //Set the direction of the PORTE 4 pin as input
			PORTE = PORTE | 0x10; //Enable internal pull-up for PORTE 4 pin
		}

		//Function to configure INT5 (PORTE 5) pin as input for the right position encoder
		void right_encoder_pin_config (void)
		{
			DDRE  = DDRE & 0xDF;  //Set the direction of the PORTE 4 pin as input
			PORTE = PORTE | 0x20; //Enable internal pull-up for PORTE 4 pin
		}

		void left_position_encoder_interrupt_init (void) //Interrupt 4 enable
		{
			cli(); //Clears the global interrupt
			EICRB = EICRB | 0x02; // INT4 is set to trigger with falling edge
			EIMSK = EIMSK | 0x10; // Enable Interrupt INT4 for left position encoder
			sei();   // Enables the global interrupt
		}

		void right_position_encoder_interrupt_init (void) //Interrupt 5 enable
		{
			cli(); //Clears the global interrupt
			EICRB = EICRB | 0x08; // INT5 is set to trigger with falling edge
			EIMSK = EIMSK | 0x20; // Enable Interrupt INT5 for right position encoder
			sei();   // Enables the global interrupt
		}
		
		void position_encode()
		{
			left_encoder_pin_config();
			right_encoder_pin_config();
			left_position_encoder_interrupt_init();
			right_position_encoder_interrupt_init();
		}

		//Function used for turning robot by specified degrees
		void angle_rotate(unsigned int Degrees)
		{
			float ReqdShaftCount = 0;
			unsigned long int ReqdShaftCountInt = 0;

			ReqdShaftCount = (float) Degrees/ 4.090; // division by resolution to get shaft count
			ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
			ShaftCountRight = 0;
			ShaftCountLeft = 0;

			while (1)
			{
				if((ShaftCountRight >= ReqdShaftCountInt) | (ShaftCountLeft >= ReqdShaftCountInt))
				break;
			}
			stop(); //Stop robot
		}

		void linear_distance_mm(unsigned int DistanceInMM)
		{
			float ReqdShaftCount = 0;
			unsigned long int ReqdShaftCountInt = 0;

			ReqdShaftCount = DistanceInMM / 5.338; // division by resolution to get shaft count
			ReqdShaftCountInt = (unsigned long int) ReqdShaftCount;
			
			ShaftCountRight = 0;
			while(1)
			{
				if(ShaftCountRight > ReqdShaftCountInt)
				{
					break;
				}
			}
			stop(); //Stop robot
		}
					
					
		//actual commands
				void forward_mm(unsigned int DistanceInMM)
				{
					forward();
					linear_distance_mm(DistanceInMM);
				}

				void back_mm(unsigned int DistanceInMM)
				{
					back();
					linear_distance_mm(DistanceInMM);
				}

				void left_degrees(unsigned int Degrees)
				{
					// 88 pulses for 360 degrees rotation 4.090 degrees per count
					left(); //Turn left
					angle_rotate(Degrees);
				}



				void right_degrees(unsigned int Degrees)
				{
					// 88 pulses for 360 degrees rotation 4.090 degrees per count
					right(); //Turn right
					angle_rotate(Degrees);
				}


				void soft_left_degrees(unsigned int Degrees)
				{
					// 176 pulses for 360 degrees rotation 2.045 degrees per count
					soft_left(); //Turn soft left
					Degrees=Degrees*2;
					angle_rotate(Degrees);
				}

				void soft_right_degrees(unsigned int Degrees)
				{
					// 176 pulses for 360 degrees rotation 2.045 degrees per count
					soft_right();  //Turn soft right
					Degrees=Degrees*2;
					angle_rotate(Degrees);
				}

				void soft_left_2_degrees(unsigned int Degrees)
				{
					// 176 pulses for 360 degrees rotation 2.045 degrees per count
					soft_left_2(); //Turn reverse soft left
					Degrees=Degrees*2;
					angle_rotate(Degrees);
				}

				void soft_right_2_degrees(unsigned int Degrees)
				{
					// 176 pulses for 360 degrees rotation 2.045 degrees per count
					soft_right_2();  //Turn reverse soft right
					Degrees=Degrees*2;
					angle_rotate(Degrees);
				}



		

