#include "mbed.h"

#include "ADXL345.h"
#include "Hx711.h"
//#include "Dht11.h"

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
        printf("tXYZ;%d;%i;%i;%i\n", int((Kernel::Clock::now() - *mcu_start_time).count()), (int16_t)acc_values[0], (int16_t)acc_values[1], (int16_t)acc_values[2]);
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
        printf("tW;%d;%f\n", int((Kernel::Clock::now() - *mcu_start_time).count()), weight_sensor.read());
        ThisThread::sleep_for(HX_SLEEP_TIME);
    }
}

// Blinking rate in milliseconds
#define BLINKING_RATE 200ms

// A5 analog pin on board
AnalogIn Ain(PC_0);
//DigitalOut led(LED1);

int main()
{
    auto start = Kernel::Clock::now();

    accelerometer_thread.start(callback(accelerometer_read_data, &start));
    weight_sensor_thread.start(callback(weight_sensor_read_data, &start));

    while (true)
    {
        printf("tMain;%d;%f\n", int((Kernel::Clock::now() - start).count()), Ain.read());
        ThisThread::sleep_for(BLINKING_RATE);
    }
}

/*
Dht11 sensor(PA_0);
Thread thread1;

void accelerometer_thread()
{
    while(true)
    {
        //led = !led;
        //ThisThread::sleep_for(SLEEP_RATE);
        sensor.read();
        printf("T: %d, H: %d\n:", sensor.getCelsius(), sensor.getHumidity());
        ThisThread::sleep_for(SLEEP_RATE);

    }
}
*/
