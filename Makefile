PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
SCRIPTDIR ?= $(PREFIX)/share/radhikachain
BUILD_DIR ?= build
BUILD_WIN_DIR ?= build-win
DIST_DIR ?= dist
VERSION ?= v8.0.0
CMAKE ?= cmake
NPROC ?= $(shell nproc 2>/dev/null || echo 4)
CMAKE_FLAGS := -GNinja \
	-DBUILD_DAEMON=ON \
	-DBUILD_CLI=ON \
	-DBUILD_TX=ON \
	-DBUILD_TESTS=OFF \
	-DENABLE_WALLET=ON
DEPENDS_HOST ?= x86_64-w64-mingw32

.PHONY: all check-secrets test install install-miner configure build smoke \
        depends-win build-win package-linux package-win clean

all: test

check-secrets:
	bash scripts/check-secrets.sh

test:
	bash scripts/test-compile.sh

configure:
	$(CMAKE) -B "$(BUILD_DIR)" $(CMAKE_FLAGS)

build: configure
	$(CMAKE) --build "$(BUILD_DIR)" -j"$(NPROC)"

smoke: build
	BUILD_DIR="$(BUILD_DIR)" bash scripts/smoke-node.sh

depends-win:
	cd depends && $(MAKE) HOST=$(DEPENDS_HOST) NO_QT=1 NO_USDT=1 -j"$(NPROC)"

configure-win: depends-win
	$(CMAKE) -B "$(BUILD_WIN_DIR)" $(CMAKE_FLAGS) \
		--toolchain depends/$(DEPENDS_HOST)/toolchain.cmake

build-win: configure-win
	$(CMAKE) --build "$(BUILD_WIN_DIR)" -j"$(NPROC)"

package-linux: build
	@mkdir -p "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu/bin"
	cp "$(BUILD_DIR)/bin/bitcoind" "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu/bin/radhikad"
	cp "$(BUILD_DIR)/bin/bitcoin-cli" "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu/bin/radhika-cli"
	cp "$(BUILD_DIR)/bin/bitcoin-tx" "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu/bin/radhika-tx"
	cp contrib/docker/entrypoint.sh share/radhika.conf.template "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu/"
	tar -C "$(DIST_DIR)" -czf "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu.tar.gz" "radhikachain-$(VERSION)-x86_64-linux-gnu"
	sha256sum "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu.tar.gz" > "$(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu.tar.gz.sha256"
	@echo "OK: $(DIST_DIR)/radhikachain-$(VERSION)-x86_64-linux-gnu.tar.gz"

package-win: build-win
	@mkdir -p "$(DIST_DIR)/radhikachain-$(VERSION)-win64/bin"
	cp "$(BUILD_WIN_DIR)/bin/bitcoind.exe" "$(DIST_DIR)/radhikachain-$(VERSION)-win64/bin/radhikad.exe"
	cp "$(BUILD_WIN_DIR)/bin/bitcoin-cli.exe" "$(DIST_DIR)/radhikachain-$(VERSION)-win64/bin/radhika-cli.exe"
	cp "$(BUILD_WIN_DIR)/bin/bitcoin-tx.exe" "$(DIST_DIR)/radhikachain-$(VERSION)-win64/bin/radhika-tx.exe"
	cp share/radhika.conf.template "$(DIST_DIR)/radhikachain-$(VERSION)-win64/"
	cd "$(DIST_DIR)" && zip -r "radhikachain-$(VERSION)-win64.zip" "radhikachain-$(VERSION)-win64"
	sha256sum "$(DIST_DIR)/radhikachain-$(VERSION)-win64.zip" > "$(DIST_DIR)/radhikachain-$(VERSION)-win64.zip.sha256"
	@echo "OK: $(DIST_DIR)/radhikachain-$(VERSION)-win64.zip"

install-miner:
	install -d "$(DESTDIR)$(BINDIR)" "$(DESTDIR)$(SCRIPTDIR)"
	install -m 755 scripts/karma-mine.py "$(DESTDIR)$(BINDIR)/karma-mine"
	install -m 644 requirements-miner.txt .env.example "$(DESTDIR)$(SCRIPTDIR)/"

install-node: build
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 755 "$(BUILD_DIR)/bin/bitcoind" "$(DESTDIR)$(BINDIR)/radhikad"
	install -m 755 "$(BUILD_DIR)/bin/bitcoin-cli" "$(DESTDIR)$(BINDIR)/radhika-cli"
	install -m 755 "$(BUILD_DIR)/bin/bitcoin-tx" "$(DESTDIR)$(BINDIR)/radhika-tx"

install: install-miner
	@echo "Minero instalado. Para el daemon: make build && make install-node"

clean:
	rm -rf "$(BUILD_DIR)" "$(BUILD_WIN_DIR)" "$(DIST_DIR)"