/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file tram_drv.h
 * \brief Driver for Arm TRAM.
 */

#ifndef __TRAM_DRV_H__
#define __TRAM_DRV_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief ARM TRAM device configuration structure
 */
struct tram_dev_cfg_t {
    const uint32_t base;                         /*!< TRAM base address */
};

/**
 * \brief ARM TRAM device structure
 */
struct tram_dev_t {
    const struct tram_dev_cfg_t *const cfg;       /*!< TRAM configuration */
};

void tram_set_key(struct tram_dev_t *dev, const uint32_t *key);
void tram_wipe_key(struct tram_dev_t *dev);

void tram_enable_encryption(struct tram_dev_t *dev);
void tram_disable_encryption(struct tram_dev_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* __TRAM_DRV_H__ */
