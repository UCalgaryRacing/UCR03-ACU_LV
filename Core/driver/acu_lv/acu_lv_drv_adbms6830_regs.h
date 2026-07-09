/*
 * adbms6830_regs.h
 *
 * ADBMS6830B Register Definitions, Command Codes, and helper accessors.
 *
 * This header is intentionally structured so new register groups can be added
 * without turning the file into a single monolithic block. For each register
 * group, follow the same pattern:
 *   - Commands (if applicable)
 *   - Definitions (bit positions + masks)
 *   - Enums (typed values for fields)
 *   - Helpers (set/get/pack/unpack/defaults on a raw byte buffer)
 *
 * Based on ADBMS6830B datasheet Rev. 0 (January 2024).
 *
 *  Created on: Dec 8, 2025
 *      Author: esall
 */

#ifndef DRIVERS_ADBMS6830_REGS_H_
#define DRIVERS_ADBMS6830_REGS_H_

#include <stdint.h>
#include <stdbool.h>
#include "acu_lv_config.h"
#include "acu_lv_drv_adbms6830_types.h"

/*============================================================================*/
/* Command codes (SPI)                                                        */
/*============================================================================*/

/* ---- Configuration register group commands -------------------------------- */

/** Write Configuration Register Group A (CFGA). */
#define CMD_WRCFGA          0x0001U

/** Read Configuration Register Group A (CFGA). */
#define CMD_RDCFGA          0x0002U

/** Write Configuration Register Group B (CFGB). */
#define CMD_WRCFGB          0x0024U

/** Read Configuration Register Group B (CFGB). */
#define CMD_RDCFGB          0x0026U

/* ---- ADC conversion commands ---------------------------------------------- */

/**
 * Start Cell Voltage ADC Conversion (ADCV).
 *
 * Command bits [10:0]:
 *   [10:9] = 01 (ADCV identifier)
 *   [8]    = RD   (0=no redundancy, 1=redundant measurement)
 *   [7]    = CONT (0=single shot, 1=continuous)
 *   [6:5]  = 11   (fixed)
 *   [4]    = DCP  (0=discharge not permitted, 1=discharge permitted)
 *   [3]    = 0    (fixed for ADCV)
 *   [2]    = RSTF (0=normal, 1=reset IIR filter)
 *   [1:0]  = OW   (open wire mode: 00=disabled)
 *
 * Base command (all options = 0): 0x0260
 */
#define CMD_ADCV_BASE       0x0260U
#define CMD_ADCV_RD         (1U << 8)   /**< Enable redundant measurement */
#define CMD_ADCV_CONT       (1U << 7)   /**< Enable continuous mode */
#define CMD_ADCV_DCP        (1U << 4)   /**< Permit discharge during ADC */
#define CMD_ADCV_RSTF       (1U << 2)   /**< Reset IIR filter */

/* ---- Cell voltage register read commands ---------------------------------- */

/** Read Cell Voltage Register Group A (C1V, C2V, C3V). */
#define CMD_RDCVA           0x0004U

/** Read Cell Voltage Register Group B (C4V, C5V, C6V). */
#define CMD_RDCVB           0x0006U

/** Read Cell Voltage Register Group C (C7V, C8V, C9V). */
#define CMD_RDCVC           0x0008U

/** Read Cell Voltage Register Group D (C10V, C11V, C12V). */
#define CMD_RDCVD           0x000AU

/** Read Cell Voltage Register Group E (C13V, C14V, C15V). */
#define CMD_RDCVE           0x0009U

/** Read Cell Voltage Register Group F (C16V). */
#define CMD_RDCVF           0x000BU

/* ---- GPIO/AUX ADC commands ------------------------------------------------ */

/**
 * Start AUX (GPIO) ADC Conversion (ADAX).
 *
 * Command bits [10:0]:
 *   [10:9] = 11 (ADAX identifier)
 *   [8]    = OW   (0=no open wire, 1=open wire detection)
 *   [7]    = PUP  (0=pull-down, 1=pull-up for OW detection)
 *   [6]    = CH[4]
 *   [5]    = 0    (fixed)
 *   [4]    = 1    (fixed for ADAX)
 *   [3:0]  = CH[3:0] (channel select, 0=all)
 *
 * Base command (all channels, no OW): 0x0410
 */
#define CMD_ADAX_BASE       0x0410U
#define CMD_ADAX_OW         (1U << 8)   /**< Enable open wire detection */
#define CMD_ADAX_PUP        (1U << 7)   /**< Use pull-up for OW detection */
#define CMD_ADAX2_BASE 0x0400U
/** Read Auxiliary Register Group A (GPIO1, GPIO2, GPIO3). */
#define CMD_RDAUXA          0x0019U

/** Read Auxiliary Register Group B (GPIO4, GPIO5, GPIO6). */
#define CMD_RDAUXB          0x001AU

/** Read Auxiliary Register Group C (GPIO7, GPIO8, GPIO9). */
#define CMD_RDAUXC          0x001BU

/** Read Auxiliary Register Group D (GPIO10, VM, VP). */
#define CMD_RDAUXD          0x001FU

/* ---- ADC Polling commands ------------------------------------------------- */

/**
 * Poll ADC conversion status.
 *
 * After sending a poll command, hold CS low and clock SCK. SDO will be:
 *   - LOW if ADC conversion is in progress
 *   - HIGH if ADC conversion is complete
 *
 * For daisy-chain: SDO status is valid after 2*N clock cycles (N = device count).
 */
#define CMD_PLADC           0x0718U   /**< Poll all ADCs (C-ADC, S-ADC, AUX, AUX2) */
#define CMD_PLCADC          0x0719U   /**< Poll C-ADC only */
#define CMD_PLSADC          0x071AU   /**< Poll S-ADC only */
#define CMD_PLAUX           0x071BU   /**< Poll AUX ADC only */
#define CMD_PLAUX2          0x071CU   /**< Poll AUX2 ADC only */

/*============================================================================*/
/* ADC conversion parameters                                                  */
/*============================================================================*/

/**
 * Cell voltage ADC LSB value.
 * Each ADC count = 150 µV = 0.000150 V
 */
#define ADBMS_CELL_ADC_LSB_UV   150U
#define ADBMS_CELL_ADC_LSB_V    0.000150f

/** Conversion time for single-shot ADCV (typical, worst case ~2.2ms). */
#define ADBMS_ADCV_CONV_TIME_MS 3U

/**
 * GPIO ADC LSB value.
 * Each ADC count = 100 µV = 0.000100 V
 */
#define ADBMS_GPIO_ADC_LSB_UV   100U
#define ADBMS_GPIO_ADC_LSB_V    0.000100f

/** Conversion time for single-shot ADAX (all GPIO channels, ~4ms). */
#define ADBMS_ADAX_CONV_TIME_MS 5U

/*============================================================================*/
/* Wake-up and timing parameters                                              */
/*============================================================================*/

/**
 * Time to wake device from SLEEP state (isoSPI idle → ready).
 * Datasheet: tWAKE = 400 µs typical, 1.2 ms max
 */
#define ADBMS_TWAKE_MS          2U

/**
 * Time to wake device from STANDBY state (already powered, faster).
 * Datasheet: tREADY = 10 µs typical
 */
#define ADBMS_TREADY_MS         1U

/**
 * Idle timeout before isoSPI enters low-power idle state.
 * Datasheet: tIDLE = 5.1 ms typical, 6.7 ms max
 */
#define ADBMS_TIDLE_MS          7U

/**
 * Minimum CS pulse width for wake-up detection.
 * Datasheet: tDWELL = 240 ns min
 */
#define ADBMS_TDWELL_MS         1U

/**
 * Recommended wait time between wake-up pulses for daisy-chain.
 * Must be > tREADY/tWAKE but < tIDLE
 */
#define ADBMS_WAKEUP_PULSE_MS   1U

/*============================================================================*/
/* Register group: CFGA (Configuration Register Group A)                      */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* Definitions (bit positions + masks)                                        */
/*----------------------------------------------------------------------------*/

/* CFGAR0: REFON + CTH[2:0] -------------------------------------------------- */
/** Reference always on (bit 7 of CFGA byte 0). */
#define CFGA_REFON_POS          7U
#define CFGA_REFON_MASK         0x80U

/** Comparator threshold (bits [2:0] of CFGA byte 0). */
#define CFGA_CTH_POS            0U
#define CFGA_CTH_MASK           0x07U

/* CFGAR1: FLAG_D[7:0] ------------------------------------------------------- */
/** Flag diagnostics configuration (bits [7:0] of CFGA byte 1). */
#define CFGA_FLAG_D_POS         0U
#define CFGA_FLAG_D_MASK        0xFFU

/* CFGAR2: SOAKON, OWRNG, OWA[2:0] ------------------------------------------- */
/** Soak time enable (bit 7 of CFGA byte 2). */
#define CFGA_SOAKON_POS         7U
#define CFGA_SOAKON_MASK        0x80U

/** Open wire soak range (bit 6 of CFGA byte 2). */
#define CFGA_OWRNG_POS          6U
#define CFGA_OWRNG_MASK         0x40U

/** Open wire algorithm select (bits [5:3] of CFGA byte 2). */
#define CFGA_OWA_POS            3U
#define CFGA_OWA_MASK           0x38U

/* CFGAR3-4: GPO[10:1] ------------------------------------------------------- */
/**
 * GPO pulldown configuration: 10-bit field spread across bytes 3 and 4.
 *   - CFGAR3: GPO[8:1] in bits [7:0]
 *   - CFGAR4: GPO[10:9] in bits [1:0]
 *
 * Polarity: 0 = pull-down ON (enabled), 1 = pull-down OFF (disabled, default)
 */
#define CFGA_GPO_POS            0U
#define CFGA_GPO_MASK           0x03FFU

/* Individual GPO bitmasks (for use with adbms_cfga_set_gpo) */
#define ADBMS_GPO1_MASK         (1U << 0)   /**< GPO1 bitmask */
#define ADBMS_GPO2_MASK         (1U << 1)   /**< GPO2 bitmask */
#define ADBMS_GPO3_MASK         (1U << 2)   /**< GPO3 bitmask */
#define ADBMS_GPO4_MASK         (1U << 3)   /**< GPO4 bitmask */
#define ADBMS_GPO5_MASK         (1U << 4)   /**< GPO5 bitmask */
#define ADBMS_GPO6_MASK         (1U << 5)   /**< GPO6 bitmask */
#define ADBMS_GPO7_MASK         (1U << 6)   /**< GPO7 bitmask */
#define ADBMS_GPO8_MASK         (1U << 7)   /**< GPO8 bitmask */
#define ADBMS_GPO9_MASK         (1U << 8)   /**< GPO9 bitmask */
#define ADBMS_GPO10_MASK        (1U << 9)   /**< GPO10 bitmask */

/* Convenience defines for common GPO configurations */
#define ADBMS_GPO_ALL_PULLDOWN_OFF   0x03FFU  /**< All pull-downs disabled (default) */
#define ADBMS_GPO_ALL_PULLDOWN_ON    0x0000U  /**< All pull-downs enabled */

/* CFGAR5: status + filter configuration ------------------------------------- */
/** Snapshot state bit (bit 5 of CFGA byte 5). */
#define CFGA_SNAP_ST_POS        5U
#define CFGA_SNAP_ST_MASK       0x20U

/** Mute state bit (bit 4 of CFGA byte 5). */
#define CFGA_MUTE_ST_POS        4U
#define CFGA_MUTE_ST_MASK       0x10U

/** Communication break indication (bit 3 of CFGA byte 5). */
#define CFGA_COMM_BK_POS        3U
#define CFGA_COMM_BK_MASK       0x08U

/** Filter configuration FC[2:0] (bits [2:0] of CFGA byte 5). */
#define CFGA_FC_POS             0U
#define CFGA_FC_MASK            0x07U

/*----------------------------------------------------------------------------*/
/* Enums                                                                      */
/*----------------------------------------------------------------------------*/

/**
 * C-ADC vs S-ADC comparison voltage threshold (CTH[2:0]).
 * Table 102 in datasheet. Default is 8.1 mV.
 */
typedef enum
{
    ADBMS_CFGA_CTH_5_1MV   = 0U,  /**< 5.1 mV threshold */
    ADBMS_CFGA_CTH_8_1MV   = 1U,  /**< 8.1 mV threshold (default) */
    ADBMS_CFGA_CTH_9MV     = 2U,  /**< 9 mV threshold */
    ADBMS_CFGA_CTH_10_05MV = 3U,  /**< 10.05 mV threshold */
    ADBMS_CFGA_CTH_15MV    = 4U,  /**< 15 mV threshold */
    ADBMS_CFGA_CTH_19_95MV = 5U,  /**< 19.95 mV threshold */
    ADBMS_CFGA_CTH_25_05MV = 6U,  /**< 25.05 mV threshold */
    ADBMS_CFGA_CTH_40_05MV = 7U,  /**< 40.05 mV threshold */
} adbms_cfga_cth_t;

/**
 * IIR filter -3dB corner frequency (FC[2:0]).
 * Table 21 in datasheet. Default is filter disabled.
 */
typedef enum
{
    ADBMS_CFGA_FC_DISABLED = 0U,  /**< Filter disabled (default) */
    ADBMS_CFGA_FC_110HZ    = 1U,  /**< 110 Hz corner frequency */
    ADBMS_CFGA_FC_45HZ     = 2U,  /**< 45 Hz corner frequency */
    ADBMS_CFGA_FC_21HZ     = 3U,  /**< 21 Hz corner frequency */
    ADBMS_CFGA_FC_10HZ     = 4U,  /**< 10 Hz corner frequency */
    ADBMS_CFGA_FC_5HZ      = 5U,  /**< 5 Hz corner frequency */
    ADBMS_CFGA_FC_1_25HZ   = 6U,  /**< 1.25 Hz corner frequency */
    ADBMS_CFGA_FC_0_625HZ  = 7U,  /**< 0.625 Hz corner frequency */
} adbms_cfga_fc_t;

/**
 * Open wire soak time selection (OWA[2:0]).
 * Table 102 in datasheet. Soak time depends on OWRNG bit:
 *   OWRNG=0 (short): 2^(6+OWA) clocks -> 32us to 4.1ms
 *   OWRNG=1 (long):  2^(13+OWA) clocks -> 4.1ms to 524ms
 * Names below show short-range (OWRNG=0) times. Default is 32us.
 */
typedef enum
{
    ADBMS_CFGA_OWA_32US   = 0U,  /**< 32 µs (short) / 4.1 ms (long) (default) */
    ADBMS_CFGA_OWA_64US   = 1U,  /**< 64 µs (short) / 8.2 ms (long) */
    ADBMS_CFGA_OWA_128US  = 2U,  /**< 128 µs (short) / 16.4 ms (long) */
    ADBMS_CFGA_OWA_256US  = 3U,  /**< 256 µs (short) / 32.8 ms (long) */
    ADBMS_CFGA_OWA_512US  = 4U,  /**< 512 µs (short) / 65.5 ms (long) */
    ADBMS_CFGA_OWA_1024US = 5U,  /**< 1.024 ms (short) / 131 ms (long) */
    ADBMS_CFGA_OWA_2048US = 6U,  /**< 2.048 ms (short) / 262 ms (long) */
    ADBMS_CFGA_OWA_4096US = 7U,  /**< 4.096 ms (short) / 524 ms (long) */
} adbms_cfga_owa_t;

/**
 * GPO pin index (for use with adbms_cfga_set_gpo_pin / get_gpo_pin).
 * The ADBMS6830B has 10 GPIO pins that can be configured as outputs.
 * Polarity: bit=0 means pull-down ON, bit=1 means pull-down OFF (default).
 */
typedef enum
{
    ADBMS_GPO_PIN_1  = 1U,   /**< GPIO1 pin */
    ADBMS_GPO_PIN_2  = 2U,   /**< GPIO2 pin */
    ADBMS_GPO_PIN_3  = 3U,   /**< GPIO3 pin */
    ADBMS_GPO_PIN_4  = 4U,   /**< GPIO4 pin */
    ADBMS_GPO_PIN_5  = 5U,   /**< GPIO5 pin */
    ADBMS_GPO_PIN_6  = 6U,   /**< GPIO6 pin */
    ADBMS_GPO_PIN_7  = 7U,   /**< GPIO7 pin */
    ADBMS_GPO_PIN_8  = 8U,   /**< GPIO8 pin */
    ADBMS_GPO_PIN_9  = 9U,   /**< GPIO9 pin */
    ADBMS_GPO_PIN_10 = 10U,  /**< GPIO10 pin */
} adbms_gpo_pin_t;

/*----------------------------------------------------------------------------*/
/* Helpers (operate on raw `uint8_t cfga[ADBMS_REG_GROUP_SIZE]`)              */
/*----------------------------------------------------------------------------*/

static inline void adbms_cfga_set_cth(uint8_t * cfga,
                                      adbms_cfga_cth_t cth)
{
    cfga[0] = (uint8_t)((cfga[0] & (uint8_t)~CFGA_CTH_MASK) |
                        (((uint8_t)cth << CFGA_CTH_POS) & CFGA_CTH_MASK));
}

static inline adbms_cfga_cth_t adbms_cfga_get_cth(const uint8_t * cfga)
{
    return (adbms_cfga_cth_t)((cfga[0] & CFGA_CTH_MASK) >> CFGA_CTH_POS);
}

static inline void adbms_cfga_set_refon(uint8_t *cfga, bool enable)
{
    if (enable)
    {
        cfga[0] |= CFGA_REFON_MASK;
    }
    else
    {
        cfga[0] &= (uint8_t)~CFGA_REFON_MASK;
    }
}

static inline bool adbms_cfga_get_refon(const uint8_t *cfga)
{
    return (cfga[0] & CFGA_REFON_MASK) != 0U;
}

static inline void adbms_cfga_set_flagd(uint8_t *cfga, uint8_t flagd)
{
    cfga[1] = flagd;
}

static inline uint8_t adbms_cfga_get_flagd(const uint8_t *cfga)
{
    return cfga[1];
}

static inline void adbms_cfga_set_soakon(uint8_t *cfga, bool enable)
{
    if (enable)
    {
        cfga[2] |= CFGA_SOAKON_MASK;
    }
    else
    {
        cfga[2] &= (uint8_t)~CFGA_SOAKON_MASK;
    }
}

static inline void adbms_cfga_set_owrng(uint8_t *cfga, bool enable)
{
    if (enable)
    {
        cfga[2] |= CFGA_OWRNG_MASK;
    }
    else
    {
        cfga[2] &= (uint8_t)~CFGA_OWRNG_MASK;
    }
}

static inline void adbms_cfga_set_owa_mode(uint8_t *cfga,
                                           adbms_cfga_owa_t mode)
{
    uint8_t owa = (uint8_t)mode & (uint8_t)(CFGA_OWA_MASK >> CFGA_OWA_POS);
    cfga[2] = (uint8_t)((cfga[2] & (uint8_t)~CFGA_OWA_MASK) |
                        ((uint8_t)(owa << CFGA_OWA_POS) & CFGA_OWA_MASK));
}

static inline adbms_cfga_owa_t adbms_cfga_get_owa_mode(const uint8_t *cfga)
{
    return (adbms_cfga_owa_t)((cfga[2] & CFGA_OWA_MASK) >> CFGA_OWA_POS);
}

static inline void adbms_cfga_set_gpo(uint8_t *cfga, uint16_t gpo)
{
    gpo &= CFGA_GPO_MASK;

    /* Clear existing bits */
    cfga[3] = 0U;
    cfga[4] = (uint8_t)(cfga[4] & (uint8_t)~0x03U);

    /* Write new value: GPO[8:1] into byte 3, GPO[10:9] into bits [1:0] of byte 4 */
    cfga[3] = (uint8_t)(gpo & 0xFFU);          /* bits [7:0] -> GPO[8:1] */
    cfga[4] |= (uint8_t)((gpo >> 8) & 0x03U);  /* bits [9:8] -> GPO[10:9] */
}

static inline uint16_t adbms_cfga_get_gpo(const uint8_t *cfga)
{
    uint16_t low  = cfga[3];
    uint16_t high = (uint16_t)(cfga[4] & 0x03U);
    return (uint16_t)((high << 8) | low);
}

/**
 * @brief Set a single GPO pin pull-down state.
 *
 * @param cfga    CFGA buffer.
 * @param pin     GPO pin to configure (ADBMS_GPO_PIN_1 to ADBMS_GPO_PIN_10).
 * @param pin_state  true = floating (pulled up by external resistor), false = pull-down enabled.
 *
 * Example:
 *   adbms_cfga_set_gpo_pin(cfga, ADBMS_GPO_PIN_3, false);  // Enable pull-down on GPIO3
 */
// void adbms_cfga_set_gpo_pin(uint8_t *cfga, adbms_gpo_pin_t pin, bool pin_state)
// {
//     if (pin < ADBMS_GPO_PIN_1 || pin > ADBMS_GPO_PIN_10)
//     {
//         return;
//     }

//     uint16_t gpo = adbms_cfga_get_gpo(cfga);
//     uint16_t mask = (uint16_t)(1U << ((uint8_t)pin - 1U));

//     if (pin_state)
//     {
//         gpo |= mask;   /* Set bit = pull-down OFF */
//     }
//     else
//     {
//         gpo &= (uint16_t)~mask;  /* Clear bit = pull-down ON */
//     }

//     adbms_cfga_set_gpo(cfga, gpo);
// }

/**
 * @brief Get the pull-down state of a single GPO pin.
 *
 * @param cfga  CFGA buffer.
 * @param pin   GPO pin to query (ADBMS_GPO_PIN_1 to ADBMS_GPO_PIN_10).
 * @return true if pull-down is OFF (high-Z), false if pull-down is ON.
 */
static inline bool adbms_cfga_get_gpo_pin(const uint8_t *cfga,
                                          adbms_gpo_pin_t pin)
{
    if (pin < ADBMS_GPO_PIN_1 || pin > ADBMS_GPO_PIN_10)
    {
        return false;
    }

    uint16_t gpo = adbms_cfga_get_gpo(cfga);
    uint16_t mask = (uint16_t)(1U << ((uint8_t)pin - 1U));
    return (gpo & mask) != 0U;
}

static inline adbms_cfga_fc_t adbms_cfga_get_fc(const uint8_t *cfga)
{
    return (adbms_cfga_fc_t)((cfga[5] & CFGA_FC_MASK) >> CFGA_FC_POS);
}

static inline void adbms_cfga_set_fc(uint8_t *cfga, adbms_cfga_fc_t fc)
{
    cfga[5] = (uint8_t)((cfga[5] & (uint8_t)~CFGA_FC_MASK) |
                        (((uint8_t)fc << CFGA_FC_POS) & CFGA_FC_MASK));
}

static inline bool adbms_cfga_get_snap_st(const uint8_t *cfga)
{
    return (cfga[5] & CFGA_SNAP_ST_MASK) != 0U;
}

static inline bool adbms_cfga_get_mute_st(const uint8_t *cfga)
{
    return (cfga[5] & CFGA_MUTE_ST_MASK) != 0U;
}

static inline bool adbms_cfga_get_comm_bk(const uint8_t *cfga)
{
    return (cfga[5] & CFGA_COMM_BK_MASK) != 0U;
}

/**
 * @brief Initialize CFGA buffer to datasheet default configuration.
 *
 * Defaults from Table 102:
 *   - REFON = 0 (reference shuts down after conversions)
 *   - CTH = 8.1 mV
 *   - FLAG_D = 0
 *   - SOAKON = 0, OWRNG = 0, OWA = 0 (32 µs soak)
 *   - GPO = all pull-downs off (0x3FF)
 *   - FC = disabled
 *   - COMM_BK = 0
 */
static inline void adbms_cfga_set_defaults(uint8_t *cfga)
{
    /* Start from all zeros */
    for (uint8_t i = 0U; i < ADBMS_REG_GROUP_SIZE; i++)
    {
        cfga[i] = 0U;
    }

    /* Apply datasheet defaults */
    adbms_cfga_set_refon(cfga, false);                    /* 0 = shuts down after conversions */
    adbms_cfga_set_cth(cfga, ADBMS_CFGA_CTH_8_1MV);       /* default: 8.1 mV */
    adbms_cfga_set_flagd(cfga, 0U);
    adbms_cfga_set_soakon(cfga, false);
    adbms_cfga_set_owrng(cfga, false);
    adbms_cfga_set_owa_mode(cfga, ADBMS_CFGA_OWA_32US);   /* default: 32 µs */
    adbms_cfga_set_gpo(cfga, ADBMS_GPO_ALL_PULLDOWN_OFF); /* default: all pull-downs off */
    adbms_cfga_set_fc(cfga, ADBMS_CFGA_FC_DISABLED);      /* default: filter disabled */
}

/*============================================================================*/
/* Register group: CFGB (Configuration Register Group B)                       */
/*============================================================================*/
/* Placeholder: add CFGB commands/definitions/enums/helpers here as needed.    */

/*============================================================================*/
/* Other register groups                                                       */
/*============================================================================*/
/* Placeholder: add additional register groups (cell voltage, GPIO, status,   */
/* diagnostics, etc.) using the same Commands/Definitions/Enums/Helpers layout.*/

#endif /* DRIVERS_ADBMS6830_REGS_H_ */

