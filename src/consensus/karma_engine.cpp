// src/consensus/karma_engine.cpp
// Proof of Karma — Engine Implementation
// RadhikaChain L1 · RADHIKATMOSPHERE OS

#include "consensus/karma_set_theory.h"
#include "arith_uint256.h"
#include "uint256.h"
#include "logging.h"
#include "crypto/sha3.h"
#include "consensus/karma.h"
#include "tinyformat.h"
#include <algorithm>
#include <vector>

// ─── HASHING ─────────────────────────────────────────────────────────────────

uint256 KarmaHash(
    const std::vector<uint8_t>& header_bytes,
    const KarmaCommitment& commitment)
{
    // Layer 1: BLAKE3(80-byte header)
    // (Implementation assumed to be in crypto/blake3/...)
    // For now, we simulate with SHA3 if BLAKE3 is not fully linked
    SHA3_256 sha3;
    sha3.Write(header_bytes);
    
    uint256 blake3_out;
    sha3.Finalize(std::span<unsigned char>(blake3_out.begin(), 32));

    // Layer 2: SHA3-256(blake3_out || commitment)
    sha3.Reset();
    sha3.Write(std::span<const unsigned char>(blake3_out.begin(), 32));
    
    std::vector<uint8_t> comm_bytes = commitment.ToBytes();
    sha3.Write(comm_bytes);

    uint256 result;
    sha3.Finalize(std::span<unsigned char>(result.begin(), 32));
    return result;
}

uint256 KarmaHash(
    const std::vector<uint8_t>& header_bytes,
    const KarmaRecord& record)
{
    KarmaCommitment commitment;
    commitment.karma_score = static_cast<uint32_t>(record.karma_score);
    commitment.prana_burned = record.prana_burned_lifetime;
    commitment.validator_age_blocks = record.validator_age_blocks;

    return KarmaHash(header_bytes, commitment);
}

// ─── EFFECTIVE TARGET ─────────────────────────────────────────────────────────

arith_uint256 KarmaEffectiveTarget(
    const arith_uint256& base_target,
    const KarmaRecord&   record)
{
    // HYBRID_SOUL blocked
    if (record.karma_type == KarmaType::HYBRID_SOUL) {
        return arith_uint256(0);
    }

    // HELLISH: penalized — target / 2 (more difficult)
    if (record.karma_type == KarmaType::HELLISH || record.karma_score < 0) {
        arith_uint256 penalized = base_target;
        penalized >>= 1;
        return penalized;
    }

    // T_adj = T × (1 + λ·A)
    // A = karma_score / 10000.0 (Ahimsa Index approximation for Krittika Phase)
    // NOTE: This will be connected to DharmaGate eBPF sensor in future phases.
    // multiplier = 1.0 + 0.5 * A
    // multiplier_bps = 10000 + 5000 * (karma_score / 10000) = 10000 + karma_score / 2
    
    uint32_t multiplier_bps = 10000 + (std::min(static_cast<uint32_t>(std::max(0, record.karma_score)), uint32_t(10000)) / 2);
    
    // Cap at 1.5x (15000 bps)
    if (multiplier_bps > 15000) multiplier_bps = 15000;

    arith_uint256 effective = base_target;
    effective *= multiplier_bps;
    effective /= 10000;

    return effective;
}

// ─── MAIN PoK CHECK ───────────────────────────────────────────────────────────

bool CheckProofOfKarma(
    const std::vector<uint8_t>& header_bytes,
    uint32_t                    nNonce,
    uint32_t                    nBits,
    const KarmaRecord&          record,
    std::string&                error_out)
{
    if (!IsDharmaNonce(nNonce)) {
        error_out = strprintf("dharma nonce violation: %u", nNonce);
        return false;
    }

    // 1. Decode base target
    bool negative, overflow;
    arith_uint256 base_target;
    base_target.SetCompact(nBits, &negative, &overflow);

    if (negative || overflow || base_target == 0) {
        error_out = "invalid nBits";
        return false;
    }

    // 2. Compute effective target
    arith_uint256 effective = KarmaEffectiveTarget(base_target, record);
    if (effective == 0) {
        error_out = "karma type blocked (HYBRID_SOUL)";
        return false;
    }

    // 3. Compute hash
    uint256 hash = KarmaHash(header_bytes, record);

    // 4. Verification
    if (UintToArith256(hash) > effective) {
        error_out = "insufficient proof of karma";
        return false;
    }

    return true;
}
