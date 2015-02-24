// Copyright 2014 Stellar Development Foundation and contributors. Licensed
// under the ISC License. See the COPYING file at the top-level directory of
// this distribution or at http://opensource.org/licenses/ISC

#include "util/asio.h"
#include "main/Config.h"
#include "main/Application.h"
#include "xdrpp/autocheck.h"
#include "main/test.h"
#include "lib/catch.hpp"
#include "util/Logging.h"
#include "util/Timer.h"
#include <future>
#include "process/ProcessGateway.h"

using namespace stellar;

TEST_CASE("subprocess", "[process]")
{
    VirtualClock clock;
    Config const& cfg = getTestConfig();
    Application app(clock, cfg);
    auto evt = app.getProcessGateway().runProcess("hostname");
    bool exited = false;
    evt.async_wait([&](asio::error_code ec)
                   {
                       CLOG(DEBUG, "Process") << "process exited: " << ec;
                       if (ec)
                       {
                           CLOG(DEBUG, "Process") << "error code: " << ec.message();
                       }
                       exited = true;
                   });

    while (!exited && !app.getMainIOService().stopped())
    {
        app.getMainIOService().poll_one();
    }
}

TEST_CASE("subprocess redirect to file", "[process]")
{
    VirtualClock clock;
    Config const& cfg = getTestConfig();
    Application app(clock, cfg);
    std::string filename("hostname.txt");
    auto evt = app.getProcessGateway().runProcess("hostname", filename);
    bool exited = false;
    evt.async_wait([&](asio::error_code ec)
                   {
                       CLOG(DEBUG, "Process") << "process exited: " << ec;
                       if (ec)
                       {
                           CLOG(DEBUG, "Process") << "error code: " << ec.message();
                       }
                       exited = true;
                   });

    while (!exited && !app.getMainIOService().stopped())
    {
        app.crank(false);
    }

    std::ifstream in(filename);
    CHECK(in);
    std::string s;
    in >> s;
    CLOG(DEBUG, "Process") << "opened redirect file, read: " << s;
    CHECK(!s.empty());
    std::remove(filename.c_str());
}
