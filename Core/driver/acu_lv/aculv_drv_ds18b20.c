#include "aculv_drv_ds18b20.h"
#include "stm32h7xx_hal.h"
#include "acu_lv_config_pinout.h"

volatile uint8_t recvFlag;
volatile uint16_t rc_buffer[5];

/**
    @author Stanislav Lakhtin
    @date   11.07.2016
    @brief  Implementation of 1-wire protocol based on libopencm3 library for STM32F103 microcontroller
            The library may also function correctly on other microcontrollers (verification required).
            Verification is necessary to ensure correct UART/USART configuration for half-duplex operation.
            The general concept involves using the hardware USART of the microcontroller to emulate 1-wire operation.
            Devices are connected to the selected USART via its TX pin, which must be pulled to the power supply line with a 4.7K resistor.
            The library implementation performs loopback of RX to TX inside the microcontroller, leaving the RX pin available for use in other tasks.
            The library implementation assumes possible simultaneous operation with independent buses for all
            possible UART/USART in the microcontroller. In this case, all buses (up to 5) will be addressed and polled individually.
 */

#define ow_uart DS18B20_UART_HANDLE
#define OW_USART UART4

#define NO_ENERGY_METER

/*********************************************************************************************/
volatile float ds18b20_temp[MAXDEVICES_ON_THE_BUS];

uint8_t devices = 15;
OneWire ow;
uint32_t pDelay = 300;
uint8_t sensor;

Temperature t;

DEVInfo devInfo;


char *crcOK;

void aculv_drv_ds18b20_init()
{
    //enable power for ds18b20
    //Two configurations available depeding if we have energy meter or not
    #ifdef NO_ENERGY_METER
    // this is according to ryland notes in LV altium sheet
        // if no energy meter, VDD enable pin must be high
        HAL_GPIO_WritePin(DS18B20_VDD_EN_PORT, DS18B20_VDD_EN_PIN, GPIO_PIN_SET);

        //if no energy meter, weak pullup must also be high
        // might not work cause pin is open drain
        HAL_GPIO_WritePin(DS18B20_WEAK_PULL_UP_PORT, DS18B20_WEAK_PULL_UP_PIN, GPIO_PIN_RESET);
    #else
    // if energy meter present, VDD enable pin must be low
        HAL_GPIO_WritePin(DS18B20_VDD_EN_PORT, DS18B20_VDD_EN_PIN, GPIO_PIN_RESET);

        //if energy meter, weak pullup must be low
        // might not work cause pin is open drain
        HAL_GPIO_WritePin(DS18B20_WEAK_PULL_UP_PORT, DS18B20_WEAK_PULL_UP_PIN, GPIO_PIN_RESET);
    #endif

    get_ROMid();
}



uint16_t USART_ReceiveData(USART_TypeDef* USARTx)
{
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));

  /* Receive Data */
  return (uint16_t)(USARTx->RDR & (uint16_t)0x01FF);
}

void USART_SendData(USART_TypeDef* USARTx, uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));
  assert_param(IS_USART_DATA(Data));

  /* Transmit Data */
  USARTx->TDR = (Data & (uint16_t)0x01FF);
}


uint8_t getUsartIndex(void);

void usart_setup(uint32_t baud)
{
    // 1. STM32G4 REQUIREMENT: De-init the UART before changing speeds
    HAL_UART_DeInit(ow_uart);

    // 2. Set up the new parameters
    ow_uart->Instance = OW_USART;
    ow_uart->Init.BaudRate = baud;
    ow_uart->Init.WordLength = UART_WORDLENGTH_8B;
    ow_uart->Init.StopBits = UART_STOPBITS_1;
    ow_uart->Init.Parity = UART_PARITY_NONE;
    ow_uart->Init.Mode = UART_MODE_TX_RX;
    ow_uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    ow_uart->Init.OverSampling = UART_OVERSAMPLING_16;

    // 3. Re-initialize
    if (HAL_HalfDuplex_Init(ow_uart) != HAL_OK)
    {
        __asm__("NOP");
    }

    // 4. Re-enable the Receive Interrupt
    __HAL_UART_ENABLE_IT(ow_uart, UART_IT_RXNE);
}

void owInit(OneWire *ow)
{
  int i=0, k = 0;
  for (; i < MAXDEVICES_ON_THE_BUS; i++) {
   uint8_t *r = (uint8_t *)&ow->ids[i];      
    k=0;
    for (; k < 8; k++)
    r[k] = 0;
  }
  k=0;
  for (; k < 8; k++)
    ow->lastROM[k] = 0x00;
  ow->lastDiscrepancy = 64;

}

void owReadHandler()
{ //USART interrupt handler
  uint8_t index = getUsartIndex();

  // --- ADD THIS BLOCK FOR STM32G4 ---
    if (__HAL_UART_GET_FLAG(ow_uart, UART_FLAG_ORE)) {
         __HAL_UART_CLEAR_OREFLAG(ow_uart);
    }
    // ----------------------------------
  /* Check if interrupt was caused by RXNE. */
  if (((OW_USART->ISR & UART_FLAG_RXNE) != (uint16_t)RESET)) {

    /* Get data from peripheral and clear flag */
		while ((OW_USART->ISR & UART_FLAG_RXNE) == (uint16_t)RESET){;}
    rc_buffer[index] = USART_ReceiveData(OW_USART);              
    recvFlag &= ~(1 << index);//clear flag - response received 
  }
}

/** Implementation of RESET on 1-wire bus
 *
 * @param N usart -- USART selected for 1-wire implementation
 * @return Returns 1 if something is on the bus, 0 otherwise
 */

uint16_t owResetCmd()
{
	uint16_t owPresence;
	
	usart_setup(9600);

  owSend(0xF0); // Send RESET pulse
  owPresence = owEchoRead(); // Wait for PRESENCE on bus and return it

	usart_setup(115200);// reconfigure UART speed
  return owPresence;
}

// checks which UART will be used
uint8_t getUsartIndex()
{
//	uint8_t result;
//	if(OW_USART==USART1)result = 0;
//	else if (OW_USART==USART2)result = 1;
//	else if (OW_USART==USART3)result = 2;
	return 0;
}

void owSend(uint16_t data)
{
  recvFlag |= (1 << getUsartIndex());//set flag if we enter interrupt handler, it will be cleared there
  USART_SendData(OW_USART, data);//send data
	while(__HAL_UART_GET_FLAG(ow_uart, UART_FLAG_TC) == RESET);//wait for transmission to complete
}

//read whether we received a 1 or 0 in response
uint8_t owReadSlot(uint16_t data)
{
  return (data == OW_READ) ? 1 : 0; //if 0xFF was received, bit = 1, otherwise bit = 0
}

uint16_t owEchoRead()
{
  uint8_t i = getUsartIndex();//get USART number
  uint16_t pause = 1000;
  while (recvFlag & (1 << i) && pause--);// wait for response but not more than pause
  return rc_buffer[i];//depending on the USART number used 
}

//decompose 1 byte into 8 bytes, encode them into 1-wire packets
uint8_t *byteToBits(uint8_t ow_byte, uint8_t *bits)
{
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (ow_byte & 0x01) {//if current bit in byte is 1
      *bits = WIRE_1; //replace with value that will be 1 when transmitted over USART for 1-wire, i.e. 0xFF
    } else {
      *bits = WIRE_0;// same but for 0
    }
    bits++;
    ow_byte = ow_byte >> 1; //shift processed bit
  }
  return bits; //return array for transmission 
}

/**
 * Method sends 8 bytes sequentially, one for each bit in data
 * @param usart -- UART selected for 1-wire emulation
 * @param d -- data
 */
void owSendByte(uint8_t d)
{
  uint8_t data[8];
	int i;
  byteToBits(d, data);//convert byte to bits, array of bytes for UART transmission and 1-wire emulation
  for (i = 0; i < 8; ++i) {
    owSend(data[i]);
  }
}

//takes encoded byte array received over UART and converts it back to a byte
uint8_t bitsToByte(uint8_t *bits)
{
  uint8_t target_byte, i;
  target_byte = 0;
  for (i = 0; i < 8; i++) {
    target_byte = target_byte >> 1;
    if (*bits == WIRE_1) {//if 0xFF was received over USART, then we received a 1
      target_byte |= 0x80;//set most significant bit to 1
    }
    bits++;//move to next byte which is either 0=0x00 or 1=0xFF
  }
  return target_byte; //return the received byte
}

/* Calculate CRC8 of array mas with length Len */
uint8_t owCRC(uint8_t *mas, uint8_t Len)
{
  uint8_t i, dat, crc, fb, st_byt;
  st_byt = 0;
  crc = 0;
  do {
    dat = mas[st_byt];
    for (i = 0; i < 8; i++) {  // bit counter in byte
      fb = crc ^ dat;
      fb &= 1;
      crc >>= 1;
      dat >>= 1;
      if (fb == 1) crc ^= 0x8c; // polynomial
    }
    st_byt++;
  } while (st_byt < Len); // byte counter in array
  return crc;
}

uint8_t owCRC8(RomCode *rom)
{
  return owCRC((uint8_t*)rom, 7);                        
}

/*
 * return 1 if has got one more address
 * return 0 if hasn't
 * return -1 if error reading happened
 *
 * refactor to use callback functions for error handling
 */
int hasNextRom(OneWire *ow, uint8_t *ROM)
{
	uint8_t ui32BitNumber = 0;
  int zeroFork = -1;
	uint8_t i = 0;
  if (owResetCmd() == ONEWIRE_NOBODY) { //is there anybody on the bus?
    return 0;
  }
  owSendByte(ONEWIRE_SEARCH);//
  do {
		uint8_t answerBit =0;
    int byteNum = ui32BitNumber / 8;
    uint8_t *current = (ROM) + byteNum;
    uint8_t cB, cmp_cB, searchDirection = 0;
    owSend(OW_READ); // read straight bit
    cB = owReadSlot(owEchoRead());//response from sensor
    owSend(OW_READ); // read inverse bit
    cmp_cB = owReadSlot(owEchoRead());//response from sensor
    if (cB == cmp_cB && cB == 1)//compare two responses
      return -1;//error - nobody responded
    if (cB != cmp_cB) { //normal situation - received either 10 or 01
      searchDirection = cB;//choose direction to move forward
			} else {//collision - received 00, i.e., current bit in ROMs is different
				if (ui32BitNumber == ow->lastDiscrepancy)//if current collision position equals previous one
        searchDirection = 1;//choose direction to move forward
      else {
        if (ui32BitNumber > ow->lastDiscrepancy) {//if we went further
          searchDirection = 0;//choose direction to move forward
        } else {
          searchDirection = (uint8_t) ((ow->lastROM[byteNum] >> ui32BitNumber % 8) & 0x01);
        }
        if (searchDirection == 0)
          zeroFork = ui32BitNumber;//remember fork
      }
    }
    // save bit
    if (searchDirection)
      *(current) |= 1 << ui32BitNumber % 8;//set bit in current byte
    answerBit = (uint8_t) ((searchDirection == 0) ? WIRE_0 : WIRE_1);// decide which device to disconnect
    owSend(answerBit);//disable conflicting devices
    ui32BitNumber++;//search for next bit
		} while (ui32BitNumber < 64);//until entire ROM found - all bits
  ow->lastDiscrepancy = zeroFork;//remember fork
  for (; i < 7; i++)
    ow->lastROM[i] = ROM[i];//remember last ROM
  return ow->lastDiscrepancy > 0;
}

// Returns number of devices on bus or error code if value is less than 0
int owSearchCmd(OneWire *ow)
{
  int device = 0, nextROM;
  owInit(ow);
  do {
    nextROM = hasNextRom(ow, (uint8_t*)(&ow->ids[device])); //pass pointer to structure for next ROM
    if (nextROM<0)
      return -1;
    device++;
		} while (nextROM && device < MAXDEVICES_ON_THE_BUS);//search while something exists and not more than define
		return device;//return sensor (device) index on bus
}

//sends ROM skip command, after which the next command will apply
void owSkipRomCmd(OneWire *ow)
{
  owResetCmd();                 //to all devices on bus
  owSendByte(ONEWIRE_SKIP_ROM);
}

//allows master to address specific slave device
void owMatchRomCmd(RomCode *rom)
{
	int i = 0;
  owResetCmd();
  owSendByte(ONEWIRE_MATCH_ROM);//address specific device
  for (; i < 8; i++)
	owSendByte(*(((uint8_t *) rom) + i));//traverse structure as array, first dereference gets i-th byte from structure
}

void owConvertTemperatureCmd(OneWire *ow, RomCode *rom)
{
  owMatchRomCmd(rom);//allows master to address specific slave device
  owSendByte(ONEWIRE_CONVERT_TEMPERATURE);//tell sensor to convert temperature
}

/**
 * Method for reading scratchad DS18B20 OR DS18S20
 * If sensor DS18B20 then data MUST be at least 9 byte
 * If sensor DS18S20 then data MUST be at least 2 byte
 * @param ow -- OneWire pointer
 * @param rom -- selected device on the bus
 * @param data -- buffer for data
 * @return data
 */
uint8_t *owReadScratchpadCmd(OneWire *ow, RomCode *rom, uint8_t *data) //read sensor memory
{
  uint16_t b = 0, p;
  switch (rom->family) {
    case DS18B20:
    case DS18S20:
      p = 72;  //9*8 = 72, equals 9 bytes of data
      break;
    default:
      return data;

  }
  owMatchRomCmd(rom);
  owSendByte(ONEWIRE_READ_SCRATCHPAD);//send command to read memory
  while (b < p) {// while we haven't processed 9 bytes
    uint8_t pos = (uint8_t) ((p - 8) / 8 - (b / 8)); //position of byte being processed
    uint8_t bt; 
		owSend(OW_READ);
    bt = owReadSlot(owEchoRead());//read data
    if (bt == 1)
      data[pos] |= 1 << b % 8;//set bit at required position
    else
      data[pos] &= ~(1 << b % 8);//clear bit at required position
    b++;//next bit
  }
  return data;
}

void owWriteDS18B20Scratchpad(OneWire *ow, RomCode *rom, uint8_t th, uint8_t tl, uint8_t conf)
{
  if (rom->family != DS18B20)
    return;
  owMatchRomCmd(rom);//address specific device
  owSendByte(ONEWIRE_WRITE_SCRATCHPAD);//we will write to memory
  owSendByte(th);//temperature thresholds
  owSendByte(tl);
  owSendByte(conf);
}

/**
 * Get last mesaured temperature from DS18B20 or DS18S20. These temperature MUST be measured in previous
 * opearions. If you want to measure new value you can set reSense in true. In this case next invocation
 * that method will return value calculated in that step.
 * @param ow -- OneWire bus pointer
 * @param rom -- selected device
 * @param reSense -- do you want resense temp for next time?
 * @return struct with data
 */
Temperature readTemperature(OneWire *ow, RomCode *rom, uint8_t reSense)
{
	Scratchpad_DS18B20 *sp;
	Scratchpad_DS18S20 *spP;
  Temperature t;
	uint8_t pad[9];
  t.inCelsus = 0x00;
  t.frac = 0x00;
  sp = (Scratchpad_DS18B20 *) &pad; 
  spP = (Scratchpad_DS18S20 *) &pad;
  switch (rom->family) {
    case DS18B20:
      owReadScratchpadCmd(ow, rom, pad);//read memory for DS18B20
      t.inCelsus = (int8_t) (sp->temp_msb << 4) | (sp->temp_lsb >> 4);//integer part
      t.frac = (uint8_t) ((((sp->temp_lsb & 0x0F)) * 10) >> 4);//fractional part
      break;
    case DS18S20:
      owReadScratchpadCmd(ow, rom, pad);//read memory for DS18S20
      t.inCelsus = spP->temp_lsb >> 1;
      t.frac = (uint8_t) 5 * (spP->temp_lsb & 0x01);
      break;
    default:
      return t;
  }
  if (reSense) {
    owConvertTemperatureCmd(ow, rom);//can immediately after reading data give sensor command to convert temperature
  }
  return t;
}

void owCopyScratchpadCmd(OneWire *ow, RomCode *rom)
{
  owMatchRomCmd(rom);
  owSendByte(ONEWIRE_COPY_SCRATCHPAD);
}

void owRecallE2Cmd(OneWire *ow, RomCode *rom)
{
  owMatchRomCmd(rom);
  owSendByte(ONEWIRE_RECALL_E2);
}


int get_ROMid (void)
{
	int i =0;
	if (owResetCmd() != ONEWIRE_NOBODY) {    // is anybody on the bus?
			devices = owSearchCmd(&ow);        // get ROM IDs of all devices on bus or return error code
			if (devices <= 0) {
				while (1){
					pDelay = 1000000;
					for (; i < pDelay * 1; i++)    /* Wait a bit. */
						__asm__("nop");
				}

			}
			i = 0;
			for (; i < devices; i++) {//output all found ROMs to console
				RomCode *r = &ow.ids[i];
				uint8_t crc = owCRC8(r);
				crcOK = (crc == r->crc)?"CRC OK":"CRC ERROR!";
				devInfo.device = i;

				sprintf(devInfo.info, "SN: %02X/%02X%02X%02X%02X%02X%02X/%02X", r->family, r->code[5], r->code[4], r->code[3],
						r->code[2], r->code[1], r->code[0], r->crc);

				if (crc != r->crc) {
					devInfo.device = i;
					sprintf (devInfo.info,"\n can't read cause CNC error");
				}
			}

		}
		pDelay = 1000000;
		for (i = 0; i < pDelay * 1; i++)
			__asm__("nop");

		if (strcmp(crcOK,"CRC OK") == 0) return 0;
		else return -1;
}

void get_Temperature (void)
{
	uint8_t i=0;
	for (; i < devices; i++) {
		switch ((ow.ids[i]).family) {//what sensor do we have
		case DS18B20:
			// will return value of previous measurement!
			t = readTemperature(&ow, &ow.ids[i], 1);
			ds18b20_temp[i] = (float)(t.inCelsus*10+t.frac)/10.0;
			break;
		case DS18S20:
			t = readTemperature(&ow, &ow.ids[i], 1);
			ds18b20_temp[i] = (float)(t.inCelsus*10+t.frac)/10.0;
			break;
		case 0x00:
			break;
		default:
			// error handler
			break;
		}
	}
//	pDelay = 4000000;
//	for (i = 0; i < pDelay * 1; i++){}   /* Wait a bit. */
}
