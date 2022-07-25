// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/algorithm/string.hpp>
#include <boost/test/unit_test.hpp>
#include <chainparams.h>
#include <chrono>
#include <consensus/merkle.h>
#include <fstream>
#include <rpc/client.h>
#include <rpc/request.h>
#include <rpc/server.h>
#include <string>
#include <test/util/setup_common.h>
#include <thread>
#include <univalue.h>
#include <validation.h>
#include <vbk/merkle.hpp>
#include <veriblock/pop/entities/context_info_container.hpp>
#include <wallet/wallet.h>

#include <utility>
#include <vbk/test/util/e2e_fixture.hpp>

static UniValue CallRPC(std::string args)
{
    std::vector<std::string> vArgs;
    boost::split(vArgs, args, boost::is_any_of(" \t"));
    std::string strMethod = vArgs[0];
    vArgs.erase(vArgs.begin());
    JSONRPCRequest request;
    request.strMethod = strMethod;
    request.params = RPCConvertValues(strMethod, vArgs);
    request.fHelp = false;
    if (RPCIsInWarmup(nullptr)) SetRPCWarmupFinished();
    try {
        UniValue result = tableRPC.execute(request);
        return result;
    } catch (const UniValue& objError) {
        throw std::runtime_error(find_value(objError, "message").get_str());
    }
}

BOOST_AUTO_TEST_SUITE(rpc_service_tests)

BOOST_FIXTURE_TEST_CASE(getpopdata_test, E2eFixture)
{
    for (size_t i = 0; i < 20; ++i) {
        CreateAndProcessBlock({}, ChainActive().Tip()->GetBlockHash(), cbKey);
    }

    int blockHeight = ChainActive().Tip()->nHeight - 5;
    CBlockIndex* blockIndex = ChainActive()[blockHeight];
    CBlock block;

    BOOST_CHECK(ReadBlockFromDisk(block, blockIndex, Params().GetConsensus()));

    CDataStream ssBlock(SER_NETWORK, PROTOCOL_VERSION);
    ssBlock << blockIndex->GetBlockHeader();

    auto txRoot = BlockMerkleRoot(block).asVector();
    auto authedContext = altintegration::AuthenticatedContextInfoContainer::createFromPrevious(
        txRoot,
        block.popData.getMerkleRoot(),
        VeriBlock::GetAltBlockIndex(blockIndex->pprev),
        VeriBlock::GetPop().getConfig().getAltParams());
    altintegration::WriteStream w;
    authedContext.toVbkEncoding(w);

    UniValue result;
    BOOST_CHECK_NO_THROW(result = CallRPC(std::string("getpopdatabyheight ") + std::to_string(blockHeight)));

    auto blockContext = find_value(result.get_obj(), "authenticated_context").get_obj();

    BOOST_CHECK_EQUAL(find_value(blockContext, "serialized").get_str(), HexStr(w.data().begin(), w.data().end()));
    BOOST_CHECK_EQUAL(find_value(result.get_obj(), "block_header").get_str(), HexStr(ssBlock));
}

BOOST_FIXTURE_TEST_CASE(submitpop_test, E2eFixture)
{
    uint32_t generateVtbs = 20;
    std::vector<VTB> vtbs;
    vtbs.reserve(generateVtbs);
    std::generate_n(std::back_inserter(vtbs), generateVtbs, [&]() {
        return endorseVbkTip();
    });

    BOOST_CHECK_EQUAL(vtbs.size(), generateVtbs);

    std::vector<altintegration::VbkBlock> vbk_blocks;
    for (const auto& vtb : vtbs) {
        vbk_blocks.push_back(vtb.containingBlock);
    }

    BOOST_CHECK(!vbk_blocks.empty());

    for (const auto& b : vbk_blocks) {
        CallRPC(std::string("submitpopvbk ") + altintegration::SerializeToHex(b));
    }

    for (const auto& vtb : vtbs) {
        CallRPC(std::string("submitpopvtb ") + altintegration::SerializeToHex(vtb));
    }
}

BOOST_FIXTURE_TEST_CASE(extractblockinfo_test, E2eFixture)
{
    auto tip = ChainActive().Tip();
    BOOST_CHECK(tip != nullptr);

    // endorse tip
    CBlock block = endorseAltBlockAndMine(tip->GetBlockHash(), 10);

    // encode PublicationData
    BOOST_CHECK_EQUAL(block.popData.atvs.size(), 1);
    auto pubData = block.popData.atvs[0].transaction.publicationData;

    auto result = CallRPC(std::string("extractblockinfo [\"") + altintegration::SerializeToHex(pubData) + "\"]");

    // decode AuthenticatedContextInfoContainer
    altintegration::ContextInfoContainer container;
    {
        altintegration::ValidationState state;
        altintegration::ReadStream stream(pubData.contextInfo);
        BOOST_CHECK(altintegration::DeserializeFromVbkEncoding(stream, container, state));
    }

    BOOST_CHECK_EQUAL(result[0]["hash"].get_str(), tip->GetBlockHash().GetHex());
    BOOST_CHECK_EQUAL(result[0]["height"].get_int64(), tip->nHeight);
    BOOST_CHECK_EQUAL(result[0]["previousHash"].get_str(), tip->GetBlockHeader().hashPrevBlock.GetHex());
    BOOST_CHECK_EQUAL(result[0]["previousKeystone"].get_str(), uint256(container.keystones.firstPreviousKeystone).GetHex());
    BOOST_CHECK_EQUAL(result[0]["secondPreviousKeystone"].get_str(), uint256(container.keystones.secondPreviousKeystone).GetHex());
}

BOOST_FIXTURE_TEST_CASE(getblock_finalized_test, E2eFixture)
{
    auto* tip = ChainActive().Tip();
    BOOST_CHECK(tip != nullptr);

    auto finalityDistance = VeriBlock::GetPop().getAltBlockTree().getParams().getMaxReorgBlocks();
    auto settlementInterval = VeriBlock::GetPop().getAltBlockTree().getParams().preserveBlocksBehindFinal();

    for (size_t i = 0; i < (finalityDistance + settlementInterval + 5); ++i) {
        CreateAndProcessBlock({}, ChainActive().Tip()->GetBlockHash(), cbKey);
    }

    auto blockhash = CallRPC(std::string("getblockhash ") + "1");
    auto blockBeforeFinalization = CallRPC(std::string("getblock ") + blockhash.get_str());

    ::ChainstateActive().ForceFlushStateToDisk();
    CreateAndProcessBlock({}, ChainActive().Tip()->GetBlockHash(), cbKey);

    blockhash = CallRPC(std::string("getblockhash ") + "1");
    UniValue blockAfterFinalization;
    BOOST_CHECK_NO_THROW(blockAfterFinalization = CallRPC(std::string("getblock ") + blockhash.get_str()));

    BOOST_CHECK_NE(blockBeforeFinalization.write(), blockAfterFinalization.write());
}

BOOST_FIXTURE_TEST_CASE(extractblockinfo_inavlid_test, E2eFixture)
{
    CBlock block;
    bool read = ReadBlockFromDisk(block, ChainActive().Tip(), Params().GetConsensus());
    assert(read && "expected to read endorsed block from disk");

    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << ChainActive().Tip()->GetBlockHeader();
    std::vector<uint8_t> header{stream.begin(), stream.end()};

    altintegration::ContextInfoContainer container;
    container.height = 100;
    container.keystones.firstPreviousKeystone.resize(33);
    container.keystones.secondPreviousKeystone.resize(32);

    altintegration::PublicationData pubData{};
    pubData.identifier = 1000;
    pubData.header = header;
    auto serializedContext = altintegration::SerializeToVbkEncoding(container);
    pubData.contextInfo = serializedContext;

    auto result = CallRPC(std::string("extractblockinfo [\"") + altintegration::SerializeToHex(pubData) + "\"]");
    BOOST_CHECK_NE(find_value(result, "code").get_int64(), 0);

    container.keystones.firstPreviousKeystone.resize(32);
    container.keystones.secondPreviousKeystone.resize(33);

    serializedContext = altintegration::SerializeToVbkEncoding(container);
    pubData.contextInfo = serializedContext;

    result = CallRPC(std::string("extractblockinfo [\"") + altintegration::SerializeToHex(pubData) + "\"]");
    BOOST_CHECK_NE(find_value(result, "code").get_int64(), 0);
}

BOOST_FIXTURE_TEST_CASE(getpopscorestats_test, E2eFixture)
{
    auto result = CallRPC(std::string("getpopscorestats"));
    auto stats = find_value(result.get_obj(), "stats").get_obj();
    auto comparisons = find_value(stats, "popScoreComparisons").get_int64();
    CreateAndProcessBlock({}, ChainActive().Tip()->GetBlockHash(), cbKey);

    result = CallRPC(std::string("getpopscorestats"));
    stats = find_value(result.get_obj(), "stats").get_obj();
    auto comparisons_after = find_value(stats, "popScoreComparisons").get_int64();

    BOOST_CHECK_EQUAL(comparisons + 1, comparisons_after);
}

BOOST_AUTO_TEST_SUITE_END()
