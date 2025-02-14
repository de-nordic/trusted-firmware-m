/*
 *  SPDX-License-Identifier: BSD-3-Clause
 *  SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "psa/client.h"
#include "tfm_ns_interface.h"
#include "tfm_psa_call_pack.h"
#include "tfm_mailbox.h"
#include "tfm_ns_mailbox.h"

#include "hardware/structs/sio.h"

/*
 * TODO
 * Currently, force all the non-secure client to share the same ID.
 *
 * It requires a more clear mechanism to synchronize the non-secure client
 * ID with SPE in dual core scenario.
 * In current design, the value is transferred to SPE via mailbox message.
 * A dedicated routine to receive the non-secure client information in
 * TF-M core/SPM in dual core scenario should be added besides current
 * implementation for single Armv8-M.
 * The non-secure client identification is shared with SPE in
 * single Armv8-M scenario via CMSIS TrustZone context management API,
 * which may not work in dual core scenario.
 */
#define NON_SECURE_CLIENT_ID            (-1)

/*
 * TODO
 * Require a formal definition of errors related to mailbox in PSA client call.
 */
#define PSA_INTER_CORE_COMM_ERR         (INT32_MIN + 0xFF)

/**** TZ API functions ****/

uint32_t tz_psa_framework_version(void)
{
    return tfm_ns_interface_dispatch(
                                (veneer_fn)tfm_psa_framework_version_veneer,
                                0,
                                0,
                                0,
                                0);
}

uint32_t tz_psa_version(uint32_t sid)
{
    return tfm_ns_interface_dispatch(
                                (veneer_fn)tfm_psa_version_veneer,
                                sid,
                                0,
                                0,
                                0);
}

psa_status_t tz_psa_call(psa_handle_t handle, int32_t type,
                      const psa_invec *in_vec,
                      size_t in_len,
                      psa_outvec *out_vec,
                      size_t out_len)
{
    if ((type    > PSA_CALL_TYPE_MAX) ||
        (type    < PSA_CALL_TYPE_MIN) ||
        (in_len  > PSA_MAX_IOVEC)     ||
        (out_len > PSA_MAX_IOVEC)) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return tfm_ns_interface_dispatch(
                                (veneer_fn)tfm_psa_call_veneer,
                                (uint32_t)handle,
                                PARAM_PACK(type, in_len, out_len),
                                (uint32_t)in_vec,
                                (uint32_t)out_vec);
}

psa_handle_t tz_psa_connect(uint32_t sid, uint32_t version)
{
    return tfm_ns_interface_dispatch((veneer_fn)tfm_psa_connect_veneer, sid, version, 0, 0);
}

void tz_psa_close(psa_handle_t handle)
{
    (void)tfm_ns_interface_dispatch((veneer_fn)tfm_psa_close_veneer, (uint32_t)handle, 0, 0, 0);
}

/**** Mailbox API functions ****/

uint32_t mb_psa_framework_version(void)
{
    struct psa_client_params_t params;
    uint32_t version;
    int32_t ret;

    ret = tfm_ns_mailbox_client_call(MAILBOX_PSA_FRAMEWORK_VERSION,
                                     &params, NON_SECURE_CLIENT_ID,
                                     (int32_t *)&version);
    if (ret != MAILBOX_SUCCESS) {
        version = PSA_VERSION_NONE;
    }

    return version;
}

uint32_t mb_psa_version(uint32_t sid)
{
    struct psa_client_params_t params;
    uint32_t version;
    int32_t ret;

    params.psa_version_params.sid = sid;

    ret = tfm_ns_mailbox_client_call(MAILBOX_PSA_VERSION, &params,
                                     NON_SECURE_CLIENT_ID,
                                     (int32_t *)&version);
    if (ret != MAILBOX_SUCCESS) {
        version = PSA_VERSION_NONE;
    }

    return version;
}

psa_handle_t mb_psa_connect(uint32_t sid, uint32_t version)
{
    struct psa_client_params_t params;
    psa_handle_t psa_handle;
    int32_t ret;

    params.psa_connect_params.sid = sid;
    params.psa_connect_params.version = version;

    ret = tfm_ns_mailbox_client_call(MAILBOX_PSA_CONNECT, &params,
                                     NON_SECURE_CLIENT_ID,
                                     (int32_t *)&psa_handle);
    if (ret != MAILBOX_SUCCESS) {
        psa_handle = PSA_NULL_HANDLE;
    }

    return psa_handle;
}

psa_status_t mb_psa_call(psa_handle_t handle, int32_t type,
                      const psa_invec *in_vec, size_t in_len,
                      psa_outvec *out_vec, size_t out_len)
{
    struct psa_client_params_t params;
    int32_t ret;
    psa_status_t status;

    params.psa_call_params.handle = handle;
    params.psa_call_params.type = type;
    params.psa_call_params.in_vec = in_vec;
    params.psa_call_params.in_len = in_len;
    params.psa_call_params.out_vec = out_vec;
    params.psa_call_params.out_len = out_len;

    ret = tfm_ns_mailbox_client_call(MAILBOX_PSA_CALL, &params,
                                     NON_SECURE_CLIENT_ID,
                                     (int32_t *)&status);
    if (ret != MAILBOX_SUCCESS) {
        status = PSA_INTER_CORE_COMM_ERR;
    }

    return status;
}

void mb_psa_close(psa_handle_t handle)
{
    struct psa_client_params_t params;
    int32_t reply;

    params.psa_close_params.handle = handle;

    (void)tfm_ns_mailbox_client_call(MAILBOX_PSA_CLOSE, &params,
                                     NON_SECURE_CLIENT_ID, &reply);
}

/**** API functions ****/

uint32_t psa_framework_version(void)
{
    if(sio_hw->cpuid == 0) {
        return tz_psa_framework_version();
    } else {
        return mb_psa_framework_version();
    }
}

uint32_t psa_version(uint32_t sid)
{
    if(sio_hw->cpuid == 0) {
        return tz_psa_version(sid);
    } else {
        return mb_psa_version(sid);
    }
}

psa_status_t psa_call(psa_handle_t handle, int32_t type,
                      const psa_invec *in_vec,
                      size_t in_len,
                      psa_outvec *out_vec,
                      size_t out_len)
{
    if(sio_hw->cpuid == 0) {
        return tz_psa_call(handle, type, in_vec, in_len, out_vec, out_len);
    } else {
        return mb_psa_call(handle, type, in_vec, in_len, out_vec, out_len);
    }
}

psa_handle_t psa_connect(uint32_t sid, uint32_t version)
{
    if(sio_hw->cpuid == 0) {
        return tz_psa_connect(sid, version);
    } else {
        return mb_psa_connect(sid, version);
    }
}

void psa_close(psa_handle_t handle)
{
    if(sio_hw->cpuid == 0) {
        return tz_psa_close(handle);
    } else {
        return mb_psa_close(handle);
    }
}
