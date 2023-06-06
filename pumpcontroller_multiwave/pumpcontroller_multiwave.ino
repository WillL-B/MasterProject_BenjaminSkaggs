const int MEDIA_PIN_1 = 4;
const int WATER_PIN_1 = 3;
/*const int MEDIA_PIN_2 = 6;
const int WATER_PIN_2 = 5;
const int MEDIA_PIN_3 = 8;
const int WATER_PIN_3 = 7;
const int MEDIA_PIN_4 = 10;
const int WATER_PIN_4 = 9;*/

int PIN_PAIRS[1][2] = { { MEDIA_PIN_1, WATER_PIN_1 } };  //, { MEDIA_PIN_2, WATER_PIN_2 }, { MEDIA_PIN_3, WATER_PIN_3 }, { MEDIA_PIN_4, WATER_PIN_4 } };
const int PIN_PAIRS_ENABLED = 1;                         // # of pins to enable, will select from index 0-3 first-last

const int TIME_PER_CYCLE = 1000;  // time in milliseconds for each cycle, cannot be 0
//const int WAVE_SEGMENT_TIMES[][] = { {1,2}, [1,1], [1,1] };      // times to run each segment of each experiment, needs to be 2D with inner array with elements = # of different tasks to run - or manually set in fucntion call
const int NUMBER_OF_WAVES = 1;                                     // number of waves to make, cannot be 0, ** TEST with 1 wave **
                                                                   //
const float MEDIA_CONCENTRATION = 1.0;                             // concentration of media, cannot be 0
float initial_concentration[NUMBER_OF_WAVES] = { 0.1 };  // list of starting output concentrations per wave
float final_concentration[NUMBER_OF_WAVES] = { 1.0 };    // list of final desired output concentrations < MEDIA_CONCENTRATION
float current_output_concentration = 0.0;                          // current output concentration

const int STEPS = 2;                            // number of steps (gap to elevated only)
const int STEP_LENGTH = 1;                      // step length in minutes
const int STEP_GAP = 1;                         // gap between steps in minutes
const float STEP_ELEVATED_CONCENTRATION = 0.5;  // concentration during step
const float STEP_BASELINE_CONCENTRATION = 0.1;  // concentration during gap between steps
int current_step = 0;                           //

int total_cycles = 0;    // total time for all waves in cycles
int current_cycles = 0;  // total time for this wave in cycles

void setup() {

  for (int i = 0; i < PIN_PAIRS_ENABLED; i++) {  // set all pins to output mode
    pinMode(PIN_PAIRS[i][0], OUTPUT);
    pinMode(PIN_PAIRS[i][1], OUTPUT);

    digitalWrite(PIN_PAIRS[i][0], LOW);  // set all pins to low voltage (valves closed)
    digitalWrite(PIN_PAIRS[i][1], LOW);

    current_output_concentration = initial_concentration[i];
  }

  pinMode(LED_BUILTIN, OUTPUT);

  //Serial.begin(9600);
}

void loop() {

  for (byte wave = 0; wave < NUMBER_OF_WAVES; wave++) {  // advances WAVES

    // ** pulse functions here **

    //ramp(int pins[], int duration, float initial_conc, float final_conc)
    ramp(PIN_PAIRS[0], 1, initial_concentration[wave], final_concentration[wave]);

    //wait(int pins[], int duration, float set_conc)
    wait(PIN_PAIRS[0], 1, current_output_concentration);

    //step(int pins[], int interval, int gap, int steps, float baseline_conc, float elevated_conc)
    //step(PIN_PAIRS[0], STEP_LENGTH, STEP_GAP, STEPS, STEP_BASELINE_CONCENTRATION, STEP_ELEVATED_CONCENTRATION);

    // ** end of wave logic here **
    current_cycles = 0;
  }

  // ** end of ALL waves here **
  exit(0);  // exit main loop
}


// a singular pulse with a duration
void pulse(int pins[], float fraction_duty) {  // accepts pin pair and intended duty cycle %, generates (count em,) 1 pulse

  if (fraction_duty >= 1) {  // no turn off media, only on
    digitalWrite(pins[0], HIGH);
    digitalWrite(pins[1], LOW);
    delay(TIME_PER_CYCLE / PIN_PAIRS_ENABLED);

  } else {
    digitalWrite(pins[0], HIGH);
    digitalWrite(pins[1], LOW);
    delay((long)(TIME_PER_CYCLE * fraction_duty / PIN_PAIRS_ENABLED));  // delays media's % portion of 1000ms duty cycle, truncated to integer by type conversion
    digitalWrite(pins[0], LOW);
    digitalWrite(pins[1], HIGH);
    delay((long)(TIME_PER_CYCLE * (1 - fraction_duty) / PIN_PAIRS_ENABLED));  // delays by water's % portion (inverse of media) of 1000ms duty cycle, truncated to integer
  }

  if (fraction_duty == 0) {  // if no media intended, put no media duh
    digitalWrite(pins[0], LOW);
    digitalWrite(pins[1], HIGH);
    delay(TIME_PER_CYCLE);
  }
  //Serial.println(current_cycles);
  //Serial.println(fraction_duty);
  current_cycles++;
  total_cycles++;
}

void wait(int pins[], int duration, float set_conc) {  // duration in minutes LINEAR
  for (int i = 0; i <= ctime(duration); i++) {         // for x cycles
    current_output_concentration = set_conc;
    float fraction_duty = current_output_concentration / MEDIA_CONCENTRATION;
    pulse(pins, fraction_duty);
  }
}

// performs calculations for ramp rate, can add *flavor* with specific equations (e.g., conc=(final-initial)/experimenttime * currentx + initial)
void ramp(int pins[], int duration, float initial_conc, float final_conc) {  // duration in minutes, optionally add String for type of curve
  for (int i = 0; i <= ctime(duration); i++) {

    current_output_concentration = ((final_conc - initial_conc) / (ctime(duration)) * i + initial_conc);  // linear, conditional for string line type
    float fraction_duty = current_output_concentration / MEDIA_CONCENTRATION;                             // calculates duty cycle for
    pulse(pins, fraction_duty);
  }
}

void step(int pins[], int interval, int gap, int steps, float baseline_conc, float elevated_conc) {  // this will trigger wait at x elevated and baseline concs LINEAR
  for (int i = 1; i <= steps; i++) {                                                                 // duration in minutes
    wait(pins, gap, baseline_conc);                                                                  // baseline wait
    wait(pins, interval, elevated_conc);                                                             // elevated wait
  }
}

int ctime(int time_minutes) {  // converts minutes to cycles
  return (time_minutes * 60000 / TIME_PER_CYCLE);
}
