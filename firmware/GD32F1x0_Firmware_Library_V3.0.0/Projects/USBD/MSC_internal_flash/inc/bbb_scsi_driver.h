/*!
    \file  bbb_scsi_driver.h
    \brief the header file of the bbb_scsi_driver.c
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-09-06, V1.0.0, firmware for GD32F150
    2016-01-15, V1.0.1, firmware for GD32F150
    2016-04-30, V2.0.0, firmware update for GD32F150
*/

#ifndef BBB_SCSI_DRIVER_H
#define BBB_SCSI_DRIVER_H

#include "usbd_std.h"

#define BBB_CBW_SIGNATURE              0x43425355
#define BBB_CBW_LENGTH                 31

#define BBB_CSW_SIGNATURE              0x53425355
#define BBB_CSW_LENGTH                 13

#define SCSI_CMD_LENGTH                16

/* transport stage define */
#define COMMAND_STAGE                  0x00
#define DATA_IN_STAGE                  0x01
#define DATA_OUT_STAGE                 0x02
#define STATUS_STAGE                   0x03

/* SCSI command define */
#define INQUIRY                        0x12
#define READ_FORMAT_CAPACITIES         0x23
#define READ_CAPACITY                  0x25
#define READ_10                        0x28
#define WRITE_10                       0x2A
#define REQUEST_SENSE                  0x03
#define TEST_UNIT_READY                0x00
#define SCSI_MODE_SENSE6               0x1A

/* the array length define */
#define DISK_INQUIRY_DATA_LENGTH       36
#define FORMAT_CAPACITIES_DATA_LENGTH  12
#define READ_CAPACITIES_DATA_LENGTH    8
#define REQUEST_SENSE_DATA_LENGTH      18
#define SENSE6_DATA_LENGTH             8

typedef struct
{
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} bbb_cbw_struct;

typedef struct
{
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
} bbb_csw_struct;

/* function declarations */
/* parse the SCSI commands from the CBW */
void scsi_command_parse(void);
/* set the CSW struct */
void bbb_set_csw(uint32_t csw_tag, uint32_t csw_data_residue, uint8_t csw_status);
/* report array to host */
void bbb_report_array_to_host(void *pudev, uint8_t *p_data, uint32_t report_length);
/* process the scsi command */
void process_scsi_command(void *pudev);

#endif /* BBB_SCSI_DRIVER_H */
