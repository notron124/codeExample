#ifndef ASYNC_ENGINE_H_
#define ASYNC_ENGINE_H_

/* Size of the sine values table */
#define SINE_TABLE_SIZE    360UL

/* Determines slope of the U/F characteristic.
* The bigger - the steeper slope is.
*/
#define VOLTAGE_CONSTANT   6UL

/* Configures acceleration.
* There is also a coefficient, this constant have to be more than zero.
*/
#define ACCELERATION_CONSTANT 2UL

/* Configures deceleration.
* There is also a coefficient, this constant have to be more than zero.
*/
#define DECELERATION_CONSTANT 100UL

/* Desired PWM frequency, used in further calculations */
#define PWM_FREQUENCY         12000

/* Length of array that used for averaging ADC data from potentiometer*/
#define SPEED_ARR_LENGTH         8
#define TEMPERATURE_ARR_LENGTH   8

/* Delay (in seconds) before engine starts working again after overcurrent. */
#define OVERCURRENT_DELAY     5

/* Threshholds for temperature protection */
#define TRIGGER_OVERHEAT_TEMPERATURE 3102 // 80 C
#define RELEASE_OVERHEAT_TEMPERATURE 2792 // 70 C

//#define TRIGGER_OVERHEAT_TEMPERATURE 1724 //36 C  //3437 // 90 C
//#define RELEASE_OVERHEAT_TEMPERATURE 1488 //30 C  //3102 // 80 C

#define ADC_TO_DMA_BUFF_SIZE     2

volatile uint8_t flags;
#define F_OVERCURRENT   0x01
#define F_OVERHEAT      0x02
#define F_25HZ          0x04
#define F_OVERCURRENT_PROTECT		0x08

#define SET_OVERCURRENT    flags |= F_OVERCURRENT
#define RESET_OVERCURRENT  flags &= ~F_OVERCURRENT
#define OVERCURRENT        (flags & F_OVERCURRENT)

#define SET_OVERHEAT       flags |= F_OVERHEAT
#define RESET_OVERHEAT     flags &= ~F_OVERHEAT
#define OVERHEAT           (flags & F_OVERHEAT)

#define SET_25HZ           flags |= F_25HZ
#define RESET_25HZ         flags &= ~F_25HZ
#define _25HZ              (flags & F_25HZ)

/* Holds speed ADC data for averaging later */
volatile int speed[SPEED_ARR_LENGTH];

/* Holds temperature ADC data for averaging later */
volatile int temperature[TEMPERATURE_ARR_LENGTH];

/* Holds average ADC data for potentiometer*/
volatile int averageADCSpeed;
volatile int averageADCTemperature;

uint8_t cntrSpeed;

uint8_t cntrTemperature;

/* Form polir optimum */
volatile uint8_t optimum_mode;

/* Value of timer autoreload register, calculated on init */
volatile uint32_t timerARR;

/* Current "Time". Actually just a steps counter.*/
volatile uint32_t relativeTime;

/* Pointer that points to current value, that determines current PWM duty cycle.
 * Changes depending on current frequency, current step, and change speed coefficient (look below).
 */
volatile uint32_t pointer;

/* Determines how fast PWM duty cycle changes.
 * Bigger coefficient - bigger output sine frequency, less resolution.
 */
volatile uint32_t pointerChangeSpeedCoef;

/* Delay before current_speed can be changed.
 * Used for smooth (de)acceleration.
 */
volatile uint32_t delay;

/* Determines acceleration.
 * Bigger divider - faster acceleration.
 */
volatile uint32_t rumpUpDivider;

/* Determines deceleration.
 * Bigger coefficient - slower deceleration.
 */
volatile uint32_t slowDownCoeff;

/* Determines overall (de)acceleration.
 * Bigger coefficient - faster (de)acceleration.
 */
volatile uint32_t globalSpeedChangeCoeff;

/* Just a counter for overcurrent delay */
volatile uint32_t overcurrent_delay;

/* Working mode. Depends on jumper that pulls pin to the ground or power.
 * 0 - work from potentiometer;
 * 1 - work from toggle switch.
 */
uint8_t working_mode;

/* Desired frequency - the frequency that current frequency is striving to*/
volatile int32_t desired_frequency;

/* Current sine output frequency, changes
 * only on uprising slope of sine wave to avoid wave breaking
 */
volatile int32_t current_frequency;

/* Delay before motor is turned on/off */
uint32_t cntr_motor_on, cntr_motor_off;

/* DMA transfers data from ADC->DR to this array */
uint32_t adcToDma[ADC_TO_DMA_BUFF_SIZE];

uint32_t absolut(int32_t a);
void CalculateAverageADCValue(volatile int *values, int arraySize, volatile int *averageValueHolder);
void CalculateDesiredFrequency(void);
void OverheatProtection(void);
void StartEngine(void);

#endif
