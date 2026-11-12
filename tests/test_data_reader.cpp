#include "data_reader.h"
#include "errors.h"
#include "temp_file.h"

#include <UnitTest++/UnitTest++.h>

namespace {

Vectors readFrom(const std::string& content)
{
    TempFile file(content);
    return DataReader(file.path).read();
}

}

SUITE(DataReader)
{
    TEST(ReadsVectors)
    {
        const Vectors expected{{1, 2}, {65535, 0, 7}};
        CHECK(readFrom("2\n2 1 2\n3 65535 0 7\n") == expected);
    }

    TEST(NumbersMaySpanAnyWhitespace)
    {
        const Vectors expected{{10, 20}};
        CHECK(readFrom("1 2\t10\n\n20   \n") == expected);
    }

    TEST(ZeroVectors)
    {
        CHECK(readFrom("0").empty());
    }

    TEST(MissingFile)
    {
        CHECK_THROW(DataReader("/tmp/timp_test_no_such_file"), IOError);
    }

    TEST(EmptyFile)
    {
        CHECK_THROW(readFrom(""), IOError);
    }

    TEST(ValueAboveTypeMaximum)
    {
        CHECK_THROW(readFrom("1 1 65536"), IOError);
    }

    TEST(NegativeValue)
    {
        CHECK_THROW(readFrom("1 1 -1"), IOError);
    }

    TEST(NotANumber)
    {
        CHECK_THROW(readFrom("1 2 5 abc"), IOError);
    }

    TEST(FractionalNumber)
    {
        CHECK_THROW(readFrom("1 1 2.5"), IOError);
    }

    TEST(EmptyVector)
    {
        CHECK_THROW(readFrom("1 0"), IOError);
    }

    TEST(FewerValuesThanDeclared)
    {
        CHECK_THROW(readFrom("1 3 1 2"), IOError);
    }

    TEST(FewerVectorsThanDeclared)
    {
        CHECK_THROW(readFrom("2 1 5"), IOError);
    }

    TEST(ExtraDataAfterVectors)
    {
        CHECK_THROW(readFrom("1 1 5 6"), IOError);
    }
}
