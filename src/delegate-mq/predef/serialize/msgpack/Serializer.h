#ifndef SERIALIZER_H
#define SERIALIZER_H

/// @file
/// @see https://github.com/endurodave/cpp-async-delegate
/// David Lafreniere, 2025.
/// 
/// Example of serializer relying upon the serialize class to binary serialize
/// complex data objects including build-in and user defined data types.

#include "msgpack.hpp"
#include "delegate/ISerializer.h"
#include <iostream>

// make_serialized serializes each remote function argument
template<typename... Ts>
void make_serialized(msgpack::sbuffer& buffer) { }

template<typename Arg1, typename... Args>
void make_serialized(msgpack::sbuffer& buffer, Arg1& arg1, Args... args) {
    msgpack::pack(buffer, arg1);
    make_serialized(buffer, args...);
}

// make_unserialized serializes each remote function argument
template<typename... Ts>
void make_unserialized(msgpack::unpacker& unpacker) { }

template<typename Arg1, typename... Args>
void make_unserialized(msgpack::unpacker& unpacker, Arg1& arg1, Args&&... args) {
    msgpack::object_handle oh;
    unpacker.next(oh);
    arg1 = oh.get().as<Arg1>();
    make_unserialized(unpacker, args...);
}

template <class R>
struct Serializer; // Not defined

// Serialize all target function argument data
template<class RetType, class... Args>
class Serializer<RetType(Args...)> : public DelegateMQ::ISerializer<RetType(Args...)>
{
public:
    virtual std::ostream& Write(std::ostream& os, Args... args) override {
        msgpack::sbuffer buffer;
        make_serialized(buffer, args...);
        os.write(buffer.data(), buffer.size());
        return os;
    }

    virtual std::istream& Read(std::istream& is, Args&... args) override {
        std::string buffer_data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        msgpack::unpacker unpacker;
        unpacker.reserve_buffer(buffer_data.size());
        std::memcpy(unpacker.buffer(), buffer_data.data(), buffer_data.size());
        unpacker.buffer_consumed(buffer_data.size());
        make_unserialized(unpacker, args...);
        return is;
    }
};

#endif