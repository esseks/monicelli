#ifndef SCOPE_HPP
#define SCOPE_HPP

#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <vector>
#include <unordered_map>


namespace monicelli {

template<class Key, class Value>
class Scope {
public:
    boost::optional<Value> lookup(Key name) {
        for (auto const& table: boost::adaptors::reverse(tables)) {
            auto result = table.find(name);
            if (result != table.end()) {
                return result->second;
            }
        }

        return boost::none;
    }

    void push(Key const& key, Value const& value) {
        tables.back().insert({key, value});
    }

    void enter() {
        tables.emplace_back();
    }

    void leave() {
        if (!tables.empty()) {
            tables.pop_back();
        }
    }

    void drop() {
        tables.clear();
    }

private:
    std::vector<std::unordered_map<Key, Value>> tables;
};

}

#endif
