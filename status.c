#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

size_t  n;
int     i;
int     j;
int     k;
int     cpu = 0;
FILE    *cpufile;
double  cpuidle[3];
double  cputotal[3] = {0, 0, 0};
double  cpubusy;
char    *token;
int     mem = 0;
double  memfree;
char    batt[4];
FILE    *battfile;
char    buf[512];
char    datestr[32];
char    timestr[32];
time_t  now;
struct  tm *local;
char    *format;

struct timespec request = {0, 500000000};
struct timespec remaining = {0, 500000000};
struct sysinfo  meminfo;

int
main(void)
{
	/* cpu */
	for (i = 0; i < 2; i++) {
		cpufile = fopen("/proc/stat", "r");
		assert(cpufile);
		n = fread(buf, 512, 1, cpufile);
		assert(n == 1);
		buf[511] = 0;
		token = strtok(buf, " ");
		for (j = 0; j < 11; j++) {
			token = strtok(NULL, " ");
			if (j > 0) {
				k = atoi(token);
				cputotal[i] += k;
			}
			if (j == 3)
				cpuidle[i] = k;
		}
		fclose(cpufile);
		nanosleep(&request, &remaining);
	}
	cpuidle[3] = cpuidle[1] - cpuidle[0];
	cputotal[3] = cputotal[1] - cputotal[0];
	if (cputotal[3] == 0)
		cpubusy = 100;
	else
		cpubusy = cpuidle[3] / cputotal[3] * 100;
	cpu = round(100 - cpubusy);

	/* memory */
	assert(sysinfo(&meminfo) != -1);
	memfree = meminfo.freeram + meminfo.bufferram + meminfo.sharedram;
	mem = round(((meminfo.totalram - memfree) / meminfo.totalram) * 100);

	/* battery */
	battfile = fopen("/sys/class/power_supply/BAT0/capacity", "r");
	assert(battfile);
	n = fread(buf, 3, 1, battfile);
	assert(n == 1);
	buf[3] = '\0';
	strcpy(batt, buf);
	fclose(battfile);

	/* datetime */
	now = time(0);
	local = localtime(&now);
	n = strftime(datestr, sizeof(datestr), "%Y-%m-%d", local);
	assert(n != 0);
	n = strftime(timestr, sizeof(timestr), "%H:%M:%S", local);
	assert(n != 0);

	format = "│ cpu %d%% │ mem %d%% │ batt %s%% │ %s │ %s \n";
	printf(format, cpu, mem, batt, datestr, timestr);
}
