#include "errors.h"
#include "user_interface.h"

#include <UnitTest++/UnitTest++.h>

#include <cstdlib>
#include <string>
#include <vector>

namespace {

bool parse(UserInterface& ui, std::vector<const char*> args)
{
    args.insert(args.begin(), "client");
    return ui.parse(args.size(), args.data());
}

bool parseRequiredWith(UserInterface& ui, std::vector<const char*> extra)
{
    std::vector<const char*> args{"-a", "127.0.0.1", "-i", "data.txt",
                                  "-o", "result.txt"};
    args.insert(args.end(), extra.begin(), extra.end());
    return parse(ui, args);
}

}

SUITE(UserInterface)
{
    TEST(NoArgumentsMeansHelp)
    {
        UserInterface ui;
        CHECK(!parse(ui, {}));
    }

    TEST(HelpOption)
    {
        UserInterface ui;
        CHECK(!parse(ui, {"-h"}));
        CHECK(!parse(ui, {"--help"}));
    }

    TEST(HelpDescribesUsage)
    {
        const std::string help = UserInterface().help();
        CHECK(help.find("Usage: client") == 0);
        CHECK(help.find("--address") != std::string::npos);
        CHECK(help.find("--config") != std::string::npos);
    }

    TEST(RequiredOptionsAndDefaults)
    {
        UserInterface ui;
        CHECK(parseRequiredWith(ui, {}));
        const Params& params = ui.params();
        CHECK_EQUAL("127.0.0.1", params.address);
        CHECK_EQUAL("data.txt", params.inputFile);
        CHECK_EQUAL("result.txt", params.outputFile);
        CHECK_EQUAL(33333, params.port);
        CHECK_EQUAL(std::string(std::getenv("HOME")) + "/.config/vclient.conf",
                    params.configFile);
    }

    TEST(LongOptionNames)
    {
        UserInterface ui;
        CHECK(parse(ui, {"--address", "10.0.0.1", "--input", "in",
                         "--output", "out", "--port", "4000", "--config",
                         "my.conf"}));
        CHECK_EQUAL("10.0.0.1", ui.params().address);
        CHECK_EQUAL(4000, ui.params().port);
        CHECK_EQUAL("my.conf", ui.params().configFile);
    }

    TEST(ConfigPathFromHomeIsExpanded)
    {
        UserInterface ui;
        parseRequiredWith(ui, {"-c", "~/client.conf"});
        CHECK_EQUAL(std::string(std::getenv("HOME")) + "/client.conf",
                    ui.params().configFile);
    }

    TEST(MissingRequiredOption)
    {
        UserInterface ui;
        CHECK_THROW(parse(ui, {"-i", "data.txt", "-o", "result.txt"}),
                    InterfaceError);
    }

    TEST(UnknownOption)
    {
        UserInterface ui;
        CHECK_THROW(parseRequiredWith(ui, {"-x"}), InterfaceError);
    }

    TEST(PositionalArgument)
    {
        UserInterface ui;
        CHECK_THROW(parseRequiredWith(ui, {"extra"}), InterfaceError);
    }

    TEST(PortIsNotANumber)
    {
        UserInterface ui;
        CHECK_THROW(parseRequiredWith(ui, {"-p", "abc"}), InterfaceError);
    }

    TEST(PortAboveMaximum)
    {
        UserInterface ui;
        CHECK_THROW(parseRequiredWith(ui, {"-p", "65536"}), InterfaceError);
    }

    TEST(NegativePort)
    {
        UserInterface ui;
        CHECK_THROW(parseRequiredWith(ui, {"-p", "-1"}), InterfaceError);
    }

    TEST(ZeroPort)
    {
        UserInterface ui;
        CHECK_THROW(parseRequiredWith(ui, {"-p", "0"}), InterfaceError);
    }

    TEST(PortBoundaries)
    {
        UserInterface lowest;
        CHECK(parseRequiredWith(lowest, {"-p", "1"}));
        CHECK_EQUAL(1, lowest.params().port);
        UserInterface highest;
        CHECK(parseRequiredWith(highest, {"-p", "65535"}));
        CHECK_EQUAL(65535, highest.params().port);
    }
}
