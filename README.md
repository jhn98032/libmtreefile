# Introduction

This is a library for operating on binary files that contains a tree of
entries with data.

It can be used for storing structured application data.

# Building

libzip is needed for building:

    sudo apt install libzip-dev


# File format

The file consist of entries.

Each entry consist of:

- 4 bytes:  Type.
- 4 bytes: Length in bytes of entire entry excluding 'type' field and extra end padding.
- Null terminated name
- Null terminated description
- Data (vector contains more than one entry)

If total size of entry is not even 4 bytes then it is padded with 0x0.

Types are:


- 'DIR '
- 'U8  '
- 'S8  '
- 'U16 '
- 'S16 '
- 'U32 '
- 'S32 '
- 'DBL '
- 'STR '
- 'VEC '








