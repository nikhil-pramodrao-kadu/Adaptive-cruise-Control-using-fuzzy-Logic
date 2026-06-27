#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <electus_at16.h>

#define TRIG_PIN   PA0
#define ECHO_PIN   PA1
#define MIN_START_PWM 40

unsigned int target_speed = 0;
unsigned int current_speed = 0;
unsigned int speed_cmhr  = 0;


void Timer0_init(void)
{
	TCCR0 = (1<<CS01);
	TCNT0 = 0;
}

void triggerUltrasonic(void)
{
	PORTA &= ~(1 << TRIG_PIN);
	_delay_us(2);
	PORTA |= (1 << TRIG_PIN);
	_delay_us(10);
	PORTA &= ~(1 << TRIG_PIN);
}


uint16_t getPulse(void)
{
	uint16_t count = 0;

	while(!(PINA & (1<<ECHO_PIN)));
	TCNT0 = 0;

	while(PINA & (1<<ECHO_PIN))
	{
		if(TCNT0 >= 250)
		{
			count += 250;
			TCNT0 = 0;
		}
	}

	count += TCNT0;
	return count;
}


unsigned int getAverageDistance()
{
	unsigned long sum = 0;

	for(int i=0;i<3;i++)
	{
		triggerUltrasonic();
		sum += (getPulse()/58);
		_delay_ms(8);
	}

	return sum/3;
}

int main(void)
{
	unsigned int distance;

	DDRA |= (1 << TRIG_PIN);
	DDRA &= ~(1 << ECHO_PIN);

	motor_init();
	Timer0_init();
	lcd_init();

	motor1_clk;
	motor2_clk;

	motor1_speed = 0;
	motor2_speed = 0;

	lcd_clrscr();
	lcd_write_string("ACC System");
	_delay_ms(1000);

	while(1)
	{
		distance = getAverageDistance();

		
		if(distance <= 20)
		{
			target_speed = 0;
			current_speed = 0;
		}
		else if(distance <= 30)
		{
			target_speed = 80;
		}
		else if(distance <= 45)
		{
			target_speed = 130;
		}
		else if(distance <= 60)
		{
			target_speed = 180;
		}
		else if(distance <= 90)
		{
			target_speed = 220;
		}
		else
		{
			target_speed = 255;
		}

		
		if(distance > 20)
		{
			if(current_speed < target_speed)
			{
				if(current_speed < MIN_START_PWM)
				current_speed = MIN_START_PWM;
				else
				current_speed += 3;
			}
			else if(current_speed > target_speed)
			{
				current_speed -= 35;
			}
		}

		if(current_speed > 255)
		current_speed = 255;

		
		if(current_speed > 0 && current_speed < MIN_START_PWM)
		{
			motor1_speed = MIN_START_PWM;
			motor2_speed = MIN_START_PWM;
			_delay_ms(50);              // allow both motors to start together
		}

		/* Apply balanced speed */
		motor1_speed = current_speed;
		motor2_speed = current_speed+2;

		/* Convert PWM to cm/hr */
		speed_cmhr = (current_speed * 300) / 255;

		/* -------- LCD DISPLAY -------- */
		lcd_clrscr();
		lcd_write_string("Dist:");
		lcd_write_int(distance);
		lcd_write_string("cm");

		lcd_goto(1,2);
		lcd_write_string("Speed:");
		lcd_write_int(speed_cmhr);
		lcd_write_string("cm/hr");

		_delay_ms(8);
	}
}