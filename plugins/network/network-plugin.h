#pragma once

#include "plugin-i.h"

namespace Kiran
{
class NetworkPlugin : public Plugin
{
public:
    NetworkPlugin();
    virtual ~NetworkPlugin();

    virtual void activate();

    virtual void deactivate();
};
}  // namespace Kiran
