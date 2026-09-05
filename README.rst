============
libmtreefile
============

This is a library for operating on binary files that contains a tree of
entries with data.

It can be used for storing structured application data.

Build and install
=================

libzip is needed for building:

    sudo apt install libzip-dev

Run:

    ./configure

Followed by:

    make
    make install



Usage instructions
==================

To use add the following to the .pro file in your application:

  QMAKE_CXXFLAGS+=`libmtreefile-config --cflags`
  LIBS+=`libmtreefile-config --ldflags`




Author
======

Written by Johan Henriksson (johan[a]dexar.se)






