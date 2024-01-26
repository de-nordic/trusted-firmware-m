/*
 * Copyright (c) 2023-2024 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "dpe_plat.h"
#include "tfm_plat_defs.h"
#include "device_definition.h"
#include "rse_kmu_slot_ids.h"
#include "tfm_crypto_defs.h"
#include "tfm_plat_defs.h"

#include "sds.h"

#define SDS_DPE_CTX_HANDLE_STRUCT_ID    (0x0000000A)

TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_DPE", "APP-ROT")
int dpe_plat_get_rot_cdi(uint8_t *buf, size_t buf_size)
{
    psa_status_t status;
    size_t key_len;

    status = psa_export_key(TFM_BUILTIN_KEY_ID_ROT_CDI, buf, buf_size,  &key_len);
    if (status != PSA_SUCCESS) {
        return -1;
    }

    return 0;
}

TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_DPE", "APP-ROT")
psa_key_id_t dpe_plat_get_root_attest_key_id(void)
{
    return TFM_BUILTIN_KEY_ID_IAK;
}

TFM_LINK_SET_RO_IN_PARTITION_SECTION("TFM_SP_DPE", "APP-ROT")
int dpe_plat_share_context_with_ap(int ctx_handle)
{
    enum tfm_plat_err_t ret;
    struct sds_structure_desc struct_desc = {
        .id = SDS_DPE_CTX_HANDLE_STRUCT_ID,
        .size = sizeof(ctx_handle),
        .payload = &ctx_handle,
        .finalize = true
    };

    ret = sds_struct_add(&struct_desc);
    if (ret != TFM_PLAT_ERR_SUCCESS) {
        return -1;
    }

    return 0;
}
