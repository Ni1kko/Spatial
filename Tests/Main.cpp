#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int __cdecl main(int argc, char* argv[])
{
    return Catch::Session().run(argc, argv);
}
