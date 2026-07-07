// Copyright (c) 2024-2026 The RadhikaChain developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <consensus/karma.h>
#include <consensus/params.h>
#include <pow.h>
#include <test/util/random.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(karma_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(karma_hash_basic)
{
    std::vector<uint8_t> header_bytes(80, 0);
    KarmaCommitment commitment;
    commitment.karma_score = 0;
    commitment.prana_burned = 0;
    commitment.validator_age_blocks = 0;

    uint256 hash = KarmaHash(header_bytes, commitment);
    BOOST_CHECK(hash != uint256::ZERO);
}

BOOST_AUTO_TEST_CASE(karma_hash_different_scores)
{
    std::vector<uint8_t> header_bytes(80, 0);
    KarmaCommitment c1, c2;
    c1.karma_score = 0;
    c2.karma_score = 5000;

    uint256 hash1 = KarmaHash(header_bytes, c1);
    uint256 hash2 = KarmaHash(header_bytes, c2);

    BOOST_CHECK(hash1 != hash2);
}

BOOST_AUTO_TEST_CASE(karma_hash_different_headers)
{
    std::vector<uint8_t> header1(80, 0);
    std::vector<uint8_t> header2(80, 1);
    KarmaCommitment commitment;

    uint256 hash1 = KarmaHash(header1, commitment);
    uint256 hash2 = KarmaHash(header2, commitment);

    BOOST_CHECK(hash1 != hash2);
}

BOOST_AUTO_TEST_CASE(karma_effective_target_no_bonus)
{
    arith_uint256 base_target = UintToArith256(uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
    arith_uint256 effective = KarmaEffectiveTarget(base_target, 0);

    BOOST_CHECK(effective == base_target);
}

BOOST_AUTO_TEST_CASE(karma_effective_target_max_bonus)
{
    arith_uint256 base_target = UintToArith256(uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
    arith_uint256 effective = KarmaEffectiveTarget(base_target, KARMA_MAX_SCORE);

    BOOST_CHECK(effective > base_target);
}

BOOST_AUTO_TEST_CASE(karma_effective_target_scaling)
{
    arith_uint256 base_target = UintToArith256(uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

    arith_uint256 eff_low = KarmaEffectiveTarget(base_target, 1000);
    arith_uint256 eff_mid = KarmaEffectiveTarget(base_target, 5000);
    arith_uint256 eff_high = KarmaEffectiveTarget(base_target, 10000);

    BOOST_CHECK(eff_low > base_target);
    BOOST_CHECK(eff_mid > eff_low);
    BOOST_CHECK(eff_high > eff_mid);
}

BOOST_AUTO_TEST_CASE(is_dharma_nonce)
{
    BOOST_CHECK(IsDharmaNonce(0));
    BOOST_CHECK(IsDharmaNonce(108));
    BOOST_CHECK(IsDharmaNonce(216));
    BOOST_CHECK(IsDharmaNonce(1996090516));

    BOOST_CHECK(!IsDharmaNonce(1));
    BOOST_CHECK(!IsDharmaNonce(107));
    BOOST_CHECK(!IsDharmaNonce(109));
}

BOOST_AUTO_TEST_CASE(check_proof_of_karma_nonce_validation)
{
    std::vector<uint8_t> header_bytes(80, 0);
    KarmaCommitment commitment;
    commitment.karma_score = 0;

    const auto consensus = CreateChainParams(*m_node.args, ChainType::MAIN)->GetConsensus();

    BOOST_CHECK(!CheckProofOfKarma(header_bytes, 1, 0x1d00ffff, commitment, consensus));
    BOOST_CHECK(!CheckProofOfKarma(header_bytes, 107, 0x1d00ffff, commitment, consensus));
    BOOST_CHECK(!CheckProofOfKarma(header_bytes, 109, 0x1d00ffff, commitment, consensus));
}

BOOST_AUTO_TEST_CASE(karma_commitment_serialization)
{
    KarmaCommitment commitment;
    commitment.karma_score = 5000;
    commitment.prana_burned = 1000000;
    commitment.validator_age_blocks = 144;

    std::vector<uint8_t> bytes = commitment.ToBytes();
    BOOST_CHECK_EQUAL(bytes.size(), 16);

    uint32_t score;
    memcpy(&score, bytes.data(), 4);
    BOOST_CHECK_EQUAL(score, 5000);
}

BOOST_AUTO_TEST_CASE(karma_score_bounds)
{
    arith_uint256 base_target = UintToArith256(uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

    arith_uint256 eff_min = KarmaEffectiveTarget(base_target, 0);
    arith_uint256 eff_max = KarmaEffectiveTarget(base_target, KARMA_MAX_SCORE);
    arith_uint256 eff_over = KarmaEffectiveTarget(base_target, KARMA_MAX_SCORE + 1000);

    BOOST_CHECK(eff_max == eff_over);
}

BOOST_AUTO_TEST_SUITE_END()
