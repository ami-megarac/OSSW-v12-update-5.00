/*
// Copyright (C) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions
// and limitations under the License.
//
//
// SPDX-License-Identifier: Apache-2.0
*/

#include "common.h"
#ifndef SPX_BMC_ACD
#include <cJSON.h>
#else
#include <dlfcn.h>
#endif
#include "SysInfo.h"

#ifdef SPX_BMC_ACD
extern void *dl_handle;
#endif

void stub_SysInfo_GetDevID(SGetDeviceIdResp *deviceId)
{
#ifndef SPX_BMC_ACD
    deviceId->u8FirmwareMajorVersion = 0x04;
    deviceId->u8FirmwareMinorVersion = 0x14;
    deviceId->u8B3AuxFwRevInfo = 0x95;
    deviceId->u8B2AuxFwRevInfo = 0x00;
    deviceId->u8B1AuxFwRevInfo = 0x00;
    deviceId->u8B0AuxFwRevInfo = 0x00;
#else
    UINT32 auxVer = 0;
    int (*GetBMCVer)(UINT8*, UINT8 *, UINT32 *);

    GetBMCVer = dlsym(dl_handle,"ACDPDK_GetBMCVersion");
    if (GetBMCVer != NULL)
    {
        GetBMCVer(&(deviceId->u8FirmwareMajorVersion), &(deviceId->u8FirmwareMinorVersion), &auxVer);
    }

    deviceId->u8B0AuxFwRevInfo = (auxVer & 0x000000ff);
    deviceId->u8B1AuxFwRevInfo = (auxVer & 0x0000ff00) >> 8;
    deviceId->u8B2AuxFwRevInfo = (auxVer & 0x00ff0000) >> 16;
    deviceId->u8B3AuxFwRevInfo = (auxVer & 0xff000000) >> 24;
#endif
}

void stub_getMeDeviceId(SGetDeviceIdResp * devId)
{
#ifndef SPX_BMC_ACD
    devId->u8FirmwareMajorVersion = 0x94;
    devId->u8FirmwareMinorVersion = 0x14;
    devId->u8B3AuxFwRevInfo = 0x00;
    devId->u8B2AuxFwRevInfo = 0xA9;
    devId->u8B1AuxFwRevInfo = 0x11;
    devId->u8B0AuxFwRevInfo = 0x00;
#else
    int (*GetMEVer)(UINT8*, UINT8 *, UINT8*, UINT8*);

    GetMEVer = dlsym(dl_handle,"ACDPDK_GetMEVersion");
    if (GetMEVer != NULL)
    {
        GetMEVer(&(devId->u8FirmwareMajorVersion), &(devId->u8FirmwareMinorVersion), &(devId->u8B1AuxFwRevInfo), &(devId->u8B2AuxFwRevInfo));
    }
#endif
}

void stub_getBiosId(UINT8 * biosId)
{
#ifndef SPX_BMC_ACD
    memcpy(biosId, "BIOSver", 8);
#else
    int (*GetBIOSVer)(UINT8*, int);

    GetBIOSVer = dlsym(dl_handle,"ACDPDK_GetBIOSVersion");
    if (GetBIOSVer != NULL)
    {
        GetBIOSVer(biosId, SI_BIOS_ID_LEN);
    }
#endif
}
