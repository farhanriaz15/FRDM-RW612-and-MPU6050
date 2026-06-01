/*
 * RW612 + MPU6050 Debug Test
 */

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "app.h"

#define EXAMPLE_I2C_MASTER ((I2C_Type *)EXAMPLE_I2C_MASTER_BASE)

#define SENSOR_ADDR        (0x68U)

#define REG_WHO_AM_I       (0x75U)
#define REG_PWR_MGMT_1     (0x6BU)
#define REG_ACCEL_XOUT_H   (0x3BU)

#define I2C_BAUDRATE       (50000U)

/*------------------------------------------------------------------*/
static status_t Sensor_WriteRegister(uint8_t reg,
                                     uint8_t value)
{
    i2c_master_transfer_t xfer;

    memset(&xfer, 0, sizeof(xfer));

    xfer.slaveAddress   = SENSOR_ADDR;
    xfer.direction      = kI2C_Write;
    xfer.subaddress     = reg;
    xfer.subaddressSize = 1;
    xfer.data           = &value;
    xfer.dataSize       = 1;
    xfer.flags          = kI2C_TransferDefaultFlag;

    return I2C_MasterTransferBlocking(
                EXAMPLE_I2C_MASTER,
                &xfer);
}

/*------------------------------------------------------------------*/
static status_t Sensor_ReadRegisters(uint8_t reg,
                                     uint8_t *data,
                                     uint8_t length)
{
    i2c_master_transfer_t xfer;

    memset(&xfer, 0, sizeof(xfer));

    xfer.slaveAddress   = SENSOR_ADDR;
    xfer.direction      = kI2C_Read;
    xfer.subaddress     = reg;
    xfer.subaddressSize = 1;
    xfer.data           = data;
    xfer.dataSize       = length;
    xfer.flags          = kI2C_TransferDefaultFlag;

    return I2C_MasterTransferBlocking(
                EXAMPLE_I2C_MASTER,
                &xfer);
}

/*------------------------------------------------------------------*/
int main(void)
{
    i2c_master_config_t masterConfig;

    uint8_t whoAmI;
    uint8_t pwrMgmt;
    uint8_t sensorData[14];

    BOARD_InitHardware();

    PRINTF("\r\n");
    PRINTF("=====================================\r\n");
    PRINTF("RW612 MPU6050 DEBUG TEST\r\n");
    PRINTF("=====================================\r\n");

    I2C_MasterGetDefaultConfig(&masterConfig);

    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    I2C_MasterInit(EXAMPLE_I2C_MASTER,
                   &masterConfig,
                   I2C_MASTER_CLOCK_FREQUENCY);

    PRINTF("I2C Initialized\r\n");

    /* Read WHO_AM_I */
    if (Sensor_ReadRegisters(REG_WHO_AM_I,
                             &whoAmI,
                             1) == kStatus_Success)
    {
        PRINTF("WHO_AM_I = 0x%02X\r\n", whoAmI);
    }
    else
    {
        PRINTF("WHO_AM_I Read Failed\r\n");
    }

    /* Wake sensor */
    if (Sensor_WriteRegister(REG_PWR_MGMT_1,
                             0x00) != kStatus_Success)
    {
        PRINTF("Failed to wake sensor\r\n");

        while (1)
        {
        }
    }

    /* Verify wake-up */
    if (Sensor_ReadRegisters(REG_PWR_MGMT_1,
                             &pwrMgmt,
                             1) == kStatus_Success)
    {
        PRINTF("PWR_MGMT_1 = 0x%02X\r\n", pwrMgmt);
    }
    else
    {
        PRINTF("PWR_MGMT_1 Read Failed\r\n");
    }

    PRINTF("\r\nStarting sensor stream...\r\n");

    while (1)
    {
        status_t status;

        status = Sensor_ReadRegisters(
                    REG_ACCEL_XOUT_H,
                    sensorData,
                    14);

        if (status != kStatus_Success)
        {
            PRINTF("Sensor Read Failed: %d\r\n",
                   status);

            SDK_DelayAtLeastUs(
                500000,
                CLOCK_GetCoreSysClkFreq());

            continue;
        }

        int32_t accelX =
            (int16_t)((sensorData[0] << 8) | sensorData[1]);

        int32_t accelY =
            (int16_t)((sensorData[2] << 8) | sensorData[3]);

        int32_t accelZ =
            (int16_t)((sensorData[4] << 8) | sensorData[5]);

        int32_t tempRaw =
            (int16_t)((sensorData[6] << 8) | sensorData[7]);

        int32_t gyroX =
            (int16_t)((sensorData[8] << 8) | sensorData[9]);

        int32_t gyroY =
            (int16_t)((sensorData[10] << 8) | sensorData[11]);

        int32_t gyroZ =
            (int16_t)((sensorData[12] << 8) | sensorData[13]);

        PRINTF("AX=%d ", accelX);
        PRINTF("AY=%d ", accelY);
        PRINTF("AZ=%d ", accelZ);

        PRINTF("GX=%d ", gyroX);
        PRINTF("GY=%d ", gyroY);
        PRINTF("GZ=%d ", gyroZ);

        PRINTF("TEMP_RAW=%d\r\n", tempRaw);


        SDK_DelayAtLeastUs(
            500000,
            CLOCK_GetCoreSysClkFreq());
    }
}
