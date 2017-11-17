# $Header: /code/pg-mdagg/Makefile,v 1.1.1.1 2009/07/08 07:29:50 doj Exp $
# this is a GNU Makefile. on FreeBSD use "gmake"

MODULE_big = pg_etag
OBJS = pg_etag.o
DATA = pg_etag.sql pg_etag-uninstall.sql pg_etag-test.sql
DOCS =
SCRIPTS =
EXTRA_CLEAN = $(MODULE_big).so

OS := $(shell uname -s)
ifeq ($(OS), FreeBSD)
SHLIB_LINK = -lmd
endif

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

clean-lib:
	rm -f libpg_etag.so.0 *~
