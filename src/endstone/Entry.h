#pragma once
#include <endstone/plugin/plugin_description.h>
#include <endstone/plugin/plugin.h>

namespace GlacieHook {

class PluginDescriptionBuilderImpl : public endstone::detail::PluginDescriptionBuilder {
public:
    PluginDescriptionBuilderImpl() {
        prefix      = "GlacieHook";
        description = "Glacie Hook Library";
        website     = "https://github.com/GlacieTeam/GlacieHook";
        authors     = {"GlacieTeam"};
    }
};

class Entry : public endstone::Plugin {
public:
    static Entry* getInstance();

    [[nodiscard]] endstone::PluginDescription const& getDescription() const override { return mDescription; }

private:
    PluginDescriptionBuilderImpl mBuilder;
    endstone::PluginDescription  mDescription = mBuilder.build("glacie_hook", "1.0.0");
};
} // namespace GlacieHook