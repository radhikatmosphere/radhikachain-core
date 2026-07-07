#!/usr/bin/env bash
# entrypoint.sh — Genera bitcoin.conf en runtime con variables expandidas
# Reemplaza el heredoc con comillas simples del Dockerfile original.
#
# Variables de entorno reconocidas (todas con defaults seguros):
#   RPC_PASSWORD          contraseña RPC (se convierte a rpcauth hasheado)
#   RPC_USER              usuario RPC (default: radhika)
#   RADHIKA_DBCACHE       cache en MB (default: 1024)
#   RADHIKA_MAXMEMPOOL    mempool en MB (default: 300)  ← fix: era mempoolsize
#   RADHIKA_PORT          P2P port (default: 8108)
#   RADHIKA_RPCPORT       RPC port (default: 8332)  ← fix: era 8109 en conf original
#   RADHIKA_ADDNODE       peer manual (opcional)
#   RADHIKA_DEBUG         nivel de log (default: 0)

set -euo pipefail

DATADIR="/home/radhika/.radhikachain"
CONFFILE="${DATADIR}/bitcoin.conf"

mkdir -p "$DATADIR"

# ── Valores con defaults ──────────────────────────────────────────────────────
RPC_USER="${RPC_USER:-radhika}"
RPC_PASSWORD="${RPC_PASSWORD:-}"
DBCACHE="${RADHIKA_DBCACHE:-1024}"
MAXMEMPOOL="${RADHIKA_MAXMEMPOOL:-300}"
PORT="${RADHIKA_PORT:-8108}"
RPCPORT="${RADHIKA_RPCPORT:-8332}"
DEBUG="${RADHIKA_DEBUG:-0}"

# ── Generar rpcauth hasheado (reemplaza rpcpassword en texto plano) ───────────
# rpcauth evita almacenar la contraseña en texto plano en el conf.
# Formato: rpcauth=USER:SALT$HASH  donde HASH = HMAC-SHA256(SALT, PASSWORD)
generate_rpcauth() {
  local user="$1"
  local pass="$2"
  local salt
  salt=$(python3 -c "import os, base64; print(base64.b64encode(os.urandom(16)).decode()[:16])" 2>/dev/null) || salt=$(openssl rand -base64 16 | cut -c1-16)
  local hash
  hash=$(python3 -c "
import hmac, hashlib, sys
salt = sys.argv[1]
password = sys.argv[2]
h = hmac.new(salt.encode('utf-8'), password.encode('utf-8'), hashlib.sha256).hexdigest()
print(h)
" "$salt" "$pass")
  echo "${user}:${salt}\$${hash}"
}

# Si no hay RPC_PASSWORD, usar cookie-based auth (más seguro, default Bitcoin Core)
if [[ -z "$RPC_PASSWORD" ]]; then
  RPCAUTH_LINE="# rpcauth: using cookie-based auth (more secure)"
  echo "⚠  RPC_PASSWORD no configurado — usando cookie-based auth"
  echo "   El cookie se guarda en: ${DATADIR}/.cookie"
else
  RPCAUTH=$(generate_rpcauth "$RPC_USER" "$RPC_PASSWORD")
  RPCAUTH_LINE="rpcauth=${RPCAUTH}"
  echo "✓  rpcauth generado para usuario: $RPC_USER"
fi

# ── Escribir bitcoin.conf ─────────────────────────────────────────────────────
# Nota: se escribe con > (sin heredoc con comillas) para garantizar expansión.
cat > "$CONFFILE" << EOF
# bitcoin.conf — RadhikaChain L1 Mainnet
# Generado en runtime por entrypoint.sh — $(date -u +%Y-%m-%dT%H:%M:%SZ)
# NO editar manualmente — regenerado en cada arranque del contenedor

# ── Red ───────────────────────────────────────────────────────────────────────
chain=main
port=${PORT}
listen=1
maxconnections=32

# ── RPC ──────────────────────────────────────────────────────────────────────
server=1
rpcport=${RPCPORT}
rpcbind=0.0.0.0
rpcallowip=127.0.0.1
rpcallowip=172.16.0.0/12
${RPCAUTH_LINE}

# ── Datos ────────────────────────────────────────────────────────────────────
txindex=1
blockfilterindex=1
dbcache=${DBCACHE}
maxmempool=${MAXMEMPOOL}

# ── ZMQ ──────────────────────────────────────────────────────────────────────
zmqpubrawblock=tcp://0.0.0.0:28332
zmqpubrawtx=tcp://0.0.0.0:28333
zmqpubhashblock=tcp://0.0.0.0:28334
zmqpubhashtx=tcp://0.0.0.0:28335
zmqpubsequence=tcp://0.0.0.0:28336

# ── Logging ──────────────────────────────────────────────────────────────────
debug=${DEBUG}
logips=1
printtoconsole=1
EOF

# Agregar addnode si se proporcionó
if [[ -n "${RADHIKA_ADDNODE:-}" ]]; then
  echo "addnode=${RADHIKA_ADDNODE}" >> "$CONFFILE"
  echo "✓  addnode: ${RADHIKA_ADDNODE}"
fi

echo "✓  bitcoin.conf generado en ${CONFFILE}"
echo "   RPC port:  ${RPCPORT}"
echo "   P2P port:  ${PORT}"
echo "   dbcache:   ${DBCACHE} MB"
echo "   maxmempool:${MAXMEMPOOL} MB"
echo ""

# ── Lanzar bitcoind ───────────────────────────────────────────────────────────
exec bitcoind \
  -conf="${CONFFILE}" \
  -datadir="${DATADIR}" \
  "$@"
