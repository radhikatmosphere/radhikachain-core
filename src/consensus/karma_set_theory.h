#include <algorithm>
// src/consensus/karma_set_theory.h
// Proof of Karma — Mathematical Foundation
// Based on: Jargal Dorj, "Mathematical Proof of the Law of Karma"
// American Journal of Applied Mathematics, Vol. 2, No. 4, 2014, pp. 111-126
// doi: 10.11648/j.ajam.20140204.12
//
// RADHIKATMOSPHERE OS / A.L.I.C.E. · Krittika Phase
//
// ══════════════════════════════════════════════════════════════════════════════
// SET-THEORETIC MODEL (from the paper, Section 3)
// ══════════════════════════════════════════════════════════════════════════════
//
// The paper defines living creatures as imperfect clusters of elements using
// Set Theory, where:
//
//   Body  (B) = {∅, 1, 2}  for animals   |  {1, 2, 3}  for humans
//   Mind  (M) = {∅, 3, 4}  for animals   |  {2, 3, 4}  for humans
//
//   ∅ = the "empty element" — spiritual/empty character (cardinality 1)
//
// Union:        B ∪ M  = the full being
// Intersection: B ∩ M  = the "possession" relationship (body owns mind / mind owns body)
//
// The 5 human types are distinguished by the properties of B ∩ M:
//
//   TYPE 1 — HOLY (Enlightened):  B ∩ M = ∅   (empty intersection = no possession)
//   TYPE 2 — BOUNTEOUS BODY:     B ∩ M ≠ ∅,  body owns mind partially
//   TYPE 3 — GENEROUS MIND:      B ∩ M ≠ ∅,  mind owns body partially
//   TYPE 4 — ORDINARY:           B ∩ M ≠ ∅,  mutual ownership
//   TYPE 5 — FOOLISH:            B ∩ M ≠ ∅,  minimum intersection (crude soul)
//
// ══════════════════════════════════════════════════════════════════════════════
// MAPPING TO BLOCKCHAIN CONSENSUS
// ══════════════════════════════════════════════════════════════════════════════
//
// A validator's "karma type" is determined by the intersection of their
// on-chain activity vector (body_set) and intent record (mind_set).
//
//   body_set  = { PRANA_burned, blocks_validated, uptime_score }
//   mind_set  = { dharma_actions, community_txs, oracle_submissions }
//
// The intersection B ∩ M represents the validator's realized karma:
//   karma_type  = classify(|B ∩ M|, empty_element_present)
//   karma_score = KarmaScoreFromType(karma_type) + accumulation_bonus
//
// ══════════════════════════════════════════════════════════════════════════════
// THE 10 DHARMA ACTS (from Section 1 of the paper)
// ══════════════════════════════════════════════════════════════════════════════
//
// 10 Black Sins (negative karma, reduce score):
//   Bodily:  Kill, Steal, Improper conduct
//   Verbal:  Lie, Disparage, Excess, Gossip
//   Mental:  Greed, Hate, Negative mind
//
// 10 White Blessings (positive karma, increase score):
//   Bodily:  Protect life, Give freely, Proper conduct
//   Verbal:  Truth, Praise, Moderation, Silence
//   Mental:  Generosity, Compassion, Positive mind
//
// ══════════════════════════════════════════════════════════════════════════════
// SOUL TRANSFER → REINCARNATION PROTOCOL
// ══════════════════════════════════════════════════════════════════════════════
//
// The paper proves (Section 5, Theorem) that after death (= transaction finality),
// the soul (= UTXO karma record) is transferred to one of 6 manifestations:
//
//   Holy     → Nirvana (∅) | God's Place | Heaven | Asura | Human | Animal
//   Ordinary → Human | Animal
//   Foolish  → Low-intelligence Human | Hellish Animal | Hybrid Soul
//
// In blockchain terms: a validator's karma_record is NOT destroyed on wallet change —
// it propagates to the new address according to the karma_type rules.
// This implements "karma continuity" across wallet rotations.

#pragma once

#include <cstdint>
#include <array>
#include <set>
#include <string>
#include "arith_uint256.h"

// ─── EMPTY ELEMENT ∅ ─────────────────────────────────────────────────────────
// From the paper (Section 2):
// "An empty element is a special empty set that contains empty set
//  and with cardinality of 1."
//
// In our implementation: the empty element ∅ represents the "spiritual"
// dimension of a validator — their capacity to act without self-interest.
// A validator possesses ∅ if they have performed holy actions without
// expecting reward (KARMA_ACTION_SELFLESS).

static constexpr uint8_t EMPTY_ELEMENT = 0xFF; // ∅ sentinel in set encoding

// ─── VALIDATOR SET ENCODING ───────────────────────────────────────────────────
//
// We encode body_set and mind_set as bitmasks over 5 dimensions:
//   Bit 0 = element 1 (PRANA burned lifetime > threshold)
//   Bit 1 = element 2 (blocks validated > threshold)
//   Bit 2 = element 3 (uptime score > threshold)
//   Bit 3 = element 4 (community actions > threshold)
//   Bit 4 = ∅ present (selfless actions confirmed)
//
// Human body_set  = {1, 2, 3}     → bits 0,1,2     = 0b00111 = 0x07
// Human mind_set  = {2, 3, 4}     → bits 1,2,3     = 0b01110 = 0x0E
// Animal body_set = {∅, 1, 2}    → bits 4,0,1     = 0b10011 = 0x13
// Animal mind_set = {∅, 3, 4}    → bits 4,2,3     = 0b11100 = 0x1C

using KarmaSet = uint8_t; // bitmask

static constexpr KarmaSet HUMAN_BODY_SET  = 0x07; // {1, 2, 3}
static constexpr KarmaSet HUMAN_MIND_SET  = 0x0E; // {2, 3, 4}
static constexpr KarmaSet ANIMAL_BODY_SET = 0x13; // {∅, 1, 2}
static constexpr KarmaSet ANIMAL_MIND_SET = 0x1C; // {∅, 3, 4}
static constexpr KarmaSet EMPTY_BIT       = 0x10; // bit 4 = ∅ present

// Set operations on KarmaSet
inline KarmaSet SetUnion(KarmaSet A, KarmaSet B)        { return A | B; }
inline KarmaSet SetIntersection(KarmaSet A, KarmaSet B) { return A & B; }
inline bool     HasEmptyElement(KarmaSet S)             { return (S & EMPTY_BIT) != 0; }
inline bool     IsEmptyIntersection(KarmaSet inter)     { return (inter & ~EMPTY_BIT) == 0; }
inline uint32_t IntersectionSize(KarmaSet inter)        {
    // Count non-∅ bits
    uint8_t x = inter & ~EMPTY_BIT;
    uint32_t count = 0;
    while (x) { count += (x & 1); x >>= 1; }
    return count;
}

// ─── KARMA TYPES ─────────────────────────────────────────────────────────────
// The 5 human types from the paper, Section 3.
// Plus ANIMAL and HYBRID for the 6 living manifestations (Section 5).

enum class KarmaType : uint8_t {
    HOLY            = 0, // B ∩ M = ∅ (empty intersection, empty element present)
                         // "One who has achieved the emptiness of the Self"
                         // → MAX karma bonus in PoK
    GENEROUS_MIND   = 1, // Mind owns body partially; ∩ = {3} — shares knowledge freely
                         // → HIGH karma bonus
    BOUNTEOUS_BODY  = 2, // Body owns mind partially; ∩ = {2} — acts generously
                         // → HIGH karma bonus
    ORDINARY        = 3, // Mutual ownership; ∩ = {2, 3} — average human
                         // → STANDARD karma
    FOOLISH         = 4, // Minimum useful intersection; crude soul
                         // → REDUCED karma (but not zero — still human)
    ANIMAL          = 5, // B ∩ M = ∅ (animal-style empty) — primitive validator
                         // → MINIMAL karma
    HYBRID_SOUL     = 6, // ∩ = {3, 4} — "khainag" type: cannot find embodiment
                         // → BLOCKED: validator cannot produce blocks until karma resolved
    HELLISH         = 7, // Born from foolish soul transfer; low intelligence
                         // → PENALIZED karma
};

// Paper Section 5, Theorem: 6 living manifestations (reincarnation destinations)
enum class KarmaDestination : uint8_t {
    NIRVANA         = 0, // ∅ → Eternal Universe, no rebirth needed (validator retired)
    GODS_PLACE      = 1, // Holy soul → Paradise (highest staking tier)
    HEAVENS_PLACE   = 2, // Generous/Holy → Heaven (high staking tier)
    ASURA_HEAVEN    = 3, // Animals from Asura Heaven (mid tier)
    HUMAN           = 4, // Rebirth as human (standard)
    ANIMAL          = 5, // Rebirth as animal (primitive)
};

// ─── DHARMA ACTS — 10 SINS / 10 BLESSINGS ────────────────────────────────────

enum class DharmaAct : uint8_t {
    // === BLACK SINS (reduce karma) ===
    // Bodily sins
    SIN_KILL          = 0x01, // double-spending attack detected
    SIN_STEAL         = 0x02, // invalid UTXO claim
    SIN_IMPROPER      = 0x03, // invalid signature on community tx
    // Verbal sins
    SIN_LIE           = 0x04, // false oracle submission
    SIN_DISPARAGE     = 0x05, // submitting invalid block to punish other validator
    SIN_EXCESS        = 0x06, // fee manipulation (excessive fee extraction)
    SIN_GOSSIP        = 0x07, // propagating invalid transactions (spam)
    // Mental sins
    SIN_GREED         = 0x08, // hoarding PRANA without burning (above threshold)
    SIN_HATE          = 0x09, // consistent anti-validator targeting
    SIN_NEGATIVE_MIND = 0x0A, // consistently producing minimum-size blocks

    // === WHITE BLESSINGS (increase karma) ===
    // Bodily blessings
    BLESS_PROTECT     = 0x11, // validating transactions that protect community funds
    BLESS_GIVE        = 0x12, // PRANA burn to BURN_ADDRESS (selfless contribution)
    BLESS_PROPER      = 0x13, // clean signature record (zero invalid sigs)
    // Verbal blessings
    BLESS_TRUTH       = 0x14, // accurate oracle price submission (within 1% of median)
    BLESS_PRAISE      = 0x15, // including community OP_RETURN messages
    BLESS_MODERATION  = 0x16, // fair fee (within 10% of median)
    BLESS_SILENCE     = 0x17, // not gossiping (zero invalid tx propagations)
    // Mental blessings
    BLESS_GENEROSITY  = 0x18, // donating to community address
    BLESS_COMPASSION  = 0x19, // processing zero-fee community transactions
    BLESS_POSITIVE    = 0x1A, // consistently full blocks, high throughput
};

// karma delta for each act (applied to running karma_score)
inline int32_t KarmaDeltaForAct(DharmaAct act) {
    switch (act) {
        // SINS — negative karma (proportional to severity)
        case DharmaAct::SIN_KILL:          return -500; // severe: double-spend
        case DharmaAct::SIN_STEAL:         return -300; // severe: invalid UTXO
        case DharmaAct::SIN_IMPROPER:      return -200;
        case DharmaAct::SIN_LIE:           return -400; // oracle lies are dangerous
        case DharmaAct::SIN_DISPARAGE:     return -150;
        case DharmaAct::SIN_EXCESS:        return -100;
        case DharmaAct::SIN_GOSSIP:        return  -50;
        case DharmaAct::SIN_GREED:         return  -75;
        case DharmaAct::SIN_HATE:          return -200;
        case DharmaAct::SIN_NEGATIVE_MIND: return  -25;
        // BLESSINGS — positive karma
        case DharmaAct::BLESS_PROTECT:     return  +50;
        case DharmaAct::BLESS_GIVE:        return +200; // PRANA burn = highest blessing
        case DharmaAct::BLESS_PROPER:      return  +75;
        case DharmaAct::BLESS_TRUTH:       return +100;
        case DharmaAct::BLESS_PRAISE:      return  +25;
        case DharmaAct::BLESS_MODERATION:  return  +50;
        case DharmaAct::BLESS_SILENCE:     return  +30;
        case DharmaAct::BLESS_GENEROSITY:  return +150;
        case DharmaAct::BLESS_COMPASSION:  return  +75;
        case DharmaAct::BLESS_POSITIVE:    return  +40;
        default:                            return    0;
    }
}

// ─── KARMA TYPE CLASSIFIER ────────────────────────────────────────────────────
//
// From the paper's set-theoretic definitions (Section 3):
//
//   HOLY:           B ∩ M = ∅ (cardinality 0 after removing ∅ element)
//   GENEROUS_MIND:  ∩ = {3}     → IntersectionSize = 1, mind-dominant
//   BOUNTEOUS_BODY: ∩ = {2}     → IntersectionSize = 1, body-dominant
//   ORDINARY:       ∩ = {2, 3}  → IntersectionSize = 2, mutual
//   FOOLISH:        ∩ = {2, 3} but ∅ element absent AND low accumulation

inline KarmaType ClassifyKarmaType(
    KarmaSet body_set,
    KarmaSet mind_set,
    uint64_t prana_burned,
    uint32_t dharma_blessings,
    uint32_t dharma_sins)
{
    KarmaSet inter       = SetIntersection(body_set, mind_set);
    uint32_t inter_size  = IntersectionSize(inter);
    bool     has_empty   = HasEmptyElement(body_set) && HasEmptyElement(mind_set);
    bool     is_empty_i  = IsEmptyIntersection(inter);

    // HOLY: intersection is empty (paper: "B ∩ M = ∅")
    // Requires: empty element present AND no material intersection
    if (is_empty_i && has_empty && prana_burned > 0 && dharma_sins == 0) {
        return KarmaType::HOLY;
    }

    // HYBRID SOUL: intersection = {3, 4} type — cannot find embodiment
    // Paper: "khainag soul — unable to find its body for rebirth"
    if ((inter & 0x0C) == 0x0C && !has_empty) {
        return KarmaType::HYBRID_SOUL;
    }

    // HELLISH: foolish soul transferred to animal or degraded human
    // Triggered by excessive sins overwhelming any blessings
    if (dharma_sins > dharma_blessings * 3 && prana_burned == 0) {
        return KarmaType::HELLISH;
    }

    if (inter_size == 0 && !has_empty) {
        return KarmaType::ANIMAL; // primitive animal: no intersection
    }

    if (inter_size == 1) {
        // Determine dominant direction from the body/mind balance
        bool mind_dominant = (inter & HUMAN_MIND_SET) > (inter & HUMAN_BODY_SET);
        return mind_dominant ? KarmaType::GENEROUS_MIND : KarmaType::BOUNTEOUS_BODY;
    }

    if (inter_size == 2) {
        // Check if foolish: has sins and no PRANA burned (no selfless acts)
        bool is_foolish = (dharma_sins > 0) && (prana_burned == 0) && (dharma_blessings < 3);
        return is_foolish ? KarmaType::FOOLISH : KarmaType::ORDINARY;
    }

    return KarmaType::ORDINARY; // default: ordinary human
}

// ─── KARMA SCORE FROM TYPE ────────────────────────────────────────────────────
//
// Paper conclusion (Section 5): souls at higher levels have easier access to
// higher rebirth. We translate this as: higher karma type → more block bonus.
//
//   HOLY          → 10000 base (max — approaching Nirvana)
//   GENEROUS_MIND → 7500  (Heaven's Place tier)
//   BOUNTEOUS_BODY→ 7500  (Heaven's Place tier)
//   ORDINARY      → 5000  (Human rebirth tier)
//   FOOLISH       → 2500  (Low-intelligence human tier)
//   ANIMAL        → 1000  (Primitive animal tier)
//   HYBRID_SOUL   → 0     (Cannot find embodiment — BLOCKED)
//   HELLISH       → -100  (Hellish animal — penalized target)

inline int32_t BaseScoreForType(KarmaType t) {
    switch (t) {
        case KarmaType::HOLY:           return 10000;
        case KarmaType::GENEROUS_MIND:  return  7500;
        case KarmaType::BOUNTEOUS_BODY: return  7500;
        case KarmaType::ORDINARY:       return  5000;
        case KarmaType::FOOLISH:        return  2500;
        case KarmaType::ANIMAL:         return  1000;
        case KarmaType::HYBRID_SOUL:    return     0; // BLOCKED
        case KarmaType::HELLISH:        return  -100; // PENALIZED
        default:                        return  5000;
    }
}

// ─── REINCARNATION DESTINATION ───────────────────────────────────────────────
//
// Paper Section 5 Theorem: after death, soul transfers to one of 6 manifestations.
// In blockchain: when a validator retires a wallet, their karma_record propagates.

inline KarmaDestination GetReincarnationDestination(
    KarmaType type,
    int32_t   final_karma_score)
{
    // HOLY souls: can achieve Nirvana, God's Place, or Heaven
    if (type == KarmaType::HOLY) {
        if (final_karma_score >= 9500) return KarmaDestination::NIRVANA;
        if (final_karma_score >= 8000) return KarmaDestination::GODS_PLACE;
        return KarmaDestination::HEAVENS_PLACE;
    }
    // Generous/Bounteous → Heaven or Asura Heaven
    if (type == KarmaType::GENEROUS_MIND || type == KarmaType::BOUNTEOUS_BODY) {
        return (final_karma_score >= 6000)
            ? KarmaDestination::HEAVENS_PLACE
            : KarmaDestination::ASURA_HEAVEN;
    }
    // Ordinary/Foolish → Human rebirth
    if (type == KarmaType::ORDINARY || type == KarmaType::FOOLISH) {
        return KarmaDestination::HUMAN;
    }
    // Animal/Hellish/Hybrid → Animal rebirth
    return KarmaDestination::ANIMAL;
}

// ─── FULL KARMA COMPUTATION ───────────────────────────────────────────────────

struct KarmaRecord {
    // On-chain activity vectors (body_set and mind_set)
    KarmaSet  body_set;               // derived from on-chain activity
    KarmaSet  mind_set;               // derived from intent record

    // Raw accumulators
    uint64_t  prana_burned_lifetime;  // total PRANA burned (satoshis)
    uint32_t  validator_age_blocks;   // blocks since validator first appeared
    uint32_t  dharma_blessings;       // count of white blessing acts
    uint32_t  dharma_sins;            // count of black sin acts
    int32_t   karma_delta_accumulator;// running sum of act deltas

    // Derived (computed)
    KarmaType karma_type;
    int32_t   karma_score;            // [-100, 10000]

    // Compute full karma score from this record
    int32_t ComputeScore() const {
        // Base score from type classification
        KarmaType type = ClassifyKarmaType(
            body_set, mind_set,
            prana_burned_lifetime,
            dharma_blessings,
            dharma_sins
        );

        int32_t score = BaseScoreForType(type);

        // Add accumulation bonuses
        // PRANA burn: 1 point per 1000 satoshis burned (capped at 3000 bonus)
        score += static_cast<int32_t>(
            std::min(prana_burned_lifetime / 1000ULL, 3000ULL)
        );

        // Validator age: 1 point per 1000 blocks (capped at 1000 bonus)
        score += static_cast<int32_t>(
            std::min(validator_age_blocks / 1000U, 1000U)
        );

        // Dharma act net delta (from running accumulator)
        score += karma_delta_accumulator;

        // Clamp to [-100, 10000]
        return std::max(-100, std::min(10000, score));
    }

    // Update body_set based on on-chain metrics
    void UpdateBodySet(
        bool has_prana_burned,    // element 1
        bool has_validated_blocks,// element 2
        bool has_uptime,          // element 3
        bool is_selfless          // ∅ element
    ) {
        body_set = 0;
        if (has_prana_burned)     body_set |= 0x01;
        if (has_validated_blocks) body_set |= 0x02;
        if (has_uptime)           body_set |= 0x04;
        if (is_selfless)          body_set |= EMPTY_BIT;
    }

    // Update mind_set based on intent record
    void UpdateMindSet(
        bool has_community_txs,    // element 2
        bool has_oracle_subs,      // element 3
        bool has_dharma_actions,   // element 4
        bool is_selfless           // ∅ element
    ) {
        mind_set = 0;
        if (has_community_txs)   mind_set |= 0x02;
        if (has_oracle_subs)     mind_set |= 0x04;
        if (has_dharma_actions)  mind_set |= 0x08;
        if (is_selfless)         mind_set |= EMPTY_BIT;
    }

    // Apply a dharma act to the running accumulator
    void ApplyAct(DharmaAct act) {
        karma_delta_accumulator += KarmaDeltaForAct(act);
        if (KarmaDeltaForAct(act) > 0) dharma_blessings++;
        else if (KarmaDeltaForAct(act) < 0) dharma_sins++;
        // Clamp accumulator to prevent runaway
        karma_delta_accumulator = std::max(-2000, std::min(2000, karma_delta_accumulator));
    }

    // String representation for logging
    std::string TypeName() const {
        switch (karma_type) {
            case KarmaType::HOLY:           return "HOLY (Enlightened)";
            case KarmaType::GENEROUS_MIND:  return "GENEROUS_MIND";
            case KarmaType::BOUNTEOUS_BODY: return "BOUNTEOUS_BODY";
            case KarmaType::ORDINARY:       return "ORDINARY";
            case KarmaType::FOOLISH:        return "FOOLISH";
            case KarmaType::ANIMAL:         return "ANIMAL (Primitive)";
            case KarmaType::HYBRID_SOUL:    return "HYBRID_SOUL (BLOCKED)";
            case KarmaType::HELLISH:        return "HELLISH (Penalized)";
            default:                        return "UNKNOWN";
        }
    }
};
