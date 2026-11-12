#include "data_writer.h"
#include "errors.h"
#include "temp_file.h"

#include <UnitTest++/UnitTest++.h>

SUITE(DataWriter)
{
    TEST(WritesCountAndResults)
    {
        TempFile file;
        DataWriter(file.path).write({3, 6, 65535, 1000});
        CHECK_EQUAL("4 3 6 65535 1000\n", file.content());
    }

    TEST(WritesZeroForNoResults)
    {
        TempFile file;
        DataWriter(file.path).write({});
        CHECK_EQUAL("0\n", file.content());
    }

    TEST(ReplacesOldContent)
    {
        TempFile file("old data that is longer than results\n");
        DataWriter(file.path).write({1});
        CHECK_EQUAL("1 1\n", file.content());
    }

    TEST(MissingDirectory)
    {
        CHECK_THROW(DataWriter("/tmp/timp_test_no_such_dir/result.txt"),
                    IOError);
    }
}
