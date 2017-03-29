/**
 * \file
 * Code generated automatically by regparse.hs - DO NOT MODIFY.
 */

#include "regs.h"
extern MAYBEPROGMEM Register registerTable_DS[]={
	{65, -1.0, -1.0}, 	// RESET: (0)  reset bits - beware race conditions
	{2, -1.0, -1.0}, 	// TIMER: (1)  millis since start
	{2, -1.0, -1.0}, 	// INTERVALI2C: (2)  interval between I2C ticks
	{2, -1.0, -1.0}, 	// STATUS: (3)  see status flags in regs.h
	{65, -1.0, -1.0}, 	// DEBUGLED: (4)  debugging LEDs, turns on for some time
	{66, -1.0, -1.0}, 	// EXCEPTIONDATA: (5)  LSB: type, MSB: id. Write causes REMOTE exception
	{66, -1.0, -1.0}, 	// DISABLEDEXCEPTIONS: (6)  bitfield of disabled exceptions
	{65, -1.0, -1.0}, 	// PING: (7)  debugging
	{66, -1.0, -1.0}, 	// DEBUG: (8)  debugging
	{66, -4000.0, 4000.0}, 	// DRIVE_REQSPEED: (9)  required speed
	{66, 0.0, 10.0}, 	// DRIVE_PGAIN: (10)  P-gain
	{66, 0.0, 10.0}, 	// DRIVE_IGAIN: (11)  I-gain
	{66, -10.0, 10.0}, 	// DRIVE_DGAIN: (12)  D-gain
	{66, 0.0, 1000.0}, 	// DRIVE_INTEGRALCAP: (13)  integral error cap
	{66, 0.0, 1.0}, 	// DRIVE_INTEGRALDECAY: (14)  integral decay
	{66, 0.0, 1000.0}, 	// DRIVE_OVERCURRENTTHRESH: (15)  overcurrent threshold
	{2, -4000.0, 4000.0}, 	// DRIVE_ACTUALSPEED: (16)  actual speed from encoder
	{2, -1000.0, 1000.0}, 	// DRIVE_ERROR: (17)  required minus actual speed
	{2, -1000.0, 1000.0}, 	// DRIVE_ERRORINTEGRAL: (18)  error integral magnitude
	{2, -200.0, 200.0}, 	// DRIVE_ERRORDERIV: (19)  error derivative
	{2, -255.0, 255.0}, 	// DRIVE_CONTROL: (20)  value being sent to motor
	{2, 0.0, 1000.0}, 	// DRIVE_INTERVALCTRL: (21)  time between control runs (ms)
	{2, -1.0, -1.0}, 	// DRIVE_CURRENT: (22)  raw current reading
	{2, -1.0, -1.0}, 	// DRIVE_ODO: (23)  encoder ticks
	{65, 0.0, 255.0}, 	// DRIVE_STALLCHECK: (24)  stall check control signal level
	{65, 0.0, 50.0}, 	// DRIVE_DEADZONE: (25)  if below this value, error is set to zero
	{66, -200.0, 200.0}, 	// STEER_REQPOS: (26)  required position
	{66, 0.0, 100.0}, 	// STEER_PGAIN: (27)  P-gain
	{66, 0.0, 10.0}, 	// STEER_IGAIN: (28)  I-gain
	{66, -10.0, 10.0}, 	// STEER_DGAIN: (29)  D-gain
	{66, 0.0, 1000.0}, 	// STEER_INTEGRALCAP: (30)  integral error cap
	{66, 0.0, 1.0}, 	// STEER_INTEGRALDECAY: (31)  integral decay
	{66, 0.0, 1000.0}, 	// STEER_OVERCURRENTTHRESH: (32)  overcurrent threshold
	{2, -200.0, 200.0}, 	// STEER_ACTUALPOS: (33)  actual position from pot
	{2, -200.0, 200.0}, 	// STEER_ERROR: (34)  required minus actual position
	{2, -1000.0, 1000.0}, 	// STEER_ERRORINTEGRAL: (35)  error integral magnitude
	{2, -200.0, 200.0}, 	// STEER_ERRORDERIV: (36)  error derivative
	{2, -255.0, 255.0}, 	// STEER_CONTROL: (37)  value being sent to motor
	{2, 0.0, 1000.0}, 	// STEER_INTERVALCTRL: (38)  time between control runs (ms)
	{2, -1.0, -1.0}, 	// STEER_CURRENT: (39)  raw current reading
	{65, 0.0, 255.0}, 	// STEER_STALLCHECK: (40)  stall check control signal level
	{65, 0.0, 50.0}, 	// STEER_DEADZONE: (41)  if below this value, error is set to zero
	{65, -120.0, 120.0}, 	// STEER_CALIBMIN: (42)  minimum angle, mapped onto pot value 0
	{65, -120.0, 120.0}, 	// STEER_CALIBMAX: (43)  maximum angle, mapped onto pot value 1024
	{2, 0.0, 1024.0}, 	// CHASSIS: (44)  chassis pot reading
	{32, -1.0, -1.0}, 	// TERMINATOR: (45)  
};

extern MAYBEPROGMEM Register registerTable_LL[]={
	{65, -1.0, -1.0}, 	// RESET: (0)  reset bits - beware race conditions
	{2, -1.0, -1.0}, 	// TIMER: (1)  millis since start
	{2, -1.0, -1.0}, 	// INTERVALI2C: (2)  interval between I2C ticks
	{2, -1.0, -1.0}, 	// STATUS: (3)  see status flags in regs.h
	{65, -1.0, -1.0}, 	// DEBUGLED: (4)  debugging LEDs, turns on for some time
	{66, -1.0, -1.0}, 	// EXCEPTIONDATA: (5)  LSB: type, MSB: id. Write causes REMOTE exception
	{66, -1.0, -1.0}, 	// DISABLEDEXCEPTIONS: (6)  bitfield of disabled exceptions
	{65, -1.0, -1.0}, 	// PING: (7)  debugging
	{66, -1.0, -1.0}, 	// DEBUG: (8)  debugging
	{66, -200.0, 200.0}, 	// ONE_REQPOS: (9)  required position
	{66, 0.0, 100.0}, 	// ONE_PGAIN: (10)  P-gain
	{66, 0.0, 10.0}, 	// ONE_IGAIN: (11)  I-gain
	{66, -10.0, 10.0}, 	// ONE_DGAIN: (12)  D-gain
	{66, 0.0, 1000.0}, 	// ONE_INTEGRALCAP: (13)  integral error cap
	{66, 0.0, 1.0}, 	// ONE_INTEGRALDECAY: (14)  integral decay
	{66, 0.0, 1000.0}, 	// ONE_OVERCURRENTTHRESH: (15)  overcurrent threshold
	{2, -200.0, 200.0}, 	// ONE_ACTUALPOS: (16)  actual position from pot
	{2, -200.0, 200.0}, 	// ONE_ERROR: (17)  required minus actual position
	{2, -1000.0, 1000.0}, 	// ONE_ERRORINTEGRAL: (18)  error integral magnitude
	{2, -200.0, 200.0}, 	// ONE_ERRORDERIV: (19)  error derivative
	{2, -255.0, 255.0}, 	// ONE_CONTROL: (20)  value being sent to motor
	{2, 0.0, 1000.0}, 	// ONE_INTERVALCTRL: (21)  time between control runs (ms)
	{2, -1.0, -1.0}, 	// ONE_CURRENT: (22)  raw current reading
	{65, -120.0, 120.0}, 	// ONE_CALIBMIN: (23)  minimum angle, mapped onto pot value 0
	{65, -120.0, 120.0}, 	// ONE_CALIBMAX: (24)  maximum angle, mapped onto pot value 1024
	{65, 0.0, 255.0}, 	// ONE_STALLCHECK: (25)  stall check control signal level
	{65, 0.0, 50.0}, 	// ONE_DEADZONE: (26)  if below this value, error is set to zero
	{66, -200.0, 200.0}, 	// TWO_REQPOS: (27)  required position
	{66, 0.0, 100.0}, 	// TWO_PGAIN: (28)  P-gain
	{66, 0.0, 10.0}, 	// TWO_IGAIN: (29)  I-gain
	{66, -10.0, 10.0}, 	// TWO_DGAIN: (30)  D-gain
	{66, 0.0, 1000.0}, 	// TWO_INTEGRALCAP: (31)  integral error cap
	{66, 0.0, 1.0}, 	// TWO_INTEGRALDECAY: (32)  integral decay
	{66, 0.0, 1000.0}, 	// TWO_OVERCURRENTTHRESH: (33)  overcurrent threshold
	{2, -200.0, 200.0}, 	// TWO_ACTUALPOS: (34)  actual position from pot
	{2, -200.0, 200.0}, 	// TWO_ERROR: (35)  required minus actual position
	{2, -1000.0, 1000.0}, 	// TWO_ERRORINTEGRAL: (36)  error integral magnitude
	{2, -200.0, 200.0}, 	// TWO_ERRORDERIV: (37)  error derivative
	{2, -255.0, 255.0}, 	// TWO_CONTROL: (38)  value being sent to motor
	{2, 0.0, 1000.0}, 	// TWO_INTERVALCTRL: (39)  time between control runs (ms)
	{2, -1.0, -1.0}, 	// TWO_CURRENT: (40)  raw current reading
	{65, -120.0, 120.0}, 	// TWO_CALIBMIN: (41)  minimum angle, mapped onto pot value 0
	{65, -120.0, 120.0}, 	// TWO_CALIBMAX: (42)  maximum angle, mapped onto pot value 1024
	{65, 0.0, 255.0}, 	// TWO_STALLCHECK: (43)  stall check control signal level
	{65, 0.0, 50.0}, 	// TWO_DEADZONE: (44)  if below this value, error is set to zero
	{32, -1.0, -1.0}, 	// TERMINATOR: (45)  
};

extern MAYBEPROGMEM Register registerTable_MASTER[]={
	{65, -1.0, -1.0}, 	// RESET: (0)  set to clear exception state
	{2, -20.0, 100.0}, 	// TEMPAMBIENT: (1)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP1: (2)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP2: (3)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP3: (4)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP4: (5)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP5: (6)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP6: (7)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP7: (8)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP8: (9)  temperature sensor
	{2, -20.0, 100.0}, 	// TEMP9: (10)  temperature sensor
	{2, -1.0, -1.0}, 	// EXCEPTIONDATA: (11)  LSB: type, MSB: motor|slave
	{32, -1.0, -1.0}, 	// TERMINATOR: (12)  
};
