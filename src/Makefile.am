# name of your binary
bin_PROGRAMS = .bin/gst-rc

# list of source files
# the prefix is the name of the binary
_bin_gst_rc_SOURCES = main.cpp pipeline.cpp jsonrpc.cpp easylogging++.cpp

# list of headers we're not going to install
#noinst_HEADERS = gst-app.h 

# our CFLAGS and LDFLAGS used for compiling and linking
# make sure you prefix these with the name of your binary
_bin_gst_rc_CXXFLAGS = $(GST_CFLAGS) $(JSON_CFLAGS)
_bin_gst_rc_LDFLAGS = $(GST_LIBS) $(JSON_LIBS)
