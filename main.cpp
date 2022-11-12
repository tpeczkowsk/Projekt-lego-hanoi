#include "hFramework.h"
//#include "hCloudClient.h"
#include <stddef.h>
#include <stdio.h>
#include <cmath>
#include <cstdlib>

using namespace hFramework;

/*------ zmienne -----*/
typedef enum
{
	ENABLED,
	DISABLE,
	WAIT,
	FIRST_ENABLE
} Pomp_t;
// pneumatyka
uint64_t lastTick_pomp_enable;
uint64_t lastTick_pomp_wait;
uint64_t timer_pomp;
Pomp_t pomp = FIRST_ENABLE;
const uint64_t pomping_duration = 2000;
const uint64_t wait_duration = 3000;
const uint64_t first_enable_duration = 6000;

// krańcówki
bool rot_zero_position;
bool is_gripper_on;

// hanoi
uint8_t towers[3] = {0};
char receive[2];
int rozmiar;
int *moves_array;
int move_index = 0;

/*--------- prototypy funkcji -------*/
// pneumatyka
void set_pomp(Pomp_t);
void pomp_task();

// krańcówki
void set_switches();
void check_switches();

// Hanoi
int get_size();
void solve_hanoi(uint8_t size, uint8_t from, uint8_t to, uint8_t aux);
void hMain()
{
	sys.setLogDev(&Serial);
	rozmiar = get_size();
	// rozmiar = 3;
	int array_size = (pow(2, rozmiar) - 1) * 2;
	moves_array = new int[array_size];
	solve_hanoi(rozmiar, 1, 3, 2);
	for (int i = 0; i < array_size; i++)
	{
		printf("TO: %d\n", moves_array[i]);
	}

	sys.taskCreate(&pomp_task);
	sys.taskCreate(&check_switches);

	//   platform.begin(&RPi);
	for (;;)
	{
		hMot2.setPower(-1000);
		sys.delay(5000);
		hMot1.setPower(1000);
		hMot2.setPower(429);
		sys.delay(2000);
		hMot1.setPower(0);
		hMot2.setPower(1000);
		sys.delay(5000);
		hMot2.setPower(-429);
		hMot1.setPower(-1000);
		sys.delay(2000);
		hMot1.setPower(0);
		hMot2.setPower(0);
		sys.delay(500);
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

// krańcówki
void set_switches()
{
	hSens1.pin1.setOut();
	hSens1.pin3.setIn_pu();
	hSens1.pin1.write(false);

	hSens2.pin1.setOut();
	hSens2.pin3.setIn_pu();
	hSens2.pin1.write(false);
}

void check_switches()
{
	for (;;)
	{
		rot_zero_position = hSens1.pin3.read();
		is_gripper_on = hSens2.pin3.read();
		sys.delay(50);
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
