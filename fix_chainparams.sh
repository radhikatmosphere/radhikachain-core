#!/bin/bash
# fix_chainparams.sh — Fix 4 RADH blockers in Bitcoin Core v30.2 chainparams
# Run from bitcoin-30.2/ root

set -e

CHAINPARAMS="src/kernel/chainparams.cpp"

if [ ! -f "$CHAINPARAMS" ]; then
    echo "❌ Error: $CHAINPARAMS not found"
    exit 1
fi

echo "🔧 Fixing RadhikaChain (RADH) chainparams..."

# ─────────────────────────────────────────────────────────────────────────────
# BLOCKER 1: nDefaultPort — Bitcoin 8333 → RADH 8108
# ─────────────────────────────────────────────────────────────────────────────
if grep -q 'nDefaultPort = 8333;' "$CHAINPARAMS"; then
    sed -i 's/nDefaultPort = 8333;/nDefaultPort = 8108;  \/\/ RADH: 1008 names of Shiva\/Kali/' "$CHAINPARAMS"
    echo "✅ Fixed: nDefaultPort 8333 → 8108"
else
    echo "⚠️  nDefaultPort already patched or pattern changed"
fi

# ─────────────────────────────────────────────────────────────────────────────
# BLOCKER 2: vSeeds — Bitcoin seeds → placeholder for RADH
# ─────────────────────────────────────────────────────────────────────────────
# Comment out Bitcoin seeds and add RADH placeholder
if grep -q 'seed.bitcoin.sipa.be' "$CHAINPARAMS"; then
    sed -i '/vSeeds.emplace_back("seed.bitcoin.sipa.be");/c\        \/\/ RADH: Bitcoin seeds removed\n        \/\/ vSeeds.emplace_back("seed.bitcoin.sipa.be");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("dnsseed.bluematt.me");/c\        \/\/ vSeeds.emplace_back("dnsseed.bluematt.me");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch");/c\        \/\/ vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("seed.btc.petertodd.net");/c\        \/\/ vSeeds.emplace_back("seed.btc.petertodd.net");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("seed.bitcoin.sprovoost.nl");/c\        \/\/ vSeeds.emplace_back("seed.bitcoin.sprovoost.nl");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("dnsseed.emzy.de");/c\        \/\/ vSeeds.emplace_back("dnsseed.emzy.de");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("seed.bitcoin.wiz.biz");/c\        \/\/ vSeeds.emplace_back("seed.bitcoin.wiz.biz");' "$CHAINPARAMS"
    sed -i '/vSeeds.emplace_back("seed.mainnet.achownodes.xyz");/c\        \/\/ vSeeds.emplace_back("seed.mainnet.achownodes.xyz");\n        vSeeds.emplace_back("SEED_NODE_PLACEHOLDER");  \/\/ RADH: Replace with your seed node' "$CHAINPARAMS"
    echo "✅ Fixed: Bitcoin seeds commented out"
else
    echo "⚠️  Bitcoin seeds already patched or pattern changed"
fi

# ─────────────────────────────────────────────────────────────────────────────
# BLOCKER 3: base58Prefixes — Bitcoin 0 (1...) → RADH 63 (R...)
# ─────────────────────────────────────────────────────────────────────────────
# Only fix the one in CMainParams (line ~154)
if grep -n 'base58Prefixes\[PUBKEY_ADDRESS\] = std::vector<unsigned char>(1,0);' "$CHAINPARAMS" | head -1 | grep -q "15[0-9]:"; then
    # This is in main params, replace
    sed -i '0,/base58Prefixes\[PUBKEY_ADDRESS\] = std::vector<unsigned char>(1,0);/s//base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,63);  \/\/ RADH: R... addresses/' "$CHAINPARAMS"
    echo "✅ Fixed: base58Prefixes[PUBKEY_ADDRESS] 0 → 63 (R...)"
else
    echo "⚠️  base58Prefixes already patched or pattern changed"
fi

# ─────────────────────────────────────────────────────────────────────────────
# BLOCKER 4: bech32_hrp — Bitcoin "bc" → RADH "radh"
# ─────────────────────────────────────────────────────────────────────────────
# Only fix the one in CMainParams (line ~159)
if grep -n 'bech32_hrp = "bc";' "$CHAINPARAMS" | head -1 | grep -q "15[0-9]:"; then
    sed -i '0,/bech32_hrp = "bc";/s//bech32_hrp = "radh";  \/\/ RADH: radh1q... addresses/' "$CHAINPARAMS"
    echo "✅ Fixed: bech32_hrp bc → radh"
else
    echo "⚠️  bech32_hrp already patched or pattern changed"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔍 Verificación rápida:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -n "Port 8108: " && grep -q 'nDefaultPort = 8108' "$CHAINPARAMS" && echo "✅ OK" || echo "❌ FAIL"
echo -n "R... addresses (63): " && grep -q 'base58Prefixes\[PUBKEY_ADDRESS\] = std::vector<unsigned char>(1,63)' "$CHAINPARAMS" && echo "✅ OK" || echo "❌ FAIL"
echo -n "bech32 radh: " && grep -q 'bech32_hrp = "radh"' "$CHAINPARAMS" && echo "✅ OK" || echo "❌ FAIL"
echo -n "Magic RADH: " && grep -q 'pchMessageStart\[0\] = 0x52' "$CHAINPARAMS" && echo "✅ OK" || echo "❌ FAIL"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo ""
echo "✅ fix_chainparams.sh completado."
echo ""
echo "⚠️  Nota: El bloque génesis actual tiene hash: 000000b784aa30faf5f48cad62f5f64037f3f7bc35a41de901856cd4fa4309b8"
echo "   Si deseas usar los valores del CLAUDE.md original, necesitarás reminar."
