if DEBUG
DBGFLAGS = -g -DDEBUG
endif

DEF_SAGA           = -D_SAGA_LINUX -D_TYPEDEF_BYTE -D_TYPEDEF_WORD
CXX_INCS           = -I$(top_srcdir)/src/saga_core
AM_CXXFLAGS        = -fPIC $(CXX_INCS) $(DEF_SAGA) $(DBGFLAGS) $(GOMPFLAGS)
AM_LDFLAGS         = -fPIC -shared -avoid-version
pkglib_LTLIBRARIES = libimagery_svm.la
libimagery_svm_la_SOURCES =\
MLB_Interface.cpp\
svm_grids.cpp\
MLB_Interface.h\
svm_grids.h

if SYSTEM_SVM
AM_LDFLAGS += -lsvm
DEF_SAGA += -DSYSTEM_SVM
else
libimagery_svm_la_SOURCES +=\
svm/svm.cpp\
svm/svm.h
endif

libimagery_svm_la_LIBADD = $(top_srcdir)/src/saga_core/saga_api/libsaga_api.la

