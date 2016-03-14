#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

/** @brief Statistics header copy 1 address**/
#define HDR_ADDR1 0x0060
/** @brief Statistics header copy 2 address**/
#define HDR_ADDR2 0x2FE0
/** @brief Statistics header copy 3 address**/
#define HDR_ADDR3 0x3FE0
/** @brief Start of Global statistics area address**/
#define GLOB_ADDR 0x0080
/** @brief End of Global statistics area address (past-the-end ptr)**/
#define GLOB_END_ADDR (GLOB_ADDR + GLOB_NUM*RECSIZE)
/** @brief Start of Operative statistics area address**/
#define OPER_ADDR 0x3000
/** @brief End of Operative statistics area address (past-the-end ptr)**/
#define OPER_END_ADDR (OPER_ADDR + OPER_NUM*RECSIZE)

/** @brief Number of Global records**/
#define GLOB_NUM 379
/** @brief Number of immutable Global records (they will be written only once) **/
#define GLOB_IMM_NUM 32
/** @brief Number of Operative records**/
#define OPER_NUM 127
/** @brief Number of immutable Operative records (they will be written only once) **/
#define OPER_IMM_NUM 16
/** @brief Size of statistics record, bytes**/
#define RECSIZE 32
/** @brief Size of ptr group in statistics header, ptrs**/
#define PTRNUM 3

/** @brief Statistics header signature**/
#define HDR_SIGN 0xCA97E653
/** @brief Global record signature**/
#define GLOB_SIGN 0xCAE6
/** @brief Operative record signature**/
#define OPER_SIGN 0x9753

/** @brief Statistics header, holds end pointers*/
struct __attribute__((__packed__, aligned(1))) stat_header {
	uint32_t signature; /**< Signature*/
	uint16_t globend[PTRNUM]; /**< Global area last record pointer*/
	uint16_t operend[PTRNUM]; /**< Operative area last record pointer*/
	uint32_t nandaddr[PTRNUM]; /**< Reserved (pad)*/
	uint8_t rebootcnt; /** Soft reboots counter*/
	uint8_t resetcnt; /**< Hard resets counter*/
	uint16_t __resv2; /**< Reserved (pad)*/
};

/** @brief Global record**/
struct __attribute__((__packed__, aligned(1))) stat_glob_rec  {
	uint16_t signature; /**< Signature*/
	uint16_t status; /**< Copy of STATUS register, with boot flags OR'd in*/
	uint32_t ctrl; /**< Copy of CTRL register*/
	uint32_t timer; /**< Copy of TIMER register*/
	uint32_t errc; /**< Error code (implementation-defined)*/
	uint32_t link; /**< LR (link register) contents: from where it was called*/
	uint32_t stack; /**< SP (stack pointer) contents: how much memory is used for stack*/
	uint32_t data1; /**< Error-specific data*/
	uint32_t data2; /**< Error-specific data*/
};

/** @brief Operative record**/
struct __attribute__((__packed__, aligned(1))) stat_oper_rec {
	uint16_t signature; /**< Signature*/
	uint16_t __resv0; /**< Reserved (pad)*/
	uint32_t timer; /**< Copy of TIMER register*/
	uint32_t frm_captured; /**< How many frames are captured*/
	uint32_t frm_saved; /**< How many frames are actually saved*/
	uint32_t nandaddr; /**< Address of last written nand page*/
	uint32_t __resv4; /**< Reserved (pad)*/
	uint32_t __resv5; /**< Reserved (pad)*/
	uint32_t __resv6; /**< Reserved (pad)*/
};

enum class stat_err{
	GOOD, /**< All good */
	EBAD, /**< Something's bad*/
	EBADHSIGN, /**< Bad header signature */
	EBADGLPTR, /**< Bad Global area pointer*/
	EBADOPPTR, /**< Bad Operative area pointer*/
	EBADGLHDR, /**< Bad Global record signature*/
	EBADOPHDR, /**< Bad Operative record signature*/
	EBADMDHDR, /**< Bad Mode record signature*/
	EBADMDCHS, /**< Bad Mode record checksum*/
	ENOMODFND, /**< Can't find any mode, using previous (or default)*/
	ENXMODFND  /**< Can't find next mode, proceeding forward*/
};

#endif // MAIN_HPP_INCLUDED
