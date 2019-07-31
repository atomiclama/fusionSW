/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "rx_spi.h"


typedef struct {
    uint8_t band;       // 1=A, 2=B, 3=E, 4=F(Airwaves/Fatshark), 5=Raceband
    uint8_t channel;    // 1-8
    uint8_t power;      // 0 = lowest
} olrsConfig_t_temp;

#define MAX_CHANNEL      24

typedef struct {
    uint8_t  version;
    uint32_t rf_frequency;
    uint32_t rf_magic;
    uint8_t  rf_power;
    uint8_t  rf_channel_spacing;
    uint8_t  rf_channel[MAX_CHANNEL];
    uint8_t  modem_params;
    uint8_t  flags;
} olrsConfig_t;

PG_DECLARE(olrsConfig_t, olrsConfig);

extern olrsConfig_t bind_data;

struct rxConfig_s;
struct rxRuntimeConfig_s;
bool openlrsInit(const struct rxConfig_s *rxConfig, struct rxRuntimeConfig_s *rxRuntimeConfig) ;
void openlrsSetRcDataFromPayload(uint16_t *rcData, const uint8_t *payload);
rx_spi_received_e openlrsDataReceived(uint8_t *payload);


