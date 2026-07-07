# Minería PoK — Guía para cualquier operador

RadhikaChain usa **Proof of Karma (PoK)**: BLAKE3 sobre el header del bloque, seguido de SHA3-256 con un commitment de Karma score.

## Antes de empezar

1. Tienes un nodo sincronizado (`radhikad` o compatible).
2. RPC habilitado con usuario/contraseña **propios** (no los del repositorio).
3. Wallet cargada con permiso de minería (`generateblock`).

## Configuración

```bash
cp .env.example .env
```

Edita al menos:

```bash
RPC_URL=http://127.0.0.1:8332   # o https://rpc.tudominio.xyz detrás de túnel
RPC_USER=tu_usuario
RPC_PASS=tu_secreto_largo
RPC_WALLET_NAME=default
```

Opcional:

```bash
MINER_ADDRESS=bc1q...           # si no, el minero pide una nueva
KARMA_SCORE=5000
ENFORCE_PRANA_CONSENSUS=false    # true solo si operas con Prana Engine
```

## Ejecutar

```bash
pip install -r requirements-miner.txt
python3 scripts/karma-mine.py
```

Instalación global:

```bash
sudo bash install.sh
karma-mine
```

## Nodo remoto (túnel / VPS)

- No abras `8332` a Internet sin autenticación.
- Usa Cloudflare Tunnel, `rpcauth` bcrypt o VPN.
- Exporta `RPC_URL` apuntando al endpoint seguro.

## Solución de problemas

| Síntoma | Causa probable | Acción |
|---------|----------------|--------|
| `RPC_USER y RPC_PASS` | `.env` vacío | Rellena credenciales locales |
| `generateblock no devolvió hex` | Wallet no cargada | `radhika-cli loadwallet default` |
| `401 Unauthorized` | RPC mal configurado | Revisa usuario/contraseña del nodo |
| `Prana Engine no disponible` | `ENFORCE_PRANA_CONSENSUS=true` | Pon `false` o levanta Prana |

## Seguridad

- Este repositorio **no contiene** credenciales reales.
- Rota `RPC_PASS` si alguna vez se filtró.
- No compartas tu `.env` ni wallets entre operadores.