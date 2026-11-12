#include "credentials.h"
#include "errors.h"
#include "temp_file.h"

#include <UnitTest++/UnitTest++.h>

SUITE(LoadCredentials)
{
    TEST(ReadsLoginAndPassword)
    {
        TempFile file("user:P@ssW0rd\n");
        const Credentials credentials = loadCredentials(file.path);
        CHECK_EQUAL("user", credentials.login);
        CHECK_EQUAL("P@ssW0rd", credentials.password);
    }

    TEST(PasswordMayContainColon)
    {
        TempFile file("user:pa:ss");
        CHECK_EQUAL("pa:ss", loadCredentials(file.path).password);
    }

    TEST(MissingFile)
    {
        CHECK_THROW(loadCredentials("/tmp/timp_test_no_such_file"), IOError);
    }

    TEST(EmptyFile)
    {
        TempFile file;
        CHECK_THROW(loadCredentials(file.path), IOError);
    }

    TEST(LineWithoutColon)
    {
        TempFile file("userP@ssW0rd");
        CHECK_THROW(loadCredentials(file.path), IOError);
    }

    TEST(EmptyLogin)
    {
        TempFile file(":P@ssW0rd");
        CHECK_THROW(loadCredentials(file.path), IOError);
    }

    TEST(EmptyPassword)
    {
        TempFile file("user:");
        CHECK_THROW(loadCredentials(file.path), IOError);
    }
}
