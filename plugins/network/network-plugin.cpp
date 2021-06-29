
#include "plugins/network/network-plugin.h"
#include "lib/base/log.h"
#include "plugins/network/network-proxy.h"

PLUGIN_EXPORT_FUNC_DEF(NetworkPlugin);

namespace Kiran
{
NetworkPlugin::NetworkPlugin()
{
}

NetworkPlugin::~NetworkPlugin()
{
}

void NetworkPlugin::activate()
{
    SETTINGS_PROFILE("active network plugin.");

    NetworkProxy::global_init();
}

void NetworkPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive network plugin.");
    NetworkProxy::global_deinit();
}
}  // namespace Kiran