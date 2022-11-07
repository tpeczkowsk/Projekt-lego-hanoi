#include "hFramework.h"
//#include "hCloudClient.h"
#include <stddef.h>
#include <stdio.h>

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
const uint64_t sek_20 = 20000;
const uint64_t sek_30 = 30000;
const uint64_t sek_60 = 60000;

/*--------- prototypy funkcji -------*/
// pneumatyka
void set_pomp(Pomp_t);
void pomp_task();

void hMain()
{
	sys.setLogDev(&Serial);
	sys.taskCreate(&pomp_task);
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
			timer_pomp = 5000;
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
			if (sys.getRefTime() > lastTick_pomp_wait + 2000)
			{
				timer_pomp = 2000;
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
