#include "mbed.h"

#include "ADXL345.h"
#include "Hx711.h"
#include "Sparkfun_TB6612.h"
#include <cstddef>

#include "Dht11.h"

/*

DC-Motor driver:
Sparkfun_TB6612  library: http://os.mbed.com/users/ateyercheese/code/Sparkfun_TB6612/

Accelerometer:
ADXL45 library: http://os.mbed.com/users/aberk/code/ADXL345/

Weight Scale ADC:
HX711 library: http://os.mbed.com/users/megrootens/code/HX711/

Temperature and Humidity sensor:
DHT11 library (modified): https://os.mbed.com/users/fossum_13/code/DHT11/
*/

/* 
THREAD 1: RECEIVE DATA FROM WEIGHT SCALE
    - constantly read values from the weight scale (using I2C)
    - send them to uart

THREAD 2: RECEIVE DATA FROM ACCELEROMETER
    - constantly read values from the sensor (using SPI)
    - send them to uart

MAIN THREAD: CONTROL DC-MOTOR
    - wait for data from uart
    - check if data has correct format
    - rotate DC motor for specified angle
    - send to uart new angle (current orhesis position)
*/


/*
Accelerometer connection to MCU (PINOUT)

MCU (STM32F401RE)        ACCELEROMETER (ADXL345)
    D13 (SCK)       ===     SCL
    D12 (MISO)      ===     SDO
    D11 (MOSI)      ===     SDA
    D10 (CS)        ===     CS

    3.3V            ===     VCC
    GND             ===     GND
*/

// Accelerometer input pins on the MCU
#define ACC_MOSI D11
#define ACC_MISO D12
#define ACC_SCL D13
#define ACC_CS D10

// Accelerometer sample collection frequency
#define ACC_SLEEP_TIME 100ms

// Weigh scale input pins on the MCU
#define HX_SCL D15
#define HX_SDA D14

// Weight Scale sample collection frequency
#define HX_SLEEP_TIME 100ms

// Main thread sleep time in milliseconds
#define MAIN_SLEEP_TIME 100ms

// Temperature
#define DHT_SDI PA_0
#define DHT_SLEEP_RATE 100ms


ADXL345 accelerometer(ACC_MOSI, ACC_MISO, ACC_SCL, ACC_CS);
Thread accelerometer_thread;

// Function for accelerometer thread
void accelerometer_read_data(Kernel::Clock::time_point *mcu_start_time)
{
    // Initiate accelerometer parameters
    accelerometer.setPowerControl(0x00);
    accelerometer.setDataFormatControl(0x0B);
    accelerometer.setDataRate(ADXL345_3200HZ);
    accelerometer.setPowerControl(0x08);
    
    // printf("Activitytreshold: %d",accelerometer.getActivityThreshold());

    // Start reading values constantly and sending them to uart
    int acc_values[3] = {0,0,0};
    while(true)
    {     
        accelerometer.getOutput(acc_values);
        printf("X;%d;%i;%i;%i\n", int((Kernel::Clock::now() - *mcu_start_time).count()), (int16_t)acc_values[0], (int16_t)acc_values[1], (int16_t)acc_values[2]);
        ThisThread::sleep_for(ACC_SLEEP_TIME);
    }
}


/*
ADC (fow weigh scales) connection to MCU (PINOUT)

MCU (STM32F401RE)        ADC (XFW-HX711)
    D14 (SDA)       ===     DT
    D15 (SCL)       ===     SCK
    
    3.3V            ===     VCC
    GND             ===     GND
*/


// Function for weight scales thread
Hx711 weight_sensor(HX_SCL, HX_SDA);
Thread weight_sensor_thread;

void weight_sensor_read_data(Kernel::Clock::time_point *mcu_start_time)
{
    // Initiate accelerometer parameters
    while(!weight_sensor.is_ready())
    {
        continue;
    }

    // Start reading values constantly and sending them to uart
    while(true)
    {     
        printf("W;%d;%f\n", int((Kernel::Clock::now() - *mcu_start_time).count()), weight_sensor.read());
        ThisThread::sleep_for(HX_SLEEP_TIME);
    }
}

// ==== tr

Dht11 sensor(DHT_SDI);
Thread temperature_thread;

void temperature_read(Kernel::Clock::time_point *mcu_start_time)
{
    while(true)
    {
        sensor.read();
        printf("T: %d, H: %d\n:", sensor.getCelsius(), sensor.getHumidity());
        ThisThread::sleep_for(DHT_SLEEP_RATE);

    }
}

#define MAXIMUM_BUFFER_SIZE                                                  32
UnbufferedSerial pc(USBTX, USBRX, 9600); // tx, rx, baud rate

void read_line(char *buffer, size_t length)
{
        int index = 0;
        char c;

        // Read characters until '\n' is received
        while (pc.read(&c, 1)) {
            if (c == '\n') {
                buffer[index] = '\0'; // Null-terminate the string
                break;
            } else if (index < sizeof(buffer) - 1) {
                buffer[index++] = c; // Add character to buffer
            }
        }
}

int read_write_serial() {
    pc.write("Waiting for an integer input...\n", 33);

    char buffer[32]; // Buffer to hold input
    int number;

    
    bool use_DC_motor = false;
    if (!use_DC_motor) {
        
    }
    auto start = Kernel::Clock::now();

    // Start threads for data collection
    accelerometer_thread.start(callback(accelerometer_read_data, &start));
    weight_sensor_thread.start(callback(weight_sensor_read_data, &start));

    while (true) {

        read_line(buffer, sizeof(buffer));
        //int num = atoi(buffer);
        //pc.write("I READ YOUR NUMBER %d", num);

        // Attempt to parse the integer
        if (sscanf(buffer, "%d", &number) == 1) {
            // Trigger action based on received integer
            if (number == 0) {
                pc.write("Received 0. Turning LED off.\n", 30);
            } else {
                pc.write("Received non-zero integer. Toggling LED.\n", 41);
            }
        } else {
            pc.write("Invalid input. Please enter a valid integer.\n", 44);
        }
    }
}

AnalogIn Ain(PA_2);
// ==== tr


int main() {
    auto start = Kernel::Clock::now();

    // Start threads for data collection
    accelerometer_thread.start(callback(accelerometer_read_data, &start));
    weight_sensor_thread.start(callback(weight_sensor_read_data, &start));

    // === tr
    temperature_thread.start(callback(temperature_read, &start));
    // === tr

    while (true) {

        // === tr
        printf("A;%d;%f\n", int((Kernel::Clock::now() - start).count()), Ain.read());
        ThisThread::sleep_for(MAIN_SLEEP_TIME);
        // === tr
    }
}
