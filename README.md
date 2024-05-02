# LAE Brushless DC and Step-Motor Control System
-----------------------------------------------

## Uploading

For building and uploading the code, refer to the [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/index.html).

## Configuration

Most functionalities can be configured on **main/config.h**, such as:
- Turning closed-loop control on and off for both brushless dc and step-motor
- GPIOs
- Commands
- PWM levels
- Sensor parameters
- Motor parameters
- Bluetooth device name
  
## Commands

### Brushless DC motor:

> RGET
    - **Prints the currently measured RPM values**

> PGET
    - **Prints the current PWM values**

> PSET  \<pwm_value\>   \<motor_number\>
    - **Sets the PWM levels on motor \<motor_number\> to \<pwm_value\>**

> RSET \<rpm_value\> \<motor_number\> 
    - **Sets the desired RPM values on motor \<motor_number\> to \<rpm_value\>**

> ALLPSET \<pwm_value\>
    - **Sets the PWM levels on all motors to \<pwm_value\>**

> ALLRSET \<rpm_value\>
    - **Sets the desired RPM values on all motors to \<rpm_value\>**

> X
    - **Shuts down all motors**

> CLON
    - **Turns on the closed-loop control**

> CLOFF
    - **Turns on the closed-loop control**
    
    
### Step-Motor

> ARM
    - **Enables or re-enables the step-motor actuation**

> DISARM
    - **Disables the step-motor actuation**

> ZEROSET
    - **Set current angle as the new zero value**

> BPANGMAX
    - **Bypasses the safety measurent that disarms the system before reaching the max angle configured on main/config.h**

> ANGGET
    - **Prints current angle**

> ANGSET \<angle_value\>
    - **Sets the desired angle to \<ang_value\>**

> CTMDGET
    - **Prints main control variables**
