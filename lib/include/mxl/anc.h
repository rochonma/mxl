#pragma once

#include <mxl/mxl.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// ANC Grain memory layout. Based on RFC8331, Section 2.1
//
// Unless specified, memory ordering is little endian.
//
// -- Header --
//
// ANC_Count          8 bits
// F                  2 bits
// Reserved          22 bits
//
// -- Followed by 0 or many ANC Packet (according to ANC_Count) --
//
// C                  1 bit
// Line_Number       11 bits
// Horizontal_Offset 12 bits
// S                  1 bit
// Stream_Num         7 bits
// DID               10 bits
// SDID              10 bits
// DataCount          8 bits (no need for 2 extra parity bits of checksum for that matter)
// User_Data_Words : integer number of 10 - bit words
// word_aligned     (enough bits to end on a 32 bits boundary)

#pragma pack( push, 1 )

typedef struct AncGrainHeader
{
    uint8_t ANC_Count;
    uint8_t F : 2;
    uint32_t Reserved : 22;
} AncGrainHeader;

typedef struct AncPacket
{
    uint8_t C : 1;
    uint16_t Line_Number : 11;
    uint16_t Horizontal_Offset : 12;
    uint8_t S : 1;
    uint8_t Stream_Num : 7;
    uint16_t DID : 10;
    uint16_t SDID : 10;
    uint16_t DataCount : 10;
} AncPacket;

#pragma pack( pop )

#ifdef __cplusplus
}
#endif