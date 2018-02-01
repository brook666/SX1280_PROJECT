//-----------------------------------------------------------------------------
// SI446X_B0_defs.h
//-----------------------------------------------------------------------------
// Copyright 2011 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Created by:
//
//  Silicon Laboratiories Hungary
//  Attila Gosi
//
// Program Description:
//  Si446x B0 family specific definitions (constants, commands, properties, types)
//
// Release 2.0
//

//-----------------------------------------------------------------------------
// Header File Preprocessor Directive
//-----------------------------------------------------------------------------

#ifndef SI446X_B0_DEFS_H
#define SI446X_B0_DEFS_H

#include <stdint.h>

#define MAX_CTS_RETRY         2500
#define NIRQ_TIMEOUT          10000

//+++++++++++++++++++++
//  Command constants
//+++++++++++++++++++++

//Boot commands
#define CMD_POWER_UP          0x02  
#define CMD_PATCH_IMAGE         0x04  

//Common commands
#define CMD_NOP             0x00
#define CMD_PART_INFO         0x01
#define CMD_FUNC_INFO         0x10
#define CMD_SET_PROPERTY        0x11
#define CMD_GET_PROPERTY        0x12
#define CMD_GPIO_PIN_CFG        0x13
#define CMD_FIFO_INFO         0x15
#define CMD_IRCAL           0x17
#define CMD_GET_INT_STATUS        0x20
#define CMD_REQUEST_DEVICE_STATE    0x33
#define CMD_CHANGE_STATE        0x34
#define CMD_GET_ADC_READING       0x14
#define CMD_GET_PACKET_INFO       0x16
#define CMD_PROTOCOL_CFG        0x18
#define CMD_GET_PH_STATUS       0x21
#define CMD_GET_MODEM_STATUS      0x22
#define CMD_GET_CHIP_STATUS       0x23
#define CMD_RX_HOP            0x36   

//Tx commands
#define CMD_START_TX          0x31
#define CMD_TX_FIFO_WRITE       0x66

//Rx commands
#define CMD_START_RX          0x32
#define CMD_RX_FIFO_READ        0x77

//CTS command
#define CMD_CTS_READ          0x44

//Fast response register commands
#define CMD_FAST_RESPONSE_REG_A     0x50
#define CMD_FAST_RESPONSE_REG_B     0x51
#define CMD_FAST_RESPONSE_REG_C     0x53
#define CMD_FAST_RESPONSE_REG_D     0x57


//+++++++++++++++++++++
//Property constants
//+++++++++++++++++++++

#define PROP_INT_CTL_GROUP            0x01
#define PROP_INT_CTL_ENABLE           0x00
#define PROP_INT_CTL_PH_ENABLE          0x01
#define PROP_INT_CTL_MODEM_ENABLE       0x02
#define PROP_INT_CTL_CHIP_ENABLE        0x03

#define PROP_FRR_CTL_GROUP            0x02
#define PROP_FRR_CTL_A_MODE           0x00
#define PROP_FRR_CTL_B_MODE           0x01
#define PROP_FRR_CTL_C_MODE           0x02
#define PROP_FRR_CTL_D_MODE           0x03

#define PROP_SYNC_BITS_GROUP          0x11
#define PROP_SYNC_BITS_31_24          0x01
#define PROP_SYNC_BITS_23_16          0x02

#define PROP_PA_GROUP             0x22
#define PROP_PA_MODE              0x00
#define PROP_PA_PWR_LVL             0x01
#define PROP_PA_BIAS_CLKDUTY          0x02
#define PROP_PA_TC                0x03

#define PROP_EZCONFIG_GROUP           0x24
#define PROP_EZCONFIG_MODULATION        0x00
#define PROP_EZCONFIG_CONFIG_SELECT       0x01

#define PROP_FREQ_CONTROL_GROUP         0x40
#define PROP_FREQ_CONTROL_INTE          0x00
#define PROP_FREQ_CONTROL_FRAC_2        0x01
#define PROP_FREQ_CONTROL_FRAC_1        0x02
#define PROP_FREQ_CONTROL_FRAC_0        0x03
#define PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_1 0x04
#define PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_0 0x05

#define PROP_GLOBAL_GROUP           0x00
#define PROP_GLOBAL_XO_TUNE           0x00
#define PROP_GLOBAL_CLK_CFG           0x01
#define PROP_GLOBAL_LOW_BATT_THRESH       0x02
#define PROP_GLOBAL_CONFIG            0x03
#define PROP_GLOBAL_WUT_CONFIG          0x04
#define PROP_GLOBAL_WUT_M_15_8          0x05
#define PROP_GLOBAL_WUT_M_7_0         0x06
#define PROP_GLOBAL_WUT_R           0x07
#define PROP_GLOBAL_WUT_LDC           0x08

#define PROP_PREAMBLE_GROUP           0x10
#define PROP_PREAMBLE_TX_LENGTH         0x00
#define PROP_PREAMBLE_CONFIG_STD_1        0x01
#define PROP_PREAMBLE_CONFIG_NSTD       0x02
#define PROP_PREAMBLE_CONFIG_STD_2        0x03
#define PROP_PREAMBLE_CONFIG          0x04
#define PROP_PREAMBLE_PATTERN_31_24       0x05
#define PROP_PREAMBLE_PATTERN_23_16       0x06
#define PROP_PREAMBLE_PATTERN_15_8        0x07
#define PROP_PREAMBLE_PATTERN_7_0       0x08

#define PROP_SYNC_GROUP             0x11
#define PROP_SYNC_CONFIG            0x00
#define PROP_SYNC_BITS_15_8           0x03
#define PROP_SYNC_BITS_7_0            0x04

#define PROP_PKT_GROUP              0x12
#define PROP_PKT_CRC_CONFIG           0x00
#define PROP_PKT_WHT_POLY_15_8          0x01  
#define PROP_PKT_WHT_POLY_7_0         0x02
#define PROP_PKT_WHT_SEED_15_8          0x03
#define PROP_PKT_WHT_SEED_7_0           0x04
#define PROP_PKT_WHT_BIT_NUM          0x05
#define PROP_PKT_CONFIG1            0x06
#define PROP_PKT_CHIP_MAP           0x07  
#define PROP_PKT_LEN              0x08
#define PROP_PKT_LEN_FIELD_SOURCE       0x09
#define PROP_PKT_LEN_ADJUST           0x0A
#define PROP_PKT_TX_THRESHOLD         0x0B
#define PROP_PKT_RX_THRESHOLD         0x0C
#define PROP_PKT_FIELD_1_LENGTH_12_8      0x0D
#define PROP_PKT_FIELD_1_LENGTH_7_0       0x0E
#define PROP_PKT_FIELD_1_CONFIG         0x0F
#define PROP_PKT_FIELD_1_CRC_CONFIG       0x10
#define PROP_PKT_FIELD_2_LENGTH_12_8      0x11
#define PROP_PKT_FIELD_2_LENGTH_7_0       0x12
#define PROP_PKT_FIELD_2_CONFIG         0x13
#define PROP_PKT_FIELD_2_CRC_CONFIG       0x14
#define PROP_PKT_FIELD_3_LENGTH_12_8      0x15
#define PROP_PKT_FIELD_3_LENGTH_7_0       0x16
#define PROP_PKT_FIELD_3_CONFIG         0x17
#define PROP_PKT_FIELD_3_CRC_CONFIG       0x18
#define PROP_PKT_FIELD_4_LENGTH_12_8      0x19
#define PROP_PKT_FIELD_4_LENGTH_7_0       0x1A
#define PROP_PKT_FIELD_4_CONFIG         0x1B
#define PROP_PKT_FIELD_4_CRC_CONFIG       0x1C
#define PROP_PKT_FIELD_5_LENGTH_12_8      0x1D
#define PROP_PKT_FIELD_5_LENGTH_7_0       0x1E
#define PROP_PKT_FIELD_5_CONFIG         0x1F
#define PROP_PKT_FIELD_5_CRC_CONFIG       0x20
#define PROP_PKT_RX_FIELD_1_LENGTH_12_8     0x21
#define PROP_PKT_RX_FIELD_1_LENGTH_7_0      0x22
#define PROP_PKT_RX_FIELD_1_CONFIG        0x23
#define PROP_PKT_RX_FIELD_1_CRC_CONFIG      0x24
#define PROP_PKT_RX_FIELD_2_LENGTH_12_8     0x25
#define PROP_PKT_RX_FIELD_2_LENGTH_7_0      0x26
#define PROP_PKT_RX_FIELD_2_CONFIG        0x27
#define PROP_PKT_RX_FIELD_2_CRC_CONFIG      0x28
#define PROP_PKT_RX_FIELD_3_LENGTH_12_8     0x29
#define PROP_PKT_RX_FIELD_3_LENGTH_7_0      0x2A
#define PROP_PKT_RX_FIELD_3_CONFIG        0x2B
#define PROP_PKT_RX_FIELD_3_CRC_CONFIG      0x2C
#define PROP_PKT_RX_FIELD_4_LENGTH_12_8     0x2D
#define PROP_PKT_RX_FIELD_4_LENGTH_7_0      0x2E
#define PROP_PKT_RX_FIELD_4_CONFIG        0x2F
#define PROP_PKT_RX_FIELD_4_CRC_CONFIG      0x30
#define PROP_PKT_RX_FIELD_5_LENGTH_12_8     0x31
#define PROP_PKT_RX_FIELD_5_LENGTH_7_0      0x32
#define PROP_PKT_RX_FIELD_5_CONFIG        0x33
#define PROP_PKT_RX_FIELD_5_CRC_CONFIG      0x34

#define PROP_MODEM_GROUP            0x20
#define PROP_MODEM_MOD_TYPE           0x00
#define PROP_MODEM_MAP_CONTROL          0x01
#define PROP_MODEM_DSM_CTRL           0x02
#define PROP_MODEM_DATA_RATE_2          0x03
#define PROP_MODEM_DATA_RATE_1          0x04
#define PROP_MODEM_DATA_RATE_0          0x05
#define PROP_MODEM_FREQ_DEV_2         0x0A
#define PROP_MODEM_FREQ_DEV_1         0x0B
#define PROP_MODEM_FREQ_DEV_0         0x0C
#define PROP_MODEM_RESERVED_20_0D       0x0D
#define PROP_MODEM_RESERVED_20_0E       0x0E
#define PROP_MODEM_FSK4_MAP           0x3F
#define PROP_MODEM_ANT_DIV_CONTROL        0x49
#define PROP_MODEM_RSSI_THRESH          0x4A
#define PROP_MODEM_RSSI_JUMP_THRESH       0x4B
#define PROP_MODEM_RSSI_CONTROL         0x4C
#define PROP_MODEM_RSSI_CONTROL2        0x4D
#define PROP_MODEM_RSSI_COMP          0x4E
#define PROP_MODEM_RESERVED_20_50       0x50

#define PROP_MATCH_GROUP            0x30
#define PROP_MATCH_VALUE_1            0x00
#define PROP_MATCH_MASK_1           0x01
#define PROP_MATCH_CTRL_1           0x02
#define PROP_MATCH_VALUE_2            0x03
#define PROP_MATCH_MASK_2           0x04
#define PROP_MATCH_CTRL_2           0x05
#define PROP_MATCH_VALUE_3            0x06
#define PROP_MATCH_MASK_3           0x07
#define PROP_MATCH_CTRL_3           0x08
#define PROP_MATCH_VALUE_4            0x09
#define PROP_MATCH_MASK_4           0x0A
#define PROP_MATCH_CTRL_4           0x0B

#define PROP_FREQ_CONTROL_GROUP         0x40
#define PROP_FREQ_CONTROL_W_SIZE        0x06
#define PROP_FREQ_CONTROL_VCOCNT_RX_ADJ     0x07

#define PROP_RX_HOP_GROUP           0x50
#define PROP_RX_HOP_CONTROL           0x00
#define PROP_RX_HOP_TABLE_SIZE          0x01
#define PROP_RX_HOP_TABLE_ENTRY_0       0x02
#define PROP_RX_HOP_TABLE_ENTRY_1       0x03
#define PROP_RX_HOP_TABLE_ENTRY_2       0x04
#define PROP_RX_HOP_TABLE_ENTRY_3       0x05
#define PROP_RX_HOP_TABLE_ENTRY_4       0x06
#define PROP_RX_HOP_TABLE_ENTRY_5       0x07
#define PROP_RX_HOP_TABLE_ENTRY_6       0x08
#define PROP_RX_HOP_TABLE_ENTRY_7       0x09
#define PROP_RX_HOP_TABLE_ENTRY_8       0x0A
#define PROP_RX_HOP_TABLE_ENTRY_9       0x0B
#define PROP_RX_HOP_TABLE_ENTRY_10        0x0C
#define PROP_RX_HOP_TABLE_ENTRY_11        0x0D
#define PROP_RX_HOP_TABLE_ENTRY_12        0x0E
#define PROP_RX_HOP_TABLE_ENTRY_13        0x0F
#define PROP_RX_HOP_TABLE_ENTRY_14        0x10
#define PROP_RX_HOP_TABLE_ENTRY_15        0x11
#define PROP_RX_HOP_TABLE_ENTRY_16        0x12
#define PROP_RX_HOP_TABLE_ENTRY_17        0x13
#define PROP_RX_HOP_TABLE_ENTRY_18        0x14
#define PROP_RX_HOP_TABLE_ENTRY_19        0x15
#define PROP_RX_HOP_TABLE_ENTRY_20        0x16
#define PROP_RX_HOP_TABLE_ENTRY_21        0x17
#define PROP_RX_HOP_TABLE_ENTRY_22        0x18
#define PROP_RX_HOP_TABLE_ENTRY_23        0x19
#define PROP_RX_HOP_TABLE_ENTRY_24        0x1A
#define PROP_RX_HOP_TABLE_ENTRY_25        0x1B
#define PROP_RX_HOP_TABLE_ENTRY_26        0x1C
#define PROP_RX_HOP_TABLE_ENTRY_27        0x1D
#define PROP_RX_HOP_TABLE_ENTRY_28        0x1E
#define PROP_RX_HOP_TABLE_ENTRY_29        0x1F
#define PROP_RX_HOP_TABLE_ENTRY_30        0x20
#define PROP_RX_HOP_TABLE_ENTRY_31        0x21
#define PROP_RX_HOP_TABLE_ENTRY_32        0x22
#define PROP_RX_HOP_TABLE_ENTRY_33        0x23
#define PROP_RX_HOP_TABLE_ENTRY_34        0x24
#define PROP_RX_HOP_TABLE_ENTRY_35        0x25
#define PROP_RX_HOP_TABLE_ENTRY_36        0x26
#define PROP_RX_HOP_TABLE_ENTRY_37        0x27
#define PROP_RX_HOP_TABLE_ENTRY_38        0x28
#define PROP_RX_HOP_TABLE_ENTRY_39        0x29
#define PROP_RX_HOP_TABLE_ENTRY_40        0x2A
#define PROP_RX_HOP_TABLE_ENTRY_41        0x2B
#define PROP_RX_HOP_TABLE_ENTRY_42        0x2C
#define PROP_RX_HOP_TABLE_ENTRY_43        0x2D
#define PROP_RX_HOP_TABLE_ENTRY_44        0x2E
#define PROP_RX_HOP_TABLE_ENTRY_45        0x2F
#define PROP_RX_HOP_TABLE_ENTRY_46        0x30
#define PROP_RX_HOP_TABLE_ENTRY_47        0x31
#define PROP_RX_HOP_TABLE_ENTRY_48        0x32
#define PROP_RX_HOP_TABLE_ENTRY_49        0x33
#define PROP_RX_HOP_TABLE_ENTRY_50        0x34
#define PROP_RX_HOP_TABLE_ENTRY_51        0x35
#define PROP_RX_HOP_TABLE_ENTRY_52        0x36
#define PROP_RX_HOP_TABLE_ENTRY_53        0x37
#define PROP_RX_HOP_TABLE_ENTRY_54        0x38
#define PROP_RX_HOP_TABLE_ENTRY_55        0x39
#define PROP_RX_HOP_TABLE_ENTRY_56        0x3A
#define PROP_RX_HOP_TABLE_ENTRY_57        0x3B
#define PROP_RX_HOP_TABLE_ENTRY_58        0x3C
#define PROP_RX_HOP_TABLE_ENTRY_59        0x3D
#define PROP_RX_HOP_TABLE_ENTRY_60        0x3E
#define PROP_RX_HOP_TABLE_ENTRY_61        0x3F
#define PROP_RX_HOP_TABLE_ENTRY_62        0x40
#define PROP_RX_HOP_TABLE_ENTRY_63        0x41

/* This section contains command map declarations */
struct si446x_reply_GENERIC_map {
        uint8_t  REPLY[16];
};

struct si446x_reply_PART_INFO_map {
        uint8_t  CHIPREV;
        uint16_t  PART;
        uint8_t  PBUILD;
        uint16_t  ID;
        uint8_t  CUSTOMER;
        uint8_t  ROMID;
};

struct si446x_reply_FUNC_INFO_map {
        uint8_t  REVEXT;
        uint8_t  REVBRANCH;
        uint8_t  REVINT;
        uint8_t  FUNC;
};

struct si446x_reply_GET_PROPERTY_map {
        uint8_t  DATA[16];
};

struct si446x_reply_GPIO_PIN_CFG_map {
        uint8_t  GPIO[4];
        uint8_t  NIRQ;
        uint8_t  SDO;
        uint8_t  GEN_CONFIG;
};

struct si446x_reply_FIFO_INFO_map {
        uint8_t  RX_FIFO_COUNT;
        uint8_t  TX_FIFO_SPACE;
};

struct si446x_reply_GET_INT_STATUS_map {
        uint8_t  INT_PEND;
        uint8_t  INT_STATUS;
        uint8_t  PH_PEND;
        uint8_t  PH_STATUS;
        uint8_t  MODEM_PEND;
        uint8_t  MODEM_STATUS;
        uint8_t  CHIP_PEND;
        uint8_t  CHIP_STATUS;
};

struct si446x_reply_REQUEST_DEVICE_STATE_map {
        uint8_t  CURR_STATE;
        uint8_t  CURRENT_CHANNEL;
};

struct si446x_reply_READ_CMD_BUFF_map {
        uint8_t  BYTE[16];
};

struct si446x_reply_FRR_A_READ_map {
        uint8_t  FRR_A_VALUE;
        uint8_t  FRR_B_VALUE;
        uint8_t  FRR_C_VALUE;
        uint8_t  FRR_D_VALUE;
};

struct si446x_reply_FRR_B_READ_map {
        uint8_t  FRR_B_VALUE;
        uint8_t  FRR_C_VALUE;
        uint8_t  FRR_D_VALUE;
        uint8_t  FRR_A_VALUE;
};

struct si446x_reply_FRR_C_READ_map {
        uint8_t  FRR_C_VALUE;
        uint8_t  FRR_D_VALUE;
        uint8_t  FRR_A_VALUE;
        uint8_t  FRR_B_VALUE;
};

struct si446x_reply_FRR_D_READ_map {
        uint8_t  FRR_D_VALUE;
        uint8_t  FRR_A_VALUE;
        uint8_t  FRR_B_VALUE;
        uint8_t  FRR_C_VALUE;
};

struct si446x_reply_IRCAL_MANUAL_map {
        uint8_t  IRCAL_AMP_REPLY;
        uint8_t  IRCAL_PH_REPLY;
};

struct si446x_reply_PACKET_INFO_map {
        uint16_t  LENGTH;
};

struct si446x_reply_GET_MODEM_STATUS_map {
        uint8_t  MODEM_PEND;
        uint8_t  MODEM_STATUS;
        uint8_t  CURR_RSSI;
        uint8_t  LATCH_RSSI;
        uint8_t  ANT1_RSSI;
        uint8_t  ANT2_RSSI;
        uint16_t  AFC_FREQ_OFFSET;
};

struct si446x_reply_READ_RX_FIFO_map {
        uint8_t  DATA[2];
};

struct si446x_reply_GET_ADC_READING_map {
        uint16_t  GPIO_ADC;
        uint16_t  BATTERY_ADC;
        uint16_t  TEMP_ADC;
};

struct si446x_reply_GET_PH_STATUS_map {
        uint8_t  PH_PEND;
        uint8_t  PH_STATUS;
};

struct si446x_reply_GET_CHIP_STATUS_map {
        uint8_t  CHIP_PEND;
        uint8_t  CHIP_STATUS;
        uint8_t  CMD_ERR_STATUS;
        uint8_t  CMD_ERR_CMD_ID;
};


/* The union that stores the reply written back to the host registers */
union si446x_cmd_reply_union {
        uint8_t                                                          RAW[16];
        struct si446x_reply_GENERIC_map                                  GENERIC;
        struct si446x_reply_PART_INFO_map                                PART_INFO;
        struct si446x_reply_FUNC_INFO_map                                FUNC_INFO;
        struct si446x_reply_GET_PROPERTY_map                             GET_PROPERTY;
        struct si446x_reply_GPIO_PIN_CFG_map                             GPIO_PIN_CFG;
        struct si446x_reply_FIFO_INFO_map                                FIFO_INFO;
        struct si446x_reply_GET_INT_STATUS_map                           GET_INT_STATUS;
        struct si446x_reply_REQUEST_DEVICE_STATE_map                     REQUEST_DEVICE_STATE;
        struct si446x_reply_READ_CMD_BUFF_map                            READ_CMD_BUFF;
        struct si446x_reply_FRR_A_READ_map                               FRR_A_READ;
        struct si446x_reply_FRR_B_READ_map                               FRR_B_READ;
        struct si446x_reply_FRR_C_READ_map                               FRR_C_READ;
        struct si446x_reply_FRR_D_READ_map                               FRR_D_READ;
        struct si446x_reply_IRCAL_MANUAL_map                             IRCAL_MANUAL;
        struct si446x_reply_PACKET_INFO_map                              PACKET_INFO;
        struct si446x_reply_GET_MODEM_STATUS_map                         GET_MODEM_STATUS;
        struct si446x_reply_READ_RX_FIFO_map                             READ_RX_FIFO;
        struct si446x_reply_GET_ADC_READING_map                          GET_ADC_READING;
        struct si446x_reply_GET_PH_STATUS_map                            GET_PH_STATUS;
        struct si446x_reply_GET_CHIP_STATUS_map                          GET_CHIP_STATUS;
};


#define PHIT_STATE_FILTER_MATCH                 0x80
#define PHIT_STATE_FILTER_MISS                  0x40
#define PHIT_STATE_PACKET_SENT                  0x20
#define PHIT_STATE_PACKET_RX                    0x10
#define PHIT_STATE_CRC_ERROR                    0x01
#define PHIT_STATE_ALT_CRC_ERROR              0x02
#define PHIT_STATE_TX_FIFO_ALMOST_EMPTY         0x04
#define PHIT_STATE_FRX_FIFO_ALMOST_FULL         0x08
#define PHIT_STATE_FAULT                        -1

enum
{
    SI446X_SUCCESS,
    SI446X_NO_PATCH,
    SI446X_CTS_TIMEOUT,
    SI446X_PATCH_FAIL,
    SI446X_COMMAND_ERROR
};
//-----------------------------------------------------------------------------
// Header File PreProcessor Directive
//-----------------------------------------------------------------------------
#endif                                 // #define SI446X_B0_DEFS_H

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
