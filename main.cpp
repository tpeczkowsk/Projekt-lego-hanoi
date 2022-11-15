#include "hFramework.h"
//#include "hCloudClient.h"
#include <stddef.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>

using namespace hFramework;

/*------ zmienne -----*/
#define ROT_RATIO 2.33333
typedef enum
{
	ENABLED,
	DISABLE,
	WAIT,
	FIRST_ENABLE
} Pomp_t;
typedef enum
{
	ON,
	OFF
} Gripper_t;
typedef enum
{
	GET_SIZE,
	ROT_ZERO,
	HEIGHT_ZERO,
	WORKING
} Robot_t;
// pneumatyka
uint64_t lastTick_pomp_enable;
uint64_t lastTick_pomp_wait;
uint64_t timer_pomp;
Pomp_t pomp = FIRST_ENABLE;
Gripper_t gripper;
const uint64_t pomping_duration = 4000;
const uint64_t wait_duration = 2000;
const uint64_t first_enable_duration = 8000;

// krańcówki
bool rot_zero_position;
bool height_zero;

// hanoi
uint8_t towers[3] = {0};
char receive[2];
int rozmiar;
int *moves_array;
int move_index = 0;
int array_size = 0;
Robot_t robot = GET_SIZE;

// obrót
const int rot_speed = 400;

// wysokość
const int height_speed = 1000;
const int height_of_tire;
const int gripping_height; // do ustalenia
const int moving_height = 0;

/*--------- prototypy funkcji -------*/
// pneumatyka
void set_pomp(Pomp_t);
void pomp_task();
void Gripper(Gripper_t);

// krańcówki
void set_switches();
void check_switches();

// Hanoi
int get_size();
void solve_hanoi(uint8_t size, uint8_t from, uint8_t to, uint8_t aux);

// obrot
void set_rot_position(int);

// wysokosc
void set_height_position(int);

void hMain()
{
	hMot2.setMotorPolarity(Polarity::Normal);
	hMot2.setEncoderPolarity(Polarity::Reversed);
	hMot4.setMotorPolarity(Polarity::Normal);
	hMot4.setEncoderPolarity(Polarity::Reversed);
	hMot1.setMotorPolarity(Polarity::Normal);
	hMot1.setEncoderPolarity(Polarity::Reversed);
	set_switches();
	sys.setLogDev(&Serial);

	// for (int i = 0; i < array_size; i++)
	// {
	// 	printf("TO: %d\n", moves_array[i]);
	// }

	sys.taskCreate(&pomp_task);
	sys.taskCreate(&check_switches);
	sys.delay(10000);
	hMot2.resetEncoderCnt();
	hMot1.resetEncoderCnt();
	// hMot2.rotAbs(120 * ROT_RATIO, 200, 1, INFINITE);
	//    platform.begin(&RPi);
	for (;;)
	{
		switch (robot)
		{
		case GET_SIZE:
			rozmiar = get_size();
			// rozmiar = 3;
			array_size = (pow(2, rozmiar) - 1) * 2;
			moves_array = new int[array_size];
			solve_hanoi(rozmiar, 1, 3, 2);
			towers[0] = rozmiar;
			robot = ROT_ZERO;
			move_index = 0;
			break;
		case ROT_ZERO:
			hMot2.setPower(rot_speed);
			if (rot_zero_position)
			{
				hMot2.setPower(0);
				hMot2.resetEncoderCnt();
				robot = HEIGHT_ZERO;
			}
			break;
		case HEIGHT_ZERO:
			hMot1.setPower(height_speed);
			if (height_zero)
			{
				hMot1.setPower(0);
				hMot1.resetEncoderCnt();
				robot = WORKING;
			}
			break;
		case WORKING:
			if (move_index < array_size)
			{
				set_rot_position(moves_array[move_index]);
				sys.delay(500);

				if (move_index % 2 == 0)
				{
					set_height_position(gripping_height + height_of_tire * (towers[moves_array[move_index] - 1]));
					sys.delay(500);
					Gripper(OFF);
					towers[moves_array[move_index] - 1]++;
				}
				else
				{
					set_height_position(gripping_height + height_of_tire * (towers[moves_array[move_index] - 1] - 1));
					sys.delay(500);
					Gripper(ON);
					towers[moves_array[move_index] - 1]--;
				}
				sys.delay(2000);
				set_height_position(moving_height);
				sys.delay(500);
				move_index++;
			}
			robot = GET_SIZE;
			break;
		default:
			break;
		}
		sys.delay(1);
		// Gripper(ON);
		// sys.delay(2000);
		// hMot1.rotAbs(5000, 1000, 1, INFINITE);
		// set_rot_position(3);
		// sys.delay(500);
		// hMot1.rotAbs(0, 1000, 1, INFINITE);
		// sys.delay(500);
		// Gripper(OFF);
		// sys.delay(2000);
		// hMot1.rotAbs(5000, 1000, 1, INFINITE);
		// set_rot_position(1);
		// sys.delay(500);
		// hMot1.rotAbs(0, 1000, 1, INFINITE);
		// sys.delay(500);
	}
}
/*------ Ciała funkcji -------*/
// pneumatyka
void set_pomp(Pomp_t state)
{
	pomp = state;
}

void pomp_task()
{
	for (;;)
	{

		switch (pomp)
		{
		case FIRST_ENABLE:
			hMot3.setPower(1000);
			timer_pomp = first_enable_duration;
			lastTick_pomp_enable = sys.getRefTime();
			set_pomp(ENABLED);
			break;
		case ENABLED:
			if (sys.getRefTime() > lastTick_pomp_enable + timer_pomp)
			{
				hMot3.setPower(0);
				lastTick_pomp_wait = sys.getRefTime();
				set_pomp(WAIT);
			}
			break;
		case WAIT:
			if (sys.getRefTime() > lastTick_pomp_wait + wait_duration)
			{
				timer_pomp = pomping_duration;
				hMot3.setPower(1000);
				lastTick_pomp_enable = sys.getRefTime();
				set_pomp(ENABLED);
			}
			break;
		default:
			break;
		}
		sys.delay(1);
	}
}
void Gripper(Gripper_t gripper)
{
	switch (gripper)
	{
	case ON:
		hMot4.rotAbs(360, 200, 1, INFINITE);
		break;
	case OFF:
		hMot4.rotAbs(0, 200, 1, INFINITE);
		break;
	default:
		break;
	}
}
// krańcówki
void set_switches()
{
	// 1, zaklejony
	hSens1.pin3.setOut();
	hSens1.pin1.setIn_pu();
	hSens1.pin3.write(false);

	hSens2.pin3.setOut();
	hSens2.pin1.setIn_pu();
	hSens2.pin3.write(false);
}

void check_switches()
{
	for (;;)
	{
		rot_zero_position = hSens1.pin1.read();
		height_zero = hSens2.pin1.read();
		printf("rot: %d gripper: %d\n", rot_zero_position, height_zero);
		sys.delay(20);
	}
}

void solve_hanoi(uint8_t size, uint8_t from, uint8_t to, uint8_t aux)
{
	if (size == 1)
	{
		printf("MOVE FROM %d TO %d\n", from, to);
		moves_array[move_index] = from;
		move_index++;
		moves_array[move_index] = to;
		move_index++;
	}
	else
	{
		solve_hanoi(size - 1, from, aux, to);
		printf("MOVE FROM %d TO %d\n", from, to);
		moves_array[move_index] = from;
		move_index++;
		moves_array[move_index] = to;
		move_index++;
		solve_hanoi(size - 1, aux, to, from);
	}
}

int get_size()
{
	int counter = 0;
	bool was_pressed = false;
	while (hBtn2.isPressed() != true)
	{
		if (hBtn1.isPressed() && !was_pressed)
		{
			counter++;
			printf("Pressed 1\n");
			was_pressed = true;
		}
		if (!hBtn1.isPressed())
		{
			was_pressed = false;
		}
		sys.delay(50);
	}
	printf("Pressed 2\n");
	return counter;
}

void set_rot_position(int pos)
{
	switch (pos)
	{
	case 1:
		hMot2.rotAbs(0, rot_speed, 1, INFINITE);
		break;
	case 2:
		hMot2.rotAbs(-120 * ROT_RATIO, rot_speed, 1, INFINITE);
		break;
	case 3:
		hMot2.rotAbs(-240 * ROT_RATIO, rot_speed, 1, INFINITE);
		break;
	default:
		break;
	}
}

void set_height_position(int height)
{
	hMot1.rotAbs(height, height_speed, 1, INFINITE);
}