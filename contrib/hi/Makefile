# contrib/hi/Makefile
MODULES = hi
EXTENSION = hi

DATA = hi--1.0.sql
PGFILEDESC = "test extension"
REGRESS = hi

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/hi
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

