# RadhikaChain Core

RadhikaChain L1 Blockchain Core Implementation - Proof of Karma (PoK) consensus with eBPF network sensors.

## Quick Install

```bash
# One-liner install (auto-detect mode)
curl -fsSL https://radhikachain.xyz/install | bash

# Specific modes
curl -fsSL https://radhikachain.xyz/install | bash -s -- --peer     # Lightweight relay
curl -fsSL https://radhikachain.xyz/install | bash -s -- --full     # Full validator
curl -fsSL https://radhikachain.xyz/install | bash -s -- --cloud    # VPS-optimized
curl -fsSL https://radhikachain.xyz/install | bash -s -- --mobile   # Termux/PWA

# Dry-run (preview without changes)
curl -fsSL https://radhikachain.xyz/install | bash -s -- --dry-run
```

## Build from Source

### Prerequisites
- C++17 compiler (g++ >= 9 or clang++ >= 10)
- Make
- OpenSSL development libraries
- Git

### Compile

```bash
git clone https://github.com/radhikatmosphere/radhikachain-core.git
cd radhikachain-core
make
sudo make install
```

### Custom Build Options

```bash
# Custom install prefix
make PREFIX=/opt/radhika install

# Debug build
make DEBUG=1

# Static linking
make STATIC=1
```

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `RPC_URL` | `http://localhost:8332` | RadhikaChain RPC endpoint |
| `RPC_USER` | (required) | RPC authentication username |
| `RPC_PASS` | (required) | RPC authentication password |
| `RADHIKA_DIR` | `~/.radhika` | Data directory |
| `LOG_LEVEL` | `info` | Logging verbosity (debug/info/warn/error) |
| `NETWORK` | `mainnet` | Network (mainnet/testnet) |

### Example `.env` File

```bash
RPC_URL=http://localhost:8332
RPC_USER=radhika_user
RPC_PASS=secure_password_here
RADHIKA_DIR=/var/lib/radhika
LOG_LEVEL=info
NETWORK=mainnet
```

## Usage

### Start Node

```bash
radhikad --daemon
```

### Check Status

```bash
radhika-cli getblockchaininfo
radhika-cli getnetworkinfo
radhika-cli getwalletinfo
```

### View Logs

```bash
journalctl -u radhika -f
# or
tail -f ~/.radhika/debug.log
```

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                 RadhikaChain L1                     │
├─────────────────────────────────────────────────────┤
│  PoK Consensus (BLAKE3 + SHA3 KarmaHash)           │
│  eBPF Network Sensor (DharmaGate)                  │
│  ZMQ Oracle (PRANA burn detector)                  │
│  Atomic Swap Engine (Bridge.xyz HTLC)              │
│  Cloudflare Edge Security (Worker + Tunnel)        │
└─────────────────────────────────────────────────────┘
```

## Project Structure

```
radhikachain-core/
├── src/                    # Source code
│   ├── main.cpp           # Entry point
│   ├── lib/               # Core library
│   │   ├── radhika.cpp
│   │   └── radhika.h
│   └── utils/             # Utilities
│       ├── logger.cpp
│       └── logger.h
├── scripts/               # Helper scripts
│   ├── validate-install.sh
│   └── test-compile.sh
├── Makefile              # Build configuration
├── install.sh            # Installation script
├── compile.sh            # Compilation wrapper
├── README.md             # This file
└── LICENSE               # MIT License
```

## Testing

```bash
# Run compilation tests
bash scripts/test-compile.sh

# Validate installation
bash scripts/validate-install.sh

# Check security (no hardcoded secrets)
grep -r "password\|secret\|key" src/ --exclude="*.md" || echo "OK: No hardcoded secrets"
```

## Troubleshooting

### Docker Not Found
```bash
# Install Docker
curl -fsSL https://get.docker.com | sudo bash
sudo usermod -aG docker $USER
```

### Port Already in Use
```bash
# Check what's using port 8332
sudo lsof -i :8332

# Or change RPC port via env
export RPC_PORT=8333
```

### Build Fails
```bash
# Clean and rebuild
make clean
make DEBUG=1

# Check dependencies
g++ --version
openssl version
```

## Security Notes

- ⚠️ **Never commit** `.env` files or wallet data
- ⚠️ **Store private keys** in secure, offline storage
- ✅ **Use environment variables** for secrets
- ✅ **Enable firewall** and restrict RPC access
- ✅ **Regular backups** of wallet and blockchain data

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Support

- Documentation: https://radhikachain.xyz/docs
- Issues: https://github.com/radhikatmosphere/radhikachain-core/issues
- Website: https://radhikachain.xyz

## Version

**Current**: v8.0.0 (Krittika Phase)

**Genesis Hash**: `00000000367efa345a130ec8944e80fe3cc3d675543f8500c0f085184a4be5a7`
