#include "errors.h"

#include <UnitTest++/UnitTest++.h>

#include <string>

SUITE(Errors)
{
    TEST(MessageHasKindPlaceAndReason)
    {
        CHECK_EQUAL("I/O error in DataReader::read: value is missing",
                    std::string(IOError("DataReader::read", "value is missing")
                                    .what()));
    }

    TEST(EachKindHasItsOwnPrefix)
    {
        CHECK_EQUAL("Interface error in f: x",
                    std::string(InterfaceError("f", "x").what()));
        CHECK_EQUAL("Network error in f: x",
                    std::string(NetworkError("f", "x").what()));
        CHECK_EQUAL("Authentication error in f: x",
                    std::string(AuthError("f", "x").what()));
    }

    TEST(AllErrorsAreClientErrors)
    {
        CHECK_THROW(throw IOError("f", "x"), ClientError);
        CHECK_THROW(throw InterfaceError("f", "x"), ClientError);
        CHECK_THROW(throw NetworkError("f", "x"), ClientError);
        CHECK_THROW(throw AuthError("f", "x"), ClientError);
    }
}
