// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SRC_VBK_VBK_HPP
#define BITCOIN_SRC_VBK_VBK_HPP

#include <uint256.h>

namespace VeriBlock {

const static int32_t POP_BLOCK_VERSION_BIT = 0x80000UL;

// We want to set a limit of ATVs per VBK block by setting 2 last bytes 
// of altchain ID with specific value.
// VBK block time is 30 sec, OCPandaCoin block time is 2 min.
// OCPandaCoin maximum can contain 20 ATVs per block, so we set this limit
// to be 4.
// 0x08ff are bytes that control this behavior:

const static int64_t ALT_CHAIN_ID = 0x1119911208ff;

}  // namespace VeriBlock

#endif //BITCOIN_SRC_VBK_VBK_HPP
