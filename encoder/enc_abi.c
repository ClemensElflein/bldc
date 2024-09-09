/*
	Copyright 2016 - 2022 Benjamin Vedder	benjamin@vedder.se
	Copyright 2022 Jakub Tomczak

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "encoder/enc_abi.h"

#include "ch.h"
#include "hal.h"
#include "stm32f4xx_conf.h"
#include "hw.h"
#include "mc_interface.h"
#include "utils_math.h"

#include <string.h>
#include <math.h>

volatile int steps = 0;
volatile int steps_absolute = 0;

bool enc_abi_init(ABI_config_t *cfg) {
    steps = 0;
    steps_absolute = 0;
	EXTI_InitTypeDef EXTI_InitStructure;

	memset(&cfg->state, 0, sizeof(ABI_state));

	palSetPadMode(cfg->I_gpio, cfg->I_pin, PAL_MODE_INPUT_PULLUP);

	// Connect EXTI Line to pin
	SYSCFG_EXTILineConfig(cfg->exti_portsrc, cfg->exti_pinsrc);

	// Configure EXTI Line
	EXTI_InitStructure.EXTI_Line = cfg->exti_line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set EXTI Line Interrupt to the highest priority
	nvicEnableVector(cfg->exti_ch, 0);

	return true;
}

void enc_abi_deinit(ABI_config_t *cfg) {
	nvicDisableVector(cfg->exti_ch);
	palSetPadMode(cfg->I_gpio, cfg->I_pin, PAL_MODE_INPUT_PULLUP);
}

float enc_abi_read_deg(ABI_config_t *cfg) {
    return 0;
}

void enc_abi_pin_isr(ABI_config_t *cfg) {
	// Only reset if the pin is still high to avoid too short pulses, which
	// most likely are noise.
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	if (palReadPad(cfg->I_gpio, cfg->I_pin)) {
        if(mc_interface_get_duty_cycle_now() > 0) {
            steps++;
        } else {
            steps--;
        }
        steps_absolute++;
        mc_interface_set_tachometer_value(steps, steps_absolute);
	}
}
