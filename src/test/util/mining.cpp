// Copyright (c) 2019 The Bitcoin Core developers
// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/mining.h>

#include <chainparams.h>
#include <consensus/merkle.h>
#include <key_io.h>
#include <miner.h>
#include <pow.h>
#include <script/standard.h>
#include <validation.h>
#include <vbk/merkle.hpp>

CTxIn generatetoaddress(const std::string& address)
{
    const auto dest = DecodeDestination(address);
    assert(IsValidDestination(dest));
    const auto coinbase_script = GetScriptForDestination(dest);

    return MineBlock(coinbase_script);
}

CTxIn MineBlock(const CScript& coinbase_scriptPubKey)
{
    auto block = PrepareBlock(coinbase_scriptPubKey);

    while (!CheckProofOfWork(block->GetHash(), block->nBits, Params().GetConsensus())) {
        ++block->nNonce;
    }

    bool processed = ProcessNewBlock(Params(), block, true, nullptr);
    assert(processed);

    return CTxIn{block->vtx[0]->GetHash(), 0};
}

std::shared_ptr<CBlock> PrepareBlock(const CScript& coinbase_scriptPubKey)
{
    const auto& params = Params();
    auto block = std::make_shared<CBlock>(
        BlockAssembler{params}
            .CreateNewBlock(coinbase_scriptPubKey)
            ->block);

    LOCK(cs_main);
    block->nTime = ::ChainActive().Tip()->GetMedianTimePast() + 1;

    CBlockIndex* tip = ::ChainActive().Tip();
    assert(tip != nullptr);
    block->hashMerkleRoot = VeriBlock::TopLevelMerkleRoot(tip, *block);

    return block;
}
