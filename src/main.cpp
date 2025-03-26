#include <Arduino.h>
//#include <TensorFlowLite.h>
#include "sin_predictor_int8.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
//#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
//#include "tensorflow/lite/version.h" 



// Allocate a tensor arena
static const int kTensorArenaSize = 2048;
static uint8_t tensor_arena[kTensorArenaSize];


// Pointers to TFLM structures
static tflite::MicroInterpreter* interpreter = nullptr;
static TfLiteTensor* input_tensor = nullptr;
static TfLiteTensor* output_tensor = nullptr;

void initModel() {
  // Get the model struct from the model array
  const tflite::Model* model = tflite::GetModel(sin_predictor_int8_tflite);
  
  // Create an op resolver that has all ops
  static tflite::AllOpsResolver resolver;
  
  // Create the interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;
  
  // Allocate tensors
  interpreter->AllocateTensors();
  
  // Grab pointers to input & output
  input_tensor = interpreter->input(0);
  output_tensor = interpreter->output(0);
}



unsigned long t0;
unsigned long t1;
unsigned long t2;
unsigned long t3;
unsigned long t_print;   // microseconds from t0 to t1
unsigned long t_infer;   // microseconds from t1 to t2








#define INPUT_BUFFER_SIZE 64
#define OUTPUT_BUFFER_SIZE 64
#define INT_ARRAY_SIZE 8

// put function declarations here:
int string_to_array(char *in_str, int *int_array);
void print_int_array(int *int_array, int array_len);
int sum_array(int *int_array, int array_len);
void runSinePredictor(const int* input_array);


char received_char = (char)NULL;              
int chars_avail = 0;                    // input present on terminal
char out_str_buff[OUTPUT_BUFFER_SIZE];  // strings to print to terminal
char in_str_buff[INPUT_BUFFER_SIZE];    // stores input from terminal
int input_array[INT_ARRAY_SIZE];        // array of integers input by user
 
int in_buff_idx=0; // tracks current input location in input buffer
int array_length=0;
int array_sum=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);   //I ADDED THIS!!1
  while(!Serial) { }  
  delay(2000);
  // Arduino does not have a stdout, so printf does not work easily
  // So to print fixed messages (without variables), use 
  // Serial.println() (appends new-line)  or Serial.print() (no added new-line)
  Serial.println("Test Project waking up");
  memset(in_str_buff, (char)0, INPUT_BUFFER_SIZE*sizeof(char)); 

  //call the model initialization function
  initModel();

}

void loop() {
  // put your main code here, to run repeatedly:

  // check if characters are avialble on the terminal input
  chars_avail = Serial.available(); 
  if (chars_avail > 0) {
    received_char = Serial.read(); // get the typed character and 
    Serial.print(received_char);   // echo to the terminal

    in_str_buff[in_buff_idx++] = received_char; // add it to the buffer
    if (received_char == 13) { // 13 decimal = newline character
      // user hit 'enter', so we'll process the line.
      Serial.print("About to process line: ");
      Serial.println(in_str_buff);

      // Process and print out the array
      array_length = string_to_array(in_str_buff, input_array);
      if(array_length == 7){
        

        Serial.println("Correct number of inputs");

        runSinePredictor(input_array);
        //code to process the inputs here
      }
      else {
        Serial.println("Input is incorrect size, please try again."); 
      };




      /*
      sprintf(out_str_buff, "Read in  %d integers: ", array_length);
      Serial.print(out_str_buff);
      print_int_array(input_array, array_length);
      array_sum = sum_array(input_array, array_length);
      sprintf(out_str_buff, "Sums to %d\r\n", array_sum);
      Serial.print(out_str_buff); */

      // Now clear the input buffer and reset the index to 0
      memset(in_str_buff, (char)0, INPUT_BUFFER_SIZE*sizeof(char)); 
      in_buff_idx = 0;
    }
    else if (in_buff_idx >= INPUT_BUFFER_SIZE) {
      memset(in_str_buff, (char)0, INPUT_BUFFER_SIZE*sizeof(char)); 
      in_buff_idx = 0;
    }    
  }
}


void runSinePredictor(const int* input_array) {




  for (int i = 0; i < 7; i++) {
    input_tensor->data.int8[i] = (int8_t)(input_array[i]);
  }

  t0 = micros();

  Serial.println("Test line");

  t1 = micros();

  
  // invoke the model
  TfLiteStatus invoke_status = interpreter->Invoke();

  t2 = micros();

  t_print = t1 - t0;   // microseconds from t0 to t1
  t_infer = t2 - t1;   // microseconds from t1 to t2


  Serial.print("Printing time = ");
  Serial.print(t_print);
  Serial.print(" us.  Inference time = ");
  Serial.print(t_infer);
  Serial.println(" us.");

  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed!");
    return;
  }



  // Read the predicted int8 output
  int8_t predicted_int8 = output_tensor->data.int8[0];
  Serial.print("TFLM predicted next sine value (int8) = ");
  Serial.println(predicted_int8);
}





int string_to_array(char *in_str, int *int_array) {
  int num_integers=0;
  char *token = strtok(in_str, ",");
  
  while (token != NULL) {
    int_array[num_integers++] = atoi(token);
    token = strtok(NULL, ",");
    if (num_integers >= INT_ARRAY_SIZE) {
      break;
    }
  }
  
  return num_integers;
}

void print_int_array(int *int_array, int array_len) {
  int curr_pos = 0; // track where in the output buffer we're writing

  sprintf(out_str_buff, "Integers: [");
  curr_pos = strlen(out_str_buff); // so the next write adds to the end
  for(int i=0;i<array_len;i++) {
    // sprintf returns number of char's written. use it to update current position
    curr_pos += sprintf(out_str_buff+curr_pos, "%d, ", int_array[i]);
  }
  sprintf(out_str_buff+curr_pos, "]\r\n");
  Serial.print(out_str_buff);
}

int sum_array(int *int_array, int array_len) {
  int curr_sum = 0; // running sum of the array

  for(int i=0;i<array_len;i++) {
    curr_sum += int_array[i];
  }
  return curr_sum;
}