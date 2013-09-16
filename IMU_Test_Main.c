/* 
 * File:   IMU_Test_Main.c
 * Author: Mitch
 *
 * Created on June 15, 2013, 3:40 PM
 */ 

#define DEBUG 1 //Debug Mode = 1
#define STABILIZATION 0 //Stabilization Mode = 1

//Include Libraries
#include <stdio.h>
#include <stdlib.h>
#include <p33FJ256GP710.h>
#include <string.h>

//Include the Full Initialization Header File
#include "Clock.h"
#include "FullInitialize.h"
#include "delay.h"
#include "VN100.h"
#include "InputCapture.h"
#include "OutputCompare.h"

//For Testing/Debugging:
#if DEBUG
#include "UART1.h"
#endif
//Perform random clock things that came with the Sample Code (Keep in code)
_FOSCSEL(FNOSC_FRC);			// Internal FRC oscillator
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF  & POSCMD_NONE);
								// Clock Switching is enabled and Fail Safe Clock Monitor is disabled
								// OSC2 Pin Function: OSC2 is Clock Output
								// Primary Oscillator Mode: XT Crystanl

//Define variables for global use in the code
#define TRUE	1
#define FALSE	0





 /*****************************************************************************
 *****************************************************************************

                             STABILIZATION CODE

 *****************************************************************************
 *****************************************************************************/

//Servo scale factor is used in converting deg/s rotation input into output compare values
float SERVO_SCALE_FACTOR;

int controlSignal(float setpoint, float output, float gain) {
    int control = SERVO_SCALE_FACTOR*(setpoint-output)*gain + MIDDLE_PWM;
    return control;
}


/*****************************************************************************
 *****************************************************************************
 ******************************************************************************/

int main()
{
    //Debug Mode:
    if (DEBUG){
        InitUART1();
    }

    if(RCONbits.TRAPR == 1){
        UART1_SendString("TRAP Reset Occurred");
        RCONbits.TRAPR = 0;
    }

        if(RCONbits.IOPUWR == 1){
        UART1_SendString("Illegal Opcode Reset Occurred");
        RCONbits.IOPUWR = 0;
    }

        if(RCONbits.VREGS == 1){
        UART1_SendString("Voltage Reg Reset Occurred");
         RCONbits.VREGS = 0;
    }

        if(RCONbits.EXTR == 1){
        UART1_SendString("External Reset Occurred");
         RCONbits.EXTR = 0;
    }

        if(RCONbits.SWR == 1){
        UART1_SendString("Software Reset Occurred");
        RCONbits.SWR = 0;
    }

        if(RCONbits.WDTO == 1){
        UART1_SendString("Software WDT Reset Occurred");
        RCONbits.WDTO = 0;
    }

            if(RCONbits.SLEEP == 1){
        UART1_SendString("Sleep Mode Reset Occurred");
        RCONbits.SLEEP = 0;
    }

            if(RCONbits.IDLE == 1){
        UART1_SendString("Idle Mode Reset Occurred");
        RCONbits.IDLE = 0;
    }

            if(RCONbits.BOR == 1){
        UART1_SendString("Brown Out Reset Occurred");
        RCONbits.BOR = 0;
    }

            if(RCONbits.POR == 1){
        UART1_SendString("Power On Reset Occurred");
        RCONbits.POR = 0;
    }

   SERVO_SCALE_FACTOR = -(UPPER_PWM-MIDDLE_PWM)/45;

    // Setpoints (From radio transmitter or autopilot)
    int sp_RollRate;
    int sp_PitchRate;
    int sp_ThrottleRate;
    int sp_YawRate;
    

    // System outputs (get from IMU)
    float imu_RollRate;
    float imu_PitchRate;
    float imu_ThrottleRate;
    float imu_YawRate;

    // Derivative Gains (Set for desired pwm servo pulse width)
    float kd_Roll;
    float kd_Pitch;
    float kd_Throttle;
    float kd_Yaw;

    // Control Signals (Output compare value)
    int control_Roll;
    int control_Pitch;
    int control_Throttle;
    int control_Yaw;

    initIC(0b1111);
    initOC(0b1111);//Initialize only Output Compare 1,2,3 and 4
    
    VN100_initSPI();

    if (DEBUG){
        UART1_SendString("START OF CODE BEFORE WHILE");
    }

    while(1)
    {
        if (DEBUG){
           //UART1_SendString("Hi My Name is Mitch");
            //UART1_SendString("Hi My Name is CHRIS");
        }
 /*****************************************************************************
 *****************************************************************************

                             INPUT CAPTURE

 *****************************************************************************
 *****************************************************************************/
        
    int* icTimeDiff;
    icTimeDiff = getICValues();

    sp_RollRate = (icTimeDiff[0] - MIDDLE_PWM);
    sp_PitchRate = (icTimeDiff[1] - MIDDLE_PWM);
    sp_ThrottleRate = (icTimeDiff[2]);
    sp_YawRate = (icTimeDiff[3] - MIDDLE_PWM);

    if (DEBUG){
        //Mock Data
//        sp_RollRate = (float)(0x000);//0x171;//0x02E3 = 1.5us;
//        sp_PitchRate = (float)(0x000);
//        sp_ThrottleRate = (float)(0x000);
//        sp_YawRate = (float)(0x000);
        UART1_SendString("Input Rates");
        char str[20];
        sprintf(str, "%i", sp_RollRate);
        UART1_SendString(str);
                    sprintf(str, "%i", sp_PitchRate);
        UART1_SendString(str);
                    sprintf(str, "%i", sp_YawRate);
        UART1_SendString(str);

    }

 /*****************************************************************************
 *****************************************************************************

                             IMU COMMUNICATION

 *****************************************************************************
 *****************************************************************************/
    if (DEBUG){
       // UART1_SendString("Test 1");
        //U1TXREG = imu_RollRate;
    }

    float rates[3];
    VN100_SPI_GetRates(0,&rates);
    //Outputs in order: Roll,Pitch,Yaw
    imu_RollRate = rates[0];
    imu_PitchRate = rates[1];
    imu_YawRate = rates[2];

    if (DEBUG){
        //float YPR[3];
       // VN100_SPI_GetYPR(0,&YPR[0],&YPR[1],&YPR[2]);
        UART1_SendString("Vector Nav Rates");
        char str[20];
        sprintf(str, "%f", rates[0]);
        UART1_SendString(str);
                    sprintf(str, "%f", rates[1]);
        UART1_SendString(str);
                    sprintf(str, "%f", rates[2]);
        UART1_SendString(str);
//        UART1_SendStringNum("Rate[0]", rates[0]);
//            U1TXREG = YPR[0];
//            while(U1STAbits.TRMT == 0);
//            U1STAbits.TRMT = 0;
    }
/*****************************************************************************
 *****************************************************************************

                             CONTROL CODE

 *****************************************************************************
 *****************************************************************************/
    
    if (!STABILIZATION){
        control_Roll = sp_RollRate + MIDDLE_PWM;
        control_Pitch = sp_PitchRate + MIDDLE_PWM;
        control_Yaw = sp_YawRate + MIDDLE_PWM;
        control_Throttle = sp_ThrottleRate;
    }
    else{
        kd_Roll = 1;
        kd_Pitch = 1;
        kd_Yaw = 1;

    // Control Signals (Output compare value)
    control_Roll = controlSignal(sp_RollRate/30, imu_RollRate, kd_Roll);
    control_Pitch = controlSignal(sp_PitchRate/30, imu_PitchRate, kd_Pitch);
    control_Yaw = controlSignal(sp_YawRate/30, imu_YawRate, kd_Yaw);
    control_Throttle = sp_ThrottleRate;
    }
 /*****************************************************************************
 *****************************************************************************

                             OUTPUT COMPARE

 *****************************************************************************
 *****************************************************************************/
    if (DEBUG){
        //Mock Data
//        control_Roll = 0x02E3;//0x171;//0x02E3 = 1.5us;
//        control_Pitch = 0x02E3;
//        control_Throttle = 0x000;
//        control_Yaw = 0x02E3;

        UART1_SendString("Output Rates");
        char str[20];
        sprintf(str, "%u", control_Roll);
        UART1_SendString(str);
                    sprintf(str, "%u", control_Pitch);
        UART1_SendString(str);
                    sprintf(str, "%u", control_Yaw);
        UART1_SendString(str);
    }

    //Double check ocPin
    setPWM(1,control_Roll);
    setPWM(2,control_Pitch);
    setPWM(3,control_Throttle);
    setPWM(4,control_Yaw);
    }
}
