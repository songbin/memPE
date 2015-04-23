#!gmake
#
# Makefile for tdb directory
#

CC = gcc
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
includedir = ${prefix}/include
libdir = ${exec_prefix}/lib
VPATH = ./libreplace
srcdir = .
builddir = .
sharedbuilddir = ./sharedbuild
INSTALLCMD = /usr/bin/install -c
CPPFLAGS =  -I$(srcdir)/include -Iinclude 
CFLAGS = $(CPPFLAGS)  -I./libreplace
LDFLAGS = 
EXEEXT = 
SHLD = gcc
SHLD_FLAGS = -shared -Wl,-Bsymbolic 
PACKAGE_VERSION = 1.1.3
PICFLAG = -fPIC
SHLIBEXT = so
PYTHON = /usr/bin/python
PYTHON_CONFIG = /usr/bin/python-config
PYTHON_BUILD_TARGET = build-python
PYTHON_INSTALL_TARGET = install-python
PYTHON_CHECK_TARGET = check-python
LIB_PATH_VAR = LD_LIBRARY_PATH
tdbdir = .

TDB_OBJ = common/tdb.o common/dump.o common/transaction.o common/error.o common/traverse.o common/freelist.o common/freelistcheck.o common/io.o common/lock.o common/open.o replace.o snprintf.o getpass.o

default: all

include $(tdbdir)/tdb.mk
include $(tdbdir)/rules.mk

all:: showflags dirs $(PROGS) $(TDB_SOLIB) libtdb.a $(PYTHON_BUILD_TARGET) transfer

transfer:
		cp libtdb.a ../libtdb.a

install:: all
$(TDB_SOLIB): $(TDB_OBJ)
	$(SHLD) $(SHLD_FLAGS) -o $@ $(TDB_OBJ) -Wl,-soname=$(TDB_SONAME)

shared-build: all
	${INSTALLCMD} -d $(sharedbuilddir)/lib
	${INSTALLCMD} -m 644 libtdb.a $(sharedbuilddir)/lib
	${INSTALLCMD} -m 755 $(TDB_SOLIB) $(sharedbuilddir)/lib
	ln -sf $(TDB_SOLIB) $(sharedbuilddir)/lib/$(TDB_SONAME)
	ln -sf $(TDB_SOLIB) $(sharedbuilddir)/lib/libtdb.so
	${INSTALLCMD} -d $(sharedbuilddir)/include
	${INSTALLCMD} -m 644 $(srcdir)/include/tdb.h $(sharedbuilddir)/include

check: test

test:: $(PYTHON_CHECK_TARGET)
installcheck:: test install

clean::
	rm -f *.o *.a */*.o ../libtdb.a

distclean:: clean
	rm -f config.log config.status include/config.h config.cache
	rm -f Makefile

realdistclean:: distclean
	rm -f configure include/config.h.in
