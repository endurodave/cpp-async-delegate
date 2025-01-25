#ifndef DATA_H
#define DATA_H

/// @file 
/// @see https://github.com/endurodave/cpp-async-delegate
/// David Lafreniere, 2025.

#include <msgpack.hpp>
#include <vector>
#include <string.h>

class DataPoint
{
public:
    int x = 0;
    int y = 0;

    void Test();

    MSGPACK_DEFINE(x, y);
};

/// @brief Argument data sent to endpoint.
class Data 
{
public:
    std::vector<DataPoint> dataPoints;
    std::string msg;

    MSGPACK_DEFINE(dataPoints, msg);
};

#endif
