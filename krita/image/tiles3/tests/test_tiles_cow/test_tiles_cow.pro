######################################################################
# Automatically generated by qmake (2.01a) Fri May 22 18:59:47 2009
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += . 

LIBS += -g -Wl,-rpath,/home/devel/kde-install/release/kde/lib

LIBPATH += /home/devel/kde-install/release/kde/lib/
LIBS +=  -lkdecore


INCLUDEPATH += . ../../ /home/devel/kde-install/release/kde/include/ /home/devel/kde-src/koffice/krita/image/

# Input
HEADERS += ../../kis_tile.h \
           ../../kis_tile_data.h \
           ../../kis_tile_data_store.h \
           ../../kis_tiled_data_manager.h \
           ../../kis_tile_processors.h \
           ../../kis_tile_hash_table.h \
           ../../../kis_shared_data.h \
           ../../../kis_shared.h \
           ../../../kis_shared_ptr.h

SOURCES += ../../kis_tile.cc \
           ../../kis_tile_data.cc \
           ../../kis_tile_data_store.cc \
           ../../kis_tiled_data_manager.cc \
           ../../kis_tile_processors.cc \
           ../../kis_tile_hash_table.cc \
           ../../../kis_shared.cc \
           main.cc
