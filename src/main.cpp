#include <Arduino.h>

// board 'DO IT DEVKIT ESP32 V1'



#define dac 25 // 8bit dac of esp32
#define adc 32 // adc1 of esp32

#define measurement_average 100

#define slow_debug // slows program with 1 sec delay
#define delay 10
//#define plot_to_serial // uncomment this to see debug serial
#define interpolate
#define invert_lut

#define print_formatted

void setup() {
  // put your setup code here, to run once:
  Serial.begin(500000); // reduce the baud rate if there's a connectivity problem
}

float sum_of_readings[257];
float calibration_lookup_table[20481];
float LUT[4096];
int LUT_index = 0;
int counter = 0;

void loop() {

  // odd while "for loop" but hey it works
  // populate 4096 array with sparse 255 data 1 for every 16 
  Serial.print("sampling DAC from ADC ");
  Serial.print(counter % measurement_average);
  Serial.print(" of ");
  Serial.println(measurement_average);

  for( int array_8bit = 0; array_8bit <= 255; array_8bit++){
    
    // DAC generated signal => ADC read signal  
    dacWrite(dac, array_8bit);
    int adc_readings = analogRead(adc);

    // store sparse data into array
    sum_of_readings[array_8bit] = adc_readings + sum_of_readings[array_8bit];
   

    // debug plotter 
    #ifdef plot_to_serial
      Serial.print(array_8bit * 16);
      Serial.print(',');
      Serial.println(sum_of_readings[array_8bit] / counter); 
    #endif
    }
    sum_of_readings[256] =  sum_of_readings[255];
    // array_8bit = 0;
    counter++; // accumulate counter for averaging noise



    #ifdef interpolate
      // interpolation for "missing" data
      if (counter % measurement_average == 0){
        Serial.println("interpolating data");

        for(int array_element = 0; array_element <= 255; array_element++){

          for(int interpolation = 0; interpolation <= 79; interpolation++){
            
            // interpolate using simple average proportion between 2 points
            calibration_lookup_table[array_element * 80 + interpolation] = \
              (sum_of_readings[array_element] / counter) + \
              (((sum_of_readings[array_element + 1] / counter) -\
              (sum_of_readings[array_element] / counter)) / 80 * interpolation);

            // debugging stuff ignore it
            #ifdef interpolate_debug
              Serial.print(sum_of_readings[array_element + 1] / counter);
              Serial.print(',');
              Serial.print(array_element);
              Serial.print(',');
              Serial.print(array_element + 1);
              Serial.print(',');
              Serial.print(sum_of_readings[array_element] / counter);
              Serial.print(',');
              Serial.println(calibration_lookup_table[array_element * 80 + interpolation]);
            #endif

          }
        }

        #ifdef invert_lut
          // inverting arrays as index and index as arrays
          Serial.println("inverting lut");
          //int index =0;
          for(int index = 0; index <= 20480; index++){

            if (calibration_lookup_table[index] >= LUT_index){

              LUT[LUT_index] = (float)index / 5;

              // debugging stuff ignore it
              #ifdef invert_lut_debug 
                Serial.print(calibration_lookup_table[index]);
                Serial.print(',');
                Serial.print(index);
                Serial.print(',');
                Serial.print((float)index / 5);
                Serial.print(',');
                Serial.print(LUT[LUT_index]);
                Serial.print(',');
                Serial.println(LUT_index);
              #endif

              LUT_index++;
            }
            
          }

          LUT_index = 0;

        #endif
      
        // formatting stuff to serial output
        #ifdef print_formatted
          Serial.println("copy and paste this to your project");
          Serial.println();
          Serial.println("const float adc_lookup_table[4096] = {");
          for(int i = 0; i < 4096; i++){

            Serial.print(LUT[i]);
            
            if(i != 4095){
              Serial.print(',');
            }
            
            if((i+1) % 16 == 0){
              Serial.println();
            }
          }
          
          Serial.println("};");
          Serial.println();
        #endif

        #ifdef slow_debug
          Serial.print("recalibration in ");
          Serial.print(delay);
          Serial.println(" second(s)");
          sleep(delay); 
        #endif

        //flush array and reset counter
        memset(sum_of_readings, 0, sizeof(sum_of_readings));
        counter = 0;


      }
      
    #endif



}