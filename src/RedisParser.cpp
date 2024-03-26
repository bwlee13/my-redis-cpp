//
// Created by Brandon W Lee on 3/26/24.
//

#include "RedisParser.h"


class RedisParser {

    static std::tuple<size_t, RESP> parseBulkyString(RESP& parsed_message, const std::string_view& unparsed_message, int i) {
        size_t count = std::stoi(parsed_message.data);

        if (unparsed_message.substr(count + 1, 1) != "\n" || unparsed_message.substr(count, 1) != "\r") {
            return { 0, {} };
        }

        parsed_message.raw = unparsed_message.substr(0, count + 2);
        parsed_message.data = unparsed_message.substr(0, count);
        parsed_message.count = parsed_message.raw.size() + i;

        std::cout << "unparsed_message 2: " << unparsed_message << ": " << parsed_message.raw.size() << ": " << count << std::endl;

        return { parsed_message.count, parsed_message };
    }

public:
    static std::unordered_map<std::string, std::string> store;

    static std::tuple<size_t, RESP> parse(const std::string_view& message, RESP parsed_message = {}) {
        // *2\r\n$5\r\nhello\r\n$5\r\nworld\r\n
        std::cout << "In here \n";
        char message_type = message[0];
        std::cout << "Msg type: " << message_type << std::endl;
        std::cout << "Msg size: " << message.size() << std::endl;
        size_t i = 1;
        for (;; i++) {
            std::cout << "i: " << i << std::endl;
            std::cout << "msg: " << message[i] << std::endl;
            if (message.size() == i) {
                std::cout << "Msg size: " << message.size() << std::endl;
                std::cout << "i: " << i << std::endl;
                return { 0, {} };
            }
            if (message[i] == '\n') {
                std::cout << "Msg n: " << message[i] << std::endl;
                std::cout << "i: " << i << std::endl;
                if (message[i - 1] != '\r') {
                    std::cout << "Msg r: " << message[i - 1] << std::endl;
                    return { 0, {} };
                }
                i++;
                break;
            }
        }

        parsed_message.raw = message.substr(0, i);
        parsed_message.data = message.substr(1, i - 2);

        std::vector<std::string> data_array = {};
        std::string_view unparsed_message;
        int total_count;
        size_t number_elements;

        switch (message_type) {
        case '+':
            return { parsed_message.raw.size(), parsed_message };

        case '$':
            parsed_message.type = BULK_STRING;
            unparsed_message = message.substr(i);
            return parseBulkyString(parsed_message, unparsed_message, i);
            break;

        case '*':
            std::cout << "am i hjere? \n";
            parsed_message.type = ARRAY;
            number_elements = std::stoi(parsed_message.data);
            unparsed_message = message.substr(i);
            parsed_message.raw = unparsed_message;
            total_count = 0;

            for (size_t i = 0; i < number_elements; i++) {
                auto [count, new_message] = parse(unparsed_message, parsed_message);
                std::cout << "New Message... " << new_message.data << std::endl;
                total_count += count;
                unparsed_message = unparsed_message.substr(count);
                data_array.push_back(new_message.data);
            }

            parsed_message.array = data_array;
            parsed_message.raw = parsed_message.raw.substr(0, i + total_count);

            std::cout << "Echooooo " << message << std::endl;

            return { parsed_message.raw.size(), parsed_message };

        default:
            return { 0, {} };
        }
    }
};
