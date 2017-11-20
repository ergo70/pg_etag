MODULE_big = pg_etag
OBJS = pg_etag.o
DATA = pg_etag.sql pg_etag-uninstall.sql pg_etag-test.sql
DOCS =
SCRIPTS =
EXTRA_CLEAN = $(MODULE_big).so

PG_CPPFLAGS=-fomit-frame-pointer -I ./libb2/dist/include

SHLIB_LINK=-L ./libb2/dist/lib -lb2

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

