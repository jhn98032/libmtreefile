/*
 * libmtreefile - 
 * Copyright (C) 2026 Johan Henriksson johan[a]dexar.se
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */


namespace mtreefile
{


/**
@mainpage libmqtutil

mtreefile is a library for storing data in binary files which can be compressed.

- @ref MQTreeFile
- @ref MQTreeFileDirNode
- MQTreeFileEntryNode
- getMTreeFileVersion()


@section sec_usage Usage


Example:

@code

    #include <mtreefile/mtreefile.h>
    using namespace mtreefile;
    
    MTreeFile file;
    if(file.load("test.bin"))
        printf("Unable to load file");
    else {
        file.setFieldString("field", "data");
    }


@endcode

or:

@code
    #include <mtreefile/mqtreefile.h>
    using namespace mtreefile;

    MQTreeFile file;
    if(file.load("test.bin"))
        printf("Unable to load file");
    else {
        file.setFieldString("field", "data");
    }

@endcode


*/

}; // namespace mtreefile;

