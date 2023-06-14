#################################
# $Id$
# Author:  Eugene Vasilchenko
#################################

# Build application "graph_test"
#################################

APP = graph_test
SRC = graph_test

LIB =   $(SRAREAD_LIBS) $(SOBJMGR_LDEP) $(CMPRS_LIB)
LIBS =  $(SRA_SDK_SYSLIBS) $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)
POST_LINK = $(VDB_POST_LINK)

REQUIRES = objects

CPPFLAGS = $(ORIG_CPPFLAGS) $(SRA_INCLUDE)

CHECK_CMD = graph_test -q .

WATCHERS = vasilche ucko
