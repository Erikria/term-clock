#Term-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = clock.c
CC ?= gcc
BIN ?= term-clock
PREFIX ?= /usr/local
INSTALLPATH ?= ${DESTDIR}${PREFIX}/bin
MANPATH ?= ${DESTDIR}${PREFIX}/share/man/man1

LDFLAGS += -lncursesw

${BIN}: ${SRC}
	@echo "building ${SRC}"
	${CC} ${SRC} ${LDFLAGS} -o ${BIN}

install: ${BIN}
	@echo "installing binary file to ${INSTALLPATH}/${BIN}"
	@mkdir -p ${INSTALLPATH}
	@cp ${BIN} ${INSTALLPATH}
	@chmod 0755 ${INSTALLPATH}/${BIN}
	@echo "installing manpage to ${MANPATH}/${BIN}.1"
	@mkdir -p ${MANPATH}
	@cp ${BIN}.1 ${MANPATH}
	@chmod 0644 ${MANPATH}/${BIN}.1
	@echo "installed"

uninstall:
	@echo "uninstalling binary file (${INSTALLPATH})"
	@rm -f ${INSTALLPATH}/${BIN}
	@echo "uninstalling manpage (${MANPATH})"
	@rm -f ${MANPATH}/${BIN}.1
	@echo "${BIN} uninstalled"

clean:
	@echo "cleaning ${BIN}"
	@rm -f ${BIN}
	@echo "${BIN} cleaned"
