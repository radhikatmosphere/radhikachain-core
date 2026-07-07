PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
SCRIPTDIR ?= $(PREFIX)/share/radhikachain

.PHONY: all check-secrets test install install-miner clean

all: test

check-secrets:
	bash scripts/check-secrets.sh

test:
	bash scripts/test-compile.sh

install-miner:
	install -d "$(DESTDIR)$(BINDIR)" "$(DESTDIR)$(SCRIPTDIR)"
	install -m 755 scripts/karma-mine.py "$(DESTDIR)$(BINDIR)/karma-mine"
	install -m 644 requirements-miner.txt .env.example "$(DESTDIR)$(SCRIPTDIR)/"

install: install-miner
	@echo "Instalado karma-mine en $(DESTDIR)$(BINDIR)"
	@echo "Siguiente: cp $(SCRIPTDIR)/.env.example ~/.radhika/.env y configura RPC_USER/RPC_PASS"

clean:
	@echo "Nada que limpiar en fase scripts-only"