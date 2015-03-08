#ifndef MODULE_REGISTRY_HPP
#define MODULE_REGISTRY_HPP

#include "Pointers.hpp"

namespace monicelli {

class FunctionPrototype;

class ModuleRegistry {
public:
    ModuleRegistry();
    ModuleRegistry(ModuleRegistry&) = delete;
    virtual ~ModuleRegistry();

    PointerList<FunctionPrototype> const& getRegisteredFunctions() const;
    void registerFunction(FunctionPrototype *proto);

private:
    struct Private;
    Private *d;
};

ModuleRegistry& getModuleRegistry();
void registerStdLib(ModuleRegistry &);

}

#endif
