#!/usr/bin/env python3
"""
PoK (Proof of Karma) miner — portable para cualquier operador.

Credenciales solo por variables de entorno o archivo .env local (nunca en git).
Requiere un nodo RadhikaChain con RPC habilitado y wallet con fondos/capacidad
de generateblock (radhika-cli / bitcoin-cli compatible).
"""

from __future__ import annotations

import base64
import hashlib
import json
import os
import struct
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

try:
    import blake3 as _blake3
except ImportError:
    print("[karma-mine] Instala dependencias: pip install -r requirements-miner.txt", file=sys.stderr)
    raise SystemExit(1)

DHARMA_NONCE_MODULUS = 108


def load_env() -> dict[str, str]:
    """Carga .env local si existe; las variables del proceso tienen prioridad."""
    merged: dict[str, str] = {}
    candidates = [
        Path.cwd() / ".env",
        Path(__file__).resolve().parent.parent / ".env",
    ]
    for path in candidates:
        if not path.is_file():
            continue
        for line in path.read_text().splitlines():
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, value = line.split("=", 1)
            merged[key.strip()] = value.strip().strip("\"'")
        break
    merged.update({k: v for k, v in os.environ.items() if v is not None})
    return merged


ENV = load_env()

RPC_URL = ENV.get("RPC_URL", "http://127.0.0.1:8332").rstrip("/")
RPC_USER = ENV.get("RPC_USER") or ENV.get("RADHIKA_RPC_USER") or ""
RPC_PASS = ENV.get("RPC_PASS") or ENV.get("RPC_PASSWORD") or ENV.get("RADHIKA_RPC_PASS") or ""
WALLET_NAME = ENV.get("RPC_WALLET_NAME", "default")
RPC_WALLET_URL = ENV.get("RPC_WALLET_URL", f"{RPC_URL}/wallet/{WALLET_NAME}").rstrip("/")
CLI_BIN = ENV.get("CLI_BIN", "radhika-cli")
KARMA_SCORE = int(ENV.get("KARMA_SCORE", "5000"))
BATCH_SIZE = int(ENV.get("BATCH_SIZE", "10000"))
MINER_ADDRESS = ENV.get("MINER_ADDRESS", "").strip()
ENFORCE_PRANA = ENV.get("ENFORCE_PRANA_CONSENSUS", "false").lower() == "true"
PRANA_URL = ENV.get("PRANA_ENGINE_URL", "http://127.0.0.1:5000/status")

WALLET_METHODS = {
    "generateblock",
    "getnewaddress",
    "getrawchangeaddress",
    "sendtoaddress",
    "sendmany",
    "loadwallet",
    "createwallet",
}


def require_rpc_auth() -> None:
    if not RPC_USER or not RPC_PASS:
        print(
            "[karma-mine] ERROR: define RPC_USER y RPC_PASS en .env o entorno.\n"
            "  cp .env.example .env && edita tus credenciales.",
            file=sys.stderr,
        )
        raise SystemExit(2)


def rpc(method: str, *params):
    payload = json.dumps({"jsonrpc": "1.0", "id": "karma", "method": method, "params": list(params)}).encode()
    url = RPC_WALLET_URL if method in WALLET_METHODS else RPC_URL
    auth = base64.b64encode(f"{RPC_USER}:{RPC_PASS}".encode()).decode()
    req = urllib.request.Request(
        url,
        data=payload,
        headers={"Content-Type": "application/json", "Authorization": f"Basic {auth}"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=120) as resp:
            data = json.loads(resp.read().decode())
    except urllib.error.HTTPError as exc:
        body = exc.read().decode(errors="replace")
        raise RuntimeError(f"RPC HTTP {exc.code} {method}: {body}") from exc
    if data.get("error"):
        raise RuntimeError(f"RPC {method}: {data['error']}")
    return data.get("result")


def cli(*args):
    cmd = [
        CLI_BIN,
        f"-rpcuser={RPC_USER}",
        f"-rpcpassword={RPC_PASS}",
        f"-rpcconnect={ENV.get('RPC_HOST', '127.0.0.1')}",
        f"-rpcport={ENV.get('RPC_PORT', '8332')}",
    ] + list(args)
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        err = result.stderr.strip() or result.stdout.strip()
        raise RuntimeError(f"CLI error: {err}")
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError:
        return result.stdout.strip()


def call(method: str, *params):
    """Prefiere JSON-RPC; cae a CLI si RPC_USE_CLI=true."""
    if ENV.get("RPC_USE_CLI", "false").lower() == "true":
        return cli(method, *params)
    return rpc(method, *params)


def target_from_bits(bits: int) -> bytes:
    exponent = bits >> 24
    mantissa = bits & 0x007FFFFF
    if exponent <= 3:
        target = mantissa >> (8 * (3 - exponent))
    else:
        target = mantissa << (8 * (exponent - 3))
    return target.to_bytes(32, "big")


def effective_target(base_target: bytes, karma_score: int) -> bytes:
    value = int.from_bytes(base_target, "big")
    multiplier = 10000 + min(max(karma_score, 0), 10000) // 2
    return (value * multiplier // 10000).to_bytes(32, "big")


def blake3_hex_lines(lines: list[str]) -> list[str]:
    return [_blake3.blake3(bytes.fromhex(line)).hexdigest() for line in lines if line]


def check_prana() -> bool:
    if not ENFORCE_PRANA:
        return True
    try:
        with urllib.request.urlopen(PRANA_URL, timeout=3.0) as resp:
            return resp.status == 200
    except Exception:
        return False


def resolve_address(provided: str | None) -> str:
    if provided:
        return provided
    if MINER_ADDRESS:
        return MINER_ADDRESS
    return call("getnewaddress", "karmamining", "bech32")


def mine_block(address: str) -> bool:
    if not check_prana():
        print("[karma-mine] Prana Engine no disponible (ENFORCE_PRANA_CONSENSUS=true)")
        return False

    print(f"[karma-mine] generateblock → {address}")
    result = call("generateblock", address, [], False)
    block_hex = (result or {}).get("hex") if isinstance(result, dict) else None
    if not block_hex:
        print("[karma-mine] generateblock no devolvió hex")
        return False

    block_bytes = bytes.fromhex(block_hex)
    header = bytearray(block_bytes[:80])
    struct.pack_into("<I", header, 0, KARMA_SCORE << 16 | 0x20000000)
    struct.pack_into("<I", header, 68, int(time.time()))

    info = call("getblockchaininfo")
    start_height = int(info["blocks"])
    bits = int(info["bits"], 16)
    base_target = target_from_bits(bits)
    effective = effective_target(base_target, KARMA_SCORE)
    effective_int = int.from_bytes(effective, "big")
    commitment = struct.pack("<I", KARMA_SCORE) + struct.pack("<Q", 0) + struct.pack("<I", 0)

    nonce = 0
    max_nonce = 2**32 - 1
    nonce = (nonce // DHARMA_NONCE_MODULUS) * DHARMA_NONCE_MODULUS
    attempts = 0
    started = time.time()

    while nonce <= max_nonce:
        batch_headers: list[str] = []
        batch_nonces: list[int] = []
        for _ in range(BATCH_SIZE):
            struct.pack_into("<I", header, 76, nonce)
            batch_headers.append(bytes(header).hex())
            batch_nonces.append(nonce)
            nonce += DHARMA_NONCE_MODULUS
            if nonce > max_nonce:
                break

        for index, line in enumerate(blake3_hex_lines(batch_headers)):
            digest = hashlib.sha3_256(bytes.fromhex(line) + commitment).digest()
            attempts += 1
            if int.from_bytes(digest, "big") <= effective_int:
                struct.pack_into("<I", header, 76, batch_nonces[index])
                full_block = bytes(header) + block_bytes[80:]
                call("submitblock", full_block.hex())
                time.sleep(0.5)
                new_height = int(call("getblockchaininfo")["blocks"])
                elapsed = max(time.time() - started, 0.1)
                if new_height > start_height:
                    print(
                        f"[karma-mine] ✅ bloque aceptado height={new_height} "
                        f"intentos={attempts} ({attempts/elapsed:.0f} h/s)"
                    )
                    return True
                print("[karma-mine] ❌ bloque rechazado")
                return False

        if attempts and attempts % 50000 == 0:
            elapsed = max(time.time() - started, 0.1)
            print(f"[karma-mine] {attempts} intentos {attempts/elapsed:.0f} h/s nonce={nonce}")

    print("[karma-mine] espacio de nonce agotado")
    return False


def main() -> int:
    require_rpc_auth()
    address = resolve_address(sys.argv[1] if len(sys.argv) > 1 else None)
    print(f"[karma-mine] RPC={RPC_URL} wallet={WALLET_NAME} karma={KARMA_SCORE}")
    return 0 if mine_block(address) else 1


if __name__ == "__main__":
    raise SystemExit(main())