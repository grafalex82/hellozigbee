#ifndef _PDM_IDS_H_
#define _PDM_IDS_H_

//#include "jendefs.h"
#include "stdint.h"

const uint8 PDM_ID_NODE_STATE 	= 1;
const uint8 PDM_ID_OTA_DATA 	= 2;

const uint8 PDM_ID_EP_DATA_BASE = 0x10;

inline
uint8 getPdmIdForEndpoint(uint8 ep, uint8 param)
{
    // Reserving 8 parameters per EP
    return (ep << 3) + param + PDM_ID_EP_DATA_BASE;
}



#endif //_PDM_IDS_H_
