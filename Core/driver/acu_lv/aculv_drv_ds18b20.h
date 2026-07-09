/**
    @author Stanislav Lakhtin
    @date   11.07.2016
    @brief  Implementation of the 1-Wire protocol based on the libopencm3 library for the STM32F103 microcontroller.
            The library may also function correctly on other microcontrollers (verification required).
            The general concept involves utilizing the microcontroller's hardware USART peripheral to emulate 1-Wire operation.
            Devices are connected to the selected USART via its TX pin, which must be pulled up to the power supply line using a 4.7 kΩ resistor.
            The library implementation internally loops the RX pin back to the TX pin within the microcontroller, thereby leaving the RX pin available for use in other tasks.
 */

#ifndef STM32_DS18X20_ONEWIRE_H
#define STM32_DS18X20_ONEWIRE_H
#include <stdint.h>
#define ONEWIRE_NOBODY 0xF0 //ROM Search Command
#define ONEWIRE_SEARCH 0xF0 //ROM search command
#define ONEWIRE_SKIP_ROM 0xCC  //ROM skip command
#define ONEWIRE_READ_ROM 0x33  
#define ONEWIRE_MATCH_ROM 0x55 //ROM match command allows master to address specific slave device
#define ONEWIRE_CONVERT_TEMPERATURE 0x44
#define ONEWIRE_READ_SCRATCHPAD 0xBE    //command to read sensor memory
#define ONEWIRE_WRITE_SCRATCHPAD 0x4E   //command to write to sensor memory
#define ONEWIRE_COPY_SCRATCHPAD 0x48
#define ONEWIRE_RECALL_E2 0xB8


#define MAXDEVICES_ON_THE_BUS 15  // 3 devices per segment with five segments


#define DS18B20 0x28  //sensor family code
#define DS18S20 0x10  //sensor family code 

#define WIRE_0    0x00 // 0x00 --default
#define WIRE_1    0xff //response
#define OW_READ   0xff

typedef struct {
  int8_t inCelsus;
  uint8_t frac;
} Temperature; //

typedef struct {
  uint8_t family;
  uint8_t code[6];
  uint8_t crc;
} RomCode; //

typedef struct {
  uint8_t crc;
  uint8_t reserved[3];
  uint8_t configuration;
  uint8_t tl;
  uint8_t th;
  uint8_t temp_msb;
  uint8_t temp_lsb;
} Scratchpad_DS18B20;//

typedef struct {
  uint8_t crc;
  uint8_t count_per;
  uint8_t count_remain;
  uint8_t reserved[2];
  uint8_t tl;
  uint8_t th;
  uint8_t temp_msb;
  uint8_t temp_lsb;
} Scratchpad_DS18S20;//

typedef struct {
  RomCode ids[MAXDEVICES_ON_THE_BUS]; //for all ROMs of our sensors
  int lastDiscrepancy;
  uint8_t lastROM[8]; //last read ROM for searching all ROMs
} OneWire;

typedef struct {
	int device;
	char info[30];
}DEVInfo;

extern DEVInfo devInfo;

void usart_setup_(uint32_t baud);

uint16_t owResetCmd(void);

int owSearchCmd(OneWire *ow);

void owSkipRomCmd(OneWire *ow);

uint8_t owCRC8(RomCode *rom);

void owMatchRomCmd(RomCode *rom);

void owConvertTemperatureCmd(OneWire *ow, RomCode *rom);

uint8_t* owReadScratchpadCmd(OneWire *ow, RomCode *rom, uint8_t *data);

void owCopyScratchpadCmd(OneWire *ow, RomCode *rom);

void owRecallE2Cmd(OneWire *ow, RomCode *rom);

Temperature readTemperature(OneWire *ow, RomCode *rom, uint8_t reSense);

void owSend(uint16_t data);

void owSendByte(uint8_t data);

uint16_t owEchoRead(void);

void owReadHandler(void);

int get_ROMid (void);

void get_Temperature (void);

void aculv_drv_ds18b20_init();


#endif //STM32_DS18X20_ONEWIRE_H
