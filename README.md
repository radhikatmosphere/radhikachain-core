# RadhikaChain Core

Implementación del núcleo L1 de RadhikaChain — consenso **Proof of Karma (PoK)** con sensores de red eBPF.

> **Estado actual (v8.0.0 — fase Krittika):** minero PoK portable en Python listo para cualquier operador. El daemon C++ (`radhikad`) se integrará desde `radhika-chain` en una fase posterior; el CI empaqueta scripts mientras tanto.

## Instalación rápida

```bash
git clone https://github.com/radhikatmosphere/radhikachain-core.git
cd radhikachain-core
bash install.sh
```

O con el instalador remoto (cuando el nodo completo esté publicado):

```bash
curl -fsSL https://radhikachain.xyz/install | bash
```

## Minería PoK (cualquier usuario)

Cada operador usa **sus propias credenciales RPC** — nunca hardcodeadas en el repositorio.

```bash
# 1. Configura secretos locales
cp .env.example .env
# Edita RPC_USER y RPC_PASS con los de TU nodo

# 2. Dependencias del minero
pip install -r requirements-miner.txt

# 3. Minar (crea dirección coinbase si no pasas una)
python3 scripts/karma-mine.py

# O con dirección explícita
python3 scripts/karma-mine.py bc1q...
```

### Variables de entorno

| Variable | Por defecto | Descripción |
|----------|-------------|-------------|
| `RPC_URL` | `http://127.0.0.1:8332` | Endpoint RPC del nodo |
| `RPC_USER` | *(requerido)* | Usuario RPC |
| `RPC_PASS` | *(requerido)* | Contraseña RPC |
| `RPC_WALLET_NAME` | `default` | Wallet para `generateblock` |
| `MINER_ADDRESS` | *(auto)* | Dirección coinbase opcional |
| `KARMA_SCORE` | `5000` | Puntuación Karma PoK |
| `ENFORCE_PRANA_CONSENSUS` | `false` | Requiere Prana Engine online |
| `CLI_BIN` | `radhika-cli` | CLI alternativa (`bitcoin-cli`) |

Ver `.env.example` para la lista completa.

### Requisitos del nodo

- `radhikad` (o nodo compatible) con RPC activo y wallet cargada
- Métodos RPC: `getblockchaininfo`, `generateblock`, `submitblock`, `getnewaddress`
- Puerto RPC no expuesto públicamente sin túnel/autenticación (Cloudflare Zero Trust, `rpcauth`, firewall)

Guía detallada: [doc/MINING.md](doc/MINING.md)

## Compilar desde fuente

### Fase actual (scripts)

```bash
make test          # escaneo de secretos + validación
make install       # instala karma-mine
bash scripts/check-secrets.sh
```

### Fase futura (daemon C++)

Prerrequisitos: C++20, CMake ≥ 3.16, OpenSSL, Boost, libevent, ZMQ, SQLite3.

```bash
mkdir build && cd build
cmake -GNinja -DBUILD_RADHIKAD=ON ..
ninja
```

> El árbol `src/` se poblará al portar el núcleo desde el repositorio privado `radhika-chain`.

## Configuración segura

```bash
# Ejemplo .env — NUNCA subir al repositorio
RPC_URL=http://127.0.0.1:8332
RPC_USER=mi_usuario
RPC_PASS=cambiar_por_secreto_largo
RADHIKA_DIR=~/.radhika
NETWORK=mainnet
```

## Arquitectura

```
┌─────────────────────────────────────────────────────┐
│                 RadhikaChain L1                     │
├─────────────────────────────────────────────────────┤
│  PoK Consensus (BLAKE3 + SHA3 KarmaHash)            │
│  eBPF Network Sensor (DharmaGate)                   │
│  ZMQ Oracle (detector de quema PRANA)               │
│  Motor Atomic Swap (Bridge.xyz HTLC)                │
│  Seguridad Edge Cloudflare (Worker + Tunnel)      │
└─────────────────────────────────────────────────────┘
```

## Estructura del proyecto

```
radhikachain-core/
├── scripts/
│   ├── karma-mine.py        # Minero PoK portable
│   ├── check-secrets.sh     # CI: sin secretos hardcodeados
│   ├── validate-install.sh
│   └── test-compile.sh
├── doc/
│   └── MINING.md
├── .github/workflows/
│   ├── security.yml         # Gitleaks + patrones locales
│   └── release.yml
├── .env.example             # Plantilla sin secretos
├── requirements-miner.txt
├── install.sh
├── Makefile
├── CMakeLists.txt           # Build C++ (fase futura)
└── README.md
```

## Seguridad

- ⚠️ **Nunca** subas `.env`, wallets ni claves privadas
- ⚠️ Guarda claves privadas en almacenamiento offline
- ✅ Usa **variables de entorno** para todos los secretos
- ✅ Activa firewall y restringe acceso RPC
- ✅ El CI ejecuta `check-secrets.sh` + Gitleaks en cada push/PR

```bash
bash scripts/check-secrets.sh
```

## Licencia

MIT — ver [LICENSE](LICENSE).

## Soporte

- Documentación: https://radhikachain.xyz/docs
- Issues: https://github.com/radhikatmosphere/radhikachain-core/issues
- Web: https://radhikachain.xyz

**Versión:** v8.0.0 (Krittika Phase)  
**Genesis hash:** `00000000367efa345a130ec8944e80fe3cc3d675543f8500c0f085184a4be5a7`