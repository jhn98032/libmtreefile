============
libmtreefile
============

This is a library for operating on binary files that contains a tree of
entries with data.

It can be used for storing structured application data.

Build and install
=================

libzip and doxygen is needed for building:

.. code-block:: bash

    sudo apt install libzip-dev
    sudo apt install doxygen

Run:

.. code-block:: bash

    ./configure

Followed by:

.. code-block:: bash

    make
    make install


Usage instructions
==================

To use add the following to the .pro file in your application:

.. code-block:: bash

  QMAKE_CXXFLAGS+=`libmtreefile-config --cflags`
  LIBS+=`libmtreefile-config --ldflags`

Then include in your application:

.. code-block:: c

  <mtreefile/mtreefile.h>

API documentation exists in:

  /usr/local/docs/libmqtutil/html/index.html


Author
======

Written by Johan Henriksson (johan[a]dexar.se)


