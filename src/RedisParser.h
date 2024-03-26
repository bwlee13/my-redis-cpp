//
// Created by Brandon W Lee on 3/26/24.
//

#ifndef CODECRAFTERS_REDIS_CPP_REDISPARSER_H
#define CODECRAFTERS_REDIS_CPP_REDISPARSER_H
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>


// 1.: *x = x number of components, command, command + args etc
// 2.: $x = x length command
// 3.: command
// 4?: $x = x length of argument
// 5?: argument/s

// "hello"
// $5\r\nhello\r\n

// "hello world"
// *2\r\n$5\r\nhello\r\n$5\r\nworld\r\n
// ["hello", "world"]

struct RESPBulkString {
    std::string value;
};

struct RESPArray {
    std::vector<std::string> array;
};

struct RESP {
    char type;
    std::string raw;
    std::string data;
    std::vector<std::string> array;
    int count;
};

enum RESPMessageType {
    BULK_STRING,
    ARRAY,
};


#endif //CODECRAFTERS_REDIS_CPP_REDISPARSER_H
