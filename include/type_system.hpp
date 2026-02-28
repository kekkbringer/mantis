//
// Created by dominik on 8/3/25.
//

#ifndef TYPE_SYSTEM_HPP
#define TYPE_SYSTEM_HPP

#include <memory>
#include <string>
#include <vector>

/**
 * Enum class of different kinds of types tracked in the type pool, namely:
 * Void, Int, Pointer, Array, Function, Struct
 */
enum class Type_kind {
    Void, Int, Pointer, Array, Function, Struct,
};

struct Type {
    Type_kind kind; ///< kind of the type (Void, Int, Pointer, Array, Function, Struct)

    // TODO: slowly uncomment as new features are added
    //Type* base = nullptr;                              ///< base of pointers and arrays
    //size_t array_size = 0;                             ///< size of array
    std::vector<std::shared_ptr<Type>> params;         ///< parameter list of function
    const std::shared_ptr<Type> return_type = nullptr; ///< return type of function
    //std::string struct_name;                           ///< name of structure

    bool operator==(const Type& other) const {
        return kind == other.kind
            && return_type == other.return_type
            && params.size() == other.params.size()
            && params == other.params;
    }
};

struct Type_key_hash {
    size_t operator()(const Type& t) const {
        size_t hash = std::hash<int>()(static_cast<int>(t.kind));
        hash ^= std::hash<std::shared_ptr<Type>>()(t.return_type) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        for (const auto& param: t.params) {
            hash ^= std::hash<std::shared_ptr<Type>>()(param) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

class Type_pool {
public:
    // build in singletons
    std::shared_ptr<Type> type_void;
    std::shared_ptr<Type> type_int;

    std::unordered_map<Type, std::shared_ptr<Type>, Type_key_hash> type_map;

    Type_pool() : type_void(std::make_shared<Type>(Type(Type_kind::Void, {}))),
                  type_int(std::make_shared<Type>(Type(Type_kind::Int, {}))) {}

    std::shared_ptr<Type> get_function(const std::shared_ptr<Type>& ret,
                                       const std::vector<std::shared_ptr<Type>>& params) {
        // make type
        Type t{Type_kind::Function, params, ret};

        // look for type in pool
        if (const auto it = type_map.find(t); it != type_map.end()) return it->second;

        // not found, add to pool, then return
        type_map[t] = std::make_shared<Type>(std::move(t));
        return type_map[t];
    }
};

#endif //TYPE_SYSTEM_HPP
