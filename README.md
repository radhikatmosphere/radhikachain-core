# RadhikaChain Core

Núcleo L1 de RadhikaChain — consenso **Proof of Karma (PoK)** con BLAKE3 + SHA3, basado en Bitcoin Core v30.2 con módulos Karma (`src/consensus/karma*.cpp`).

> **Estado:** núcleo C++ integrado + minero PoK portable en Python. Sin credenciales hardcodeadas — todo por variables de entorno.

## Instalación rápida

### Minero (cualquier operador)

```bash
git clone https://github.com/radhikatmosphere/radhikachain-core.git
cd radhikachain-core
cp .env.example .env   # edita RPC_USER y RPC_PASS
pip install -r requirements-miner.txt
python3 scripts/karma-mine.py
```

### Nodo + CLI (compilar)

```bash
sudo apt install build-essential cmake ninja-build pkg-config \
  libssl-dev libevent-dev libboost-all-dev libzmq3-dev libsqlite3-dev \
  libcapnp-dev capnproto libnatpmp-dev libminiupnpc-dev

make build
sudo make install-node   # instala radhikad, radhika-cli, radhika-tx
```

O con Docker (entrypoint genera `rpcauth` en runtime, sin contraseña en el conf):

```bash
docker build -t radhikachain-node -f contrib/docker/Dockerfile.radhika-fixed contrib/docker/
docker run -e RPC_PASSWORD=tu_secreto -p 8332:8332 -p 8108:8108 radhikachain-node
```

## Minería PoK

Cada operador usa **sus propias credenciales RPC**:

| Variable | Por defecto | Descripción |
|----------|-------------|-------------|
| `RPC_URL` | `http://127.0.0.1:8332` | Endpoint RPC |
| `RPC_USER` | *(requerido)* | Usuario RPC |
| `RPC_PASS` | *(requerido)* | Contraseña RPC |
| `RPC_WALLET_NAME` | `default` | Wallet para `generateblock` |
| `MINER_ADDRESS` | *(auto)* | Dirección coinbase opcional |
| `KARMA_SCORE` | `5000` | Puntuación Karma PoK |
| `ENFORCE_PRANA_CONSENSUS` | `false` | Requiere Prana Engine |

Guía completa: [doc/MINING.md](doc/MINING.md)

## Origen del código

El núcleo C++ proviene de `radhika-chain` (fork Bitcoin Core con PoK/KarmaHash), integrado desde el snapshot local `radhika-chain-FULL-2026-06-03`.

Binarios compilados se publican como:
- `bitcoind` → `radhikad`
- `bitcoin-cli` → `radhika-cli`
- `bitcoin-tx` → `radhika-tx`

## Estructura

```
radhikachain-core/
├── src/                     # Núcleo C++ (PoK, wallet, RPC, P2P)
│   └── consensus/karma*.cpp
├── scripts/karma-mine.py    # Minero portable
├── contrib/docker/          # Dockerfile + entrypoint seguro
├── share/radhika.conf.template
├── .github/workflows/
│   ├── security.yml         # Gitleaks + check-secrets
│   └── release.yml          # Build x86_64 + minero
├── .env.example
├── Makefile
└── doc/MINING.md
```

## Seguridad

```bash
bash scripts/check-secrets.sh   # escaneo local
make test
```

- ⚠️ Nunca subas `.env`, wallets ni claves privadas
- ✅ `contrib/docker/entrypoint.sh` genera `rpcauth` hasheado desde `RPC_PASSWORD`
- ✅ CI con Gitleaks en cada push/PR

## Licencia

MIT — ver [LICENSE](LICENSE). Código base Bitcoin Core: ver [COPYING](COPYING).

**Versión:** v8.0.0 (Krittika Phase)  
**Genesis:** `00000000367efa345a130ec8944e80fe3cc3d675543f8500c0f085184a4be5a7`