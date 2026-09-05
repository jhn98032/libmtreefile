/*
 * libmtreefile - 
 * Copyright (C) 2026 Johan Henriksson johan[a]dexar.se
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "zipfile.h"

#include <zip.h>
#include <iostream>
#include <limits>

#include "file.h"


using namespace mtreefile;


ZipFile::ZipFile()
    : za(nullptr)
{

}


ZipFile::~ZipFile()
{
    if (zip_close(za) < 0) {
        std::cerr << "zip_close failed (but data was read)\n";
    }

}


/**
 * @brief Checks if a file is zip file.
 */
bool ZipFile::isZipFile(QString filename)
{
    PhysicalFile phys;
    if(phys.openReadOnly(filename))
        return -1;
    char buf[2];
    phys.read(buf,2);
    if(buf[0] == 0x50 && buf[1] == 0x4b)
        return true;
    return false;
}


int ZipFile::load(QString filename)
{
    int err = 0;
    za = zip_open(qPrintable(filename), ZIP_RDONLY, &err);
    if (!za) {
        zip_error_t ze;
        zip_error_init_with_code(&ze, err);
        std::cerr << "Failed to open ZIP: " << zip_error_strerror(&ze) << "\n";
        zip_error_fini(&ze);
        return -1;
    }

    return 0;
}



QStringList ZipFile::getEntries()
{
    QStringList list;
    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    std::cout << "Archive contains " << num_entries << " entries:\n";

    for (zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(num_entries); ++i)
    {
        const char* name = zip_get_name(za, i, 0);
        list.append(name);
    }
    return list;
}



int ZipFile::readEntry(
                          const char* entryName,
                          QVector<uint8_t>* out)
{
    out->clear();



    // Stat the entry to get its size (if available)
    zip_stat_t st;
    zip_stat_init(&st);
    if (zip_stat(za, entryName, 0, &st) != 0) {
        std::cerr << "Entry not found: " << entryName << "\n";
        zip_discard(za);
        return -1;
    }

    // Open the file inside the ZIP for reading (decompresses on the fly if needed)
    zip_file_t* zf = zip_fopen(za, entryName, 0);
    if (!zf) {
        std::cerr << "zip_fopen failed: " << zip_strerror(za) << "\n";
        zip_discard(za);
        return -1;
    }

    // If size is known, pre-allocate; otherwise, grow in chunks
    constexpr size_t kChunk = 64 * 1024;
    if ((st.valid & ZIP_STAT_SIZE) && st.size <= std::numeric_limits<size_t>::max()) {
        out->resize(static_cast<size_t>(st.size));
        size_t total = 0;
        while (total < (size_t)out->size()) {
            zip_int64_t n = zip_fread(zf, out->data() + total, out->size() - total);
            if (n < 0) {
                std::cerr << "zip_fread failed\n";
                zip_fclose(zf);
                zip_discard(za);
                out->clear();
                return -1;
            }
            if (n == 0) break; // EOF
            total += static_cast<size_t>(n);
        }
        out->resize(total); // in case of short read
    } else {
        // Unknown size: read in chunks and append
        QVector<uint8_t> buf(kChunk);
        for (;;) {
            zip_int64_t n = zip_fread(zf, buf.data(), buf.size());
            if (n < 0) {
                std::cerr << "zip_fread failed\n";
                zip_fclose(zf);
                zip_discard(za);
                out->clear();
                return -1;
            }
            if (n == 0) break; // EOF
            out->append(buf);
        }
    }

    zip_fclose(zf);
    return 0;
}

