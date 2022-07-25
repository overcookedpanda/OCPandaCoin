// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "params.hpp"
#include "util.hpp"
#include <boost/algorithm/string.hpp>
#include <chainparams.h>
#include <pow.h>
#include <util/strencodings.h>
#include <util/system.h>
#include <vbk/bootstraps.h>
#include <vbk/pop_common.hpp>
#include <veriblock/pop.hpp>


namespace VeriBlock {

bool AltChainParamsBTCSQ::checkBlockHeader(const std::vector<uint8_t>& bytes, const std::vector<uint8_t>& root, altintegration::ValidationState& state) const noexcept
{
    const CChainParams& params = Params();

    try {
        // this throws
        auto header = VeriBlock::headerFromBytes(bytes);

        /* top level merkle `root` calculated by library is same as in endorsed header */
        auto actual = header.hashMerkleRoot.asVector();
        if (actual != root) {
            return state.Invalid("bad-merkle-root", strprintf("Expected %s, got %s", HexStr(root), HexStr(actual)));
        }

        /* and POW of endorsed header is valid */
        if (!CheckProofOfWork(header.GetHash(), header.nBits, params.GetConsensus())) {
            return state.Invalid("bad-pow", "Bad proof of work");
        }

        return true;
    } catch (...) {
        return state.Invalid("bad-header", "Can not parse block header");
    }
}

std::vector<uint8_t> AltChainParamsBTCSQ::getHash(const std::vector<uint8_t>& bytes) const noexcept
{
    try {
        return VeriBlock::headerFromBytes(bytes).GetHash().asVector();
    } catch (...) {
        // return empty hash, since we can't deserialize header
        return {};
    }
}

void printConfig(const altintegration::Config& config)
{
    std::string btclast = config.btc.blocks.empty() ? "<empty>" : config.btc.blocks.rbegin()->getHash().toHex();
    std::string btcfirst = config.btc.blocks.empty() ? "<empty>" : config.btc.blocks.begin()->getHash().toHex();
    std::string vbklast = config.vbk.blocks.empty() ? "<empty>" : config.vbk.blocks.rbegin()->getHash().toHex();
    std::string vbkfirst = config.vbk.blocks.empty() ? "<empty>" : config.vbk.blocks.begin()->getHash().toHex();


    assert(config.alt);

    LogPrintf(R"(Applied POP config:
 BTC:
  network     : %s
  startHeight : %d
  total blocks: %d
  first       : %s
  last        : %s

 VBK:
  network     : %s
  startHeight : %d
  total blocks: %d
  first       : %s
  last        : %s

 ALT:
  network     : %s
  block height: %d
  block hash  : %s
  chain id    : %lld
)",
        config.btc.params->networkName(), config.btc.startHeight, config.btc.blocks.size(), btcfirst, btclast,
        config.vbk.params->networkName(), config.vbk.startHeight, config.vbk.blocks.size(), vbkfirst, vbklast,

        Params().NetworkIDString(), config.alt->getBootstrapBlock().height,
        HexStr(config.alt->getBootstrapBlock().hash),
        config.alt->getIdentifier());
}

void selectPopConfig(const std::string& network, int32_t alt_window, int32_t vbk_window, int32_t btc_window)
{
    altintegration::Config popconfig;

    if (network == CBaseChainParams::MAIN) {
        auto btcparam = std::make_shared<altintegration::BtcChainParamsMain>();
        btcparam->mMaxReorgBlocks = btc_window;
        popconfig.setBTC(mainnetBTCstartHeight, mainnetBTCblocks, btcparam);
        auto vbkparam = std::make_shared<altintegration::VbkChainParamsMain>();
        vbkparam->mMaxReorgBlocks = vbk_window;
        popconfig.setVBK(mainnetVBKstartHeight, mainnetVBKblocks, vbkparam);
        auto altparam = std::make_shared<VeriBlock::AltChainParamsBTCSQ>(Params().GenesisBlock(), alt_window);
        popconfig.alt = altparam;
    } else if (network == CBaseChainParams::TESTNET) {
        auto btcparam = std::make_shared<altintegration::BtcChainParamsTest>();
        btcparam->mMaxReorgBlocks = btc_window;
        popconfig.setBTC(testnetBTCstartHeight, testnetBTCblocks, btcparam);
        auto vbkparam = std::make_shared<altintegration::VbkChainParamsTest>();
        vbkparam->mMaxReorgBlocks = vbk_window;
        popconfig.setVBK(testnetVBKstartHeight, testnetVBKblocks, vbkparam);
        auto altparam = std::make_shared<VeriBlock::AltChainParamsBTCSQ>(Params().GenesisBlock(), alt_window);
        popconfig.alt = altparam;
    } else if (network == CBaseChainParams::REGTEST) {
        auto btcparam = std::make_shared<altintegration::BtcChainParamsRegTest>();
        btcparam->mMaxReorgBlocks = btc_window;
        popconfig.setBTC(0, {}, btcparam);
        auto vbkparam = std::make_shared<altintegration::VbkChainParamsRegTest>();
        vbkparam->mMaxReorgBlocks = vbk_window;
        popconfig.setVBK(0, {}, vbkparam);
        auto altparam = std::make_shared<VeriBlock::AltChainParamsBTCSQRegTest>(Params().GenesisBlock(), alt_window);
        popconfig.alt = altparam;
    } else if (network == CBaseChainParams::DETREGTEST) {
        auto btcparam = std::make_shared<altintegration::BtcChainParamsRegTest>();
        btcparam->mMaxReorgBlocks = btc_window;
        popconfig.setBTC(0, {}, btcparam);
        auto vbkparam = std::make_shared<altintegration::VbkChainParamsRegTest>();
        vbkparam->mMaxReorgBlocks = vbk_window;
        popconfig.setVBK(0, {}, vbkparam);
        auto altparam = std::make_shared<VeriBlock::AltChainParamsBTCSQDetRegTest>();
        popconfig.alt = altparam;
    } else {
        throw std::invalid_argument("currently only supports main/test/regtest/detregtest");
    }

    VeriBlock::SetPopConfig(popconfig);
    printConfig(popconfig);
}

} // namespace VeriBlock
