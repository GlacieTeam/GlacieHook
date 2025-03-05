#include "Entry.h"

namespace GlacieHook {

Entry* Entry::getInstance() {
    static Entry* instance;
    if (!instance) instance = new Entry();
    return instance;
}

} // namespace GlacieHook

extern "C" [[maybe_unused]] ENDSTONE_EXPORT endstone::Plugin* init_endstone_plugin() {
    return GlacieHook::Entry::getInstance();
}