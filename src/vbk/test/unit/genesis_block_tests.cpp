// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#include "vbk/genesis.hpp"
#include <chainparams.h>
#include <consensus/validation.h>
#include <validation.h>

BOOST_AUTO_TEST_SUITE(VBK_GenesisBlock)
struct GenesisBlockFixture {
    GenesisBlockFixture() = default;

    static void init(const std::string& name)
    {
        SelectParams(name);
    }

    static void check(const std::string& name)
    {
        init(name);
        auto& params = Params();
        const CBlock& block = params.GenesisBlock();
        BlockValidationState state;
        bool result = CheckBlock(block, state, params.GetConsensus(), true);
        BOOST_CHECK(result);
        if (!result) {
            printf("State: %s\n", state.GetDebugMessage().c_str());
        }
    }
    std::string initialPubkey = "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff5004ffff001d0104484f766572636f6f6b65642050616e646120666f726b732056424b207265666572656e636520666f7220506f5020656e61626c656420636f696e73202d2046656220312c2032303232ffffffff0100f2052a010000001600143dd5f2f667315cc98e669deb88d3dfe831aa2cea00000000";
    std::string pszTimestamp = "Overcooked Panda forks VBK reference for PoP enabled coins - July 28, 2022";
};

BOOST_FIXTURE_TEST_CASE(main, GenesisBlockFixture)
{
//        // Do the actual block mining
//        {
//            init("main");
//
//            CBlock block = VeriBlock::MineGenesisBlock(
//                1658962780,
//                pszTimestamp,
//                initialPubkey,
//                0x1d00ffff,
//                1,
//                0,
//                50 * COIN);
//
//            printf("%s\n", block.ToString().c_str());
//        }

    check("main");
}

BOOST_FIXTURE_TEST_CASE(test, GenesisBlockFixture)
{
//        // Do the actual block mining
//        {
//            init("test");
//
//            CBlock block = VeriBlock::MineGenesisBlock(
//                1340,
//                pszTimestamp,
//                initialPubkey,
//                0x1d1fffff, // 30 sec on macbook pro 2013
//                1,
//                0,
//                50 * COIN);
//
//            printf("%s\n", block.ToString().c_str());
//        }

    check("test");
}

BOOST_FIXTURE_TEST_CASE(regtest, GenesisBlockFixture)
{
    //    // Do the actual block mining
    //    {
    //        init("regtest");
    //
    //        CBlock block = VeriBlock::MineGenesisBlock(
    //            1337,
    //            pszTimestamp,
    //            initialPubkey,
    //            0x207fffff,
    //            1, // version
    //            0, // starting nonce
    //            50 * COIN);
    //
    //        printf("BLOCK:\n%s\n", block.ToString().c_str());
    //    }

    check("regtest");
    check("detregtest");
}


BOOST_AUTO_TEST_SUITE_END()
