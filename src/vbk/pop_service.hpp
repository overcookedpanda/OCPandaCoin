// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SRC_VBK_POP_SERVICE_HPP
#define BITCOIN_SRC_VBK_POP_SERVICE_HPP

#include "pop_common.hpp"
#include <vbk/adaptors/payloads_provider.hpp>

class BlockValidationState;
class CBlock;
class CBlockTreeDB;
class CBlockIndex;
class CDBIterator;
class CDBWrapper;
class CChainParams;

namespace Consensus {
struct Params;
}

namespace VeriBlock {

using BlockBytes = std::vector<uint8_t>;
using PoPRewards = std::map<CScript, CAmount>;

void InitPopContext(CDBWrapper& db);

CBlockIndex* compareTipToBlock(CBlockIndex* candidate);
bool acceptBlock(const CBlockIndex& indexNew, BlockValidationState& state);
bool checkPopDataSize(const altintegration::PopData& popData, altintegration::ValidationState& state);
bool addAllBlockPayloads(const CBlock& block, BlockValidationState& state);
bool setState(const uint256& block, altintegration::ValidationState& state);

PoPRewards getPopRewards(const CBlockIndex& pindexPrev, const CChainParams& params);
void addPopPayoutsIntoCoinbaseTx(CMutableTransaction& coinbaseTx, const CBlockIndex& pindexPrev, const CChainParams& params);
bool checkCoinbaseTxWithPopRewards(const CTransaction& tx, const CAmount& nFees, const CBlockIndex& pindex, const CChainParams& params, BlockValidationState& state);

std::vector<BlockBytes> getLastKnownVBKBlocks(size_t blocks);
std::vector<BlockBytes> getLastKnownBTCBlocks(size_t blocks);

//! returns true if all tips are stored in database, false otherwise
bool hasPopData(CBlockTreeDB& db);
altintegration::PopData generatePopData();
void saveTrees(CDBBatch* batch);
bool loadTrees(bool fast_load);

void removePayloadsFromMempool(const altintegration::PopData& popData);

int compareForks(const CBlockIndex& left, const CBlockIndex& right);

void addDisconnectedPopdata(const altintegration::PopData& popData);

bool isCrossedBootstrapBlock();
bool isCrossedBootstrapBlock(int32_t height);
bool isPopActive();
bool isPopActive(int32_t height);

// get stats on POP score comparisons
uint64_t getPopScoreComparisons();

CAmount GetSubsidyMultiplier(int nHeight, const CChainParams& params);

} // namespace VeriBlock

#endif // BITCOIN_SRC_VBK_POP_SERVICE_HPP
