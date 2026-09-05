
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

