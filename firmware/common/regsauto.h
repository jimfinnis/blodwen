/**
 * \file
 * Code generated automatically by regparse.hs - DO NOT MODIFY.
 */

#include "regs.h"
#define REG_RESET	0 	//reset bits - beware race conditions
#define REG_TIMER	1 	//millis since start
#define REG_INTERVALI2C	2 	//interval between I2C ticks
#define REG_STATUS	3 	//see status flags in regs.h
#define REG_DEBUGLED	4 	//debugging LEDs, turns on for some time
#define REG_EXCEPTIONDATA	5 	//LSB: type, MSB: id. Write causes REMOTE exception
#define REG_DISABLEDEXCEPTIONS	6 	//bitfield of disabled exceptions
#define REG_PING	7 	//debugging
#define REG_DEBUG	8 	//debugging





#define REGDS_DRIVE_REQSPEED	9 	//required speed
#define REGDS_DRIVE_PGAIN	10 	//P-gain
#define REGDS_DRIVE_IGAIN	11 	//I-gain
#define REGDS_DRIVE_DGAIN	12 	//D-gain
#define REGDS_DRIVE_INTEGRALCAP	13 	//integral error cap
#define REGDS_DRIVE_INTEGRALDECAY	14 	//integral decay
#define REGDS_DRIVE_OVERCURRENTTHRESH	15 	//overcurrent threshold
#define REGDS_DRIVE_ACTUALSPEED	16 	//actual speed from encoder
#define REGDS_DRIVE_ERROR	17 	//required minus actual speed
#define REGDS_DRIVE_ERRORINTEGRAL	18 	//error integral magnitude
#define REGDS_DRIVE_ERRORDERIV	19 	//error derivative
#define REGDS_DRIVE_CONTROL	20 	//value being sent to motor
#define REGDS_DRIVE_INTERVALCTRL	21 	//time between control runs (ms)
#define REGDS_DRIVE_CURRENT	22 	//raw current reading
#define REGDS_DRIVE_ODO	23 	//encoder ticks
#define REGDS_DRIVE_STALLCHECK	24 	//stall check control signal level
#define REGDS_DRIVE_DEADZONE	25 	//if below this value, error is set to zero
#define REGDS_STEER_REQPOS	26 	//required position
#define REGDS_STEER_PGAIN	27 	//P-gain
#define REGDS_STEER_IGAIN	28 	//I-gain
#define REGDS_STEER_DGAIN	29 	//D-gain
#define REGDS_STEER_INTEGRALCAP	30 	//integral error cap
#define REGDS_STEER_INTEGRALDECAY	31 	//integral decay
#define REGDS_STEER_OVERCURRENTTHRESH	32 	//overcurrent threshold
#define REGDS_STEER_ACTUALPOS	33 	//actual position from pot
#define REGDS_STEER_ERROR	34 	//required minus actual position
#define REGDS_STEER_ERRORINTEGRAL	35 	//error integral magnitude
#define REGDS_STEER_ERRORDERIV	36 	//error derivative
#define REGDS_STEER_CONTROL	37 	//value being sent to motor
#define REGDS_STEER_INTERVALCTRL	38 	//time between control runs (ms)
#define REGDS_STEER_CURRENT	39 	//raw current reading
#define REGDS_STEER_STALLCHECK	40 	//stall check control signal level
#define REGDS_STEER_DEADZONE	41 	//if below this value, error is set to zero
#define REGDS_STEER_CALIBMIN	42 	//minimum angle, mapped onto pot value 0
#define REGDS_STEER_CALIBMAX	43 	//maximum angle, mapped onto pot value 1024
#define REGDS_CHASSIS	44 	//chassis pot reading
#define NUMREGS_DS 45

extern MAYBEPROGMEM Register registerTable_DS[];


#define REGLL_ONE_REQPOS	9 	//required position
#define REGLL_ONE_PGAIN	10 	//P-gain
#define REGLL_ONE_IGAIN	11 	//I-gain
#define REGLL_ONE_DGAIN	12 	//D-gain
#define REGLL_ONE_INTEGRALCAP	13 	//integral error cap
#define REGLL_ONE_INTEGRALDECAY	14 	//integral decay
#define REGLL_ONE_OVERCURRENTTHRESH	15 	//overcurrent threshold
#define REGLL_ONE_ACTUALPOS	16 	//actual position from pot
#define REGLL_ONE_ERROR	17 	//required minus actual position
#define REGLL_ONE_ERRORINTEGRAL	18 	//error integral magnitude
#define REGLL_ONE_ERRORDERIV	19 	//error derivative
#define REGLL_ONE_CONTROL	20 	//value being sent to motor
#define REGLL_ONE_INTERVALCTRL	21 	//time between control runs (ms)
#define REGLL_ONE_CURRENT	22 	//raw current reading
#define REGLL_ONE_CALIBMIN	23 	//minimum angle, mapped onto pot value 0
#define REGLL_ONE_CALIBMAX	24 	//maximum angle, mapped onto pot value 1024
#define REGLL_ONE_STALLCHECK	25 	//stall check control signal level
#define REGLL_ONE_DEADZONE	26 	//if below this value, error is set to zero
#define REGLL_TWO_REQPOS	27 	//required position
#define REGLL_TWO_PGAIN	28 	//P-gain
#define REGLL_TWO_IGAIN	29 	//I-gain
#define REGLL_TWO_DGAIN	30 	//D-gain
#define REGLL_TWO_INTEGRALCAP	31 	//integral error cap
#define REGLL_TWO_INTEGRALDECAY	32 	//integral decay
#define REGLL_TWO_OVERCURRENTTHRESH	33 	//overcurrent threshold
#define REGLL_TWO_ACTUALPOS	34 	//actual position from pot
#define REGLL_TWO_ERROR	35 	//required minus actual position
#define REGLL_TWO_ERRORINTEGRAL	36 	//error integral magnitude
#define REGLL_TWO_ERRORDERIV	37 	//error derivative
#define REGLL_TWO_CONTROL	38 	//value being sent to motor
#define REGLL_TWO_INTERVALCTRL	39 	//time between control runs (ms)
#define REGLL_TWO_CURRENT	40 	//raw current reading
#define REGLL_TWO_CALIBMIN	41 	//minimum angle, mapped onto pot value 0
#define REGLL_TWO_CALIBMAX	42 	//maximum angle, mapped onto pot value 1024
#define REGLL_TWO_STALLCHECK	43 	//stall check control signal level
#define REGLL_TWO_DEADZONE	44 	//if below this value, error is set to zero
#define NUMREGS_LL 45

extern MAYBEPROGMEM Register registerTable_LL[];


#define REGMASTER_RESET	0 	//set to clear exception state
#define REGMASTER_TEMPAMBIENT	1 	//temperature sensor
#define REGMASTER_TEMP1	2 	//temperature sensor
#define REGMASTER_TEMP2	3 	//temperature sensor
#define REGMASTER_TEMP3	4 	//temperature sensor
#define REGMASTER_TEMP4	5 	//temperature sensor
#define REGMASTER_TEMP5	6 	//temperature sensor
#define REGMASTER_TEMP6	7 	//temperature sensor
#define REGMASTER_TEMP7	8 	//temperature sensor
#define REGMASTER_TEMP8	9 	//temperature sensor
#define REGMASTER_TEMP9	10 	//temperature sensor
#define REGMASTER_EXCEPTIONDATA	11 	//LSB: type, MSB: motor|slave
#define NUMREGS_MASTER 12

extern MAYBEPROGMEM Register registerTable_MASTER[];

