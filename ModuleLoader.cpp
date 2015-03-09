#include "Nodes.hpp"
#include "ModuleRegistry.hpp"
#include "ModuleLoader.hpp"

#include <yaml-cpp/yaml.h>
#include <string>

using namespace monicelli;


static
Type toType(std::string const& value) {
    if (value == "int") {
        return Type::INT;
    } else if (value == "float") {
        return Type::FLOAT;
    } else if (value == "double") {
        return Type::DOUBLE;
    } else if (value == "char") {
        return Type::CHAR;
    } else if (value == "bool") {
        return Type::BOOL;
    } else {
        return Type::VOID;
    }
}

void monicelli::loadModule(std::string const& from, ModuleRegistry &to) {
    YAML::Node module  = YAML::LoadFile(from);

    if (!module["functions"]) return;

    for (auto const& proto: module["functions"]) {
        PointerList<FunArg> *args = new PointerList<FunArg>();
        for (auto const& arg: proto.second["args"]) {
            args->push_back(new FunArg(
                new Id(arg.first.as<std::string>()),
                toType(arg.second.as<std::string>()),
                false
            ));
        }

        Type type;
        if (proto.second["type"]) {
            type = toType(proto.second["type"].as<std::string>());
        } else {
            type = Type::VOID;
        }

        to.registerFunction(new FunctionPrototype(
            new Id(proto.first.as<std::string>()), type, args
        ));
    }
}

