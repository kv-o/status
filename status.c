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
double  cputotal[3] = {0};
double  cpubusy;
char    *token;
int     mem = 0;
FILE    *memfile;
double  meminfo[10] = {0};
double  memfree;
char    batt[4];
FILE    *battfile;
char    buf[512];
char    datestr[32];
char    timestr[32];
time_t  now;
char    *format;

struct timespec request = {0, 500000000};
struct timespec remaining = {0, 500000000};
struct tm       *local;

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
		for (j = 0; j < 10; j++) {
			token = strtok(NULL, " ");
			k = atoi(token);
			cputotal[i] += k;
			if (j == 3)
				cpuidle[i] = k;
		}
		fclose(cpufile);
		nanosleep(&request, &remaining);
	}
	cpuidle[2] = cpuidle[1] - cpuidle[0];
	cputotal[2] = cputotal[1] - cputotal[0];
	if (cputotal[2] == 0)
		cpubusy = 100;
	else
		cpubusy = cpuidle[2] / cputotal[2] * 100;
	cpu = round(100 - cpubusy);

	/* memory */
	memfile = fopen("/proc/meminfo", "r");
	assert(memfile);
	n = fread(buf, 512, 1, memfile);
	assert(n == 1);
	token = strtok(buf, " ");
	for (i = 1; i < 10; i++) {
		token = strtok(NULL, " ");
		if (i == 1 || i == 3 || i == 7 || i == 9)
			meminfo[i] = atoi(token);
	}
	fclose(memfile);
	memfree = meminfo[3] + meminfo[7] + meminfo[9];
	if (meminfo[1] != 0)
		mem = round((meminfo[1] - memfree) / meminfo[1] * 100);

	/* battery */
	battfile = fopen("/sys/class/power_supply/BAT0/capacity", "r");
	assert(battfile);
	n = fread(buf, 4, 1, battfile);
	if (n != 1 && !feof(battfile)) {
		fprintf(stderr, "status: error reading battery status\n");
		return 1;
	}
	for (i = 0; i < 4; i++)
		if (buf[i] == '\n')
			buf[i] = '\0';
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
