PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
SCRIPTDIR ?= $(PREFIX)/share/radhikachain
BUILD_DIR ?= build
CMAKE ?= cmake

.PHONY: all check-secrets test install install-miner configure build clean

all: test

check-secrets:
	bash scripts/check-secrets.sh

test:
	bash scripts/test-compile.sh

configure:
	mkdir -p "$(BUILD_DIR)"
	cd "$(BUILD_DIR)" && $(CMAKE) -GNinja \
		-DBUILD_DAEMON=ON \
		-DBUILD_CLI=ON \
		-DBUILD_TX=ON \
		-DBUILD_UTIL=ON \
		-DBUILD_WALLET=ON \
		..

build: configure
	cd "$(BUILD_DIR)" && ninja

install-miner:
	install -d "$(DESTDIR)$(BINDIR)" "$(DESTDIR)$(SCRIPTDIR)"
	install -m 755 scripts/karma-mine.py "$(DESTDIR)$(BINDIR)/karma-mine"
	install -m 644 requirements-miner.txt .env.example "$(DESTDIR)$(SCRIPTDIR)/"

install-node: build
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 755 "$(BUILD_DIR)/bin/bitcoind" "$(DESTDIR)$(BINDIR)/radhikad"
	install -m 755 "$(BUILD_DIR)/bin/bitcoin-cli" "$(DESTDIR)$(BINDIR)/radhika-cli"
	install -m 755 "$(BUILD_DIR)/bin/bitcoin-tx" "$(DESTDIR)$(BINDIR)/radhika-tx"
	ln -sf radhika-cli "$(DESTDIR)$(BINDIR)/bitcoin-cli" 2>/dev/null || true
	ln -sf radhikad "$(DESTDIR)$(BINDIR)/bitcoind" 2>/dev/null || true

install: install-miner
	@echo "Minero instalado. Para el daemon: make build && make install-node"

clean:
	rm -rf "$(BUILD_DIR)"