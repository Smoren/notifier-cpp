#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <utility>
#include <set>
#include <map>
#include <deque>

template <typename Collection>
std::string join(const Collection& collection, const std::string& delimiter);

template <typename First, typename Second>
std::ostream& operator <<(std::ostream& stream, const std::pair<First, Second>& p);

template <typename Key, typename Value>
std::ostream& operator <<(std::ostream& stream, const std::map<Key, Value>& m);

template <typename Value>
std::ostream& operator <<(std::ostream& stream, const std::vector<Value>& m);

template <typename Value>
std::ostream& operator <<(std::ostream& stream, const std::deque<Value>& m);

template <typename Value>
std::ostream& operator <<(std::ostream& stream, const std::set<Value>& m);


template <typename Collection>
std::string join(const Collection& collection, const std::string& delimiter) {
    bool isFirst = true;
    std::stringstream ss;
    for(auto& item : collection) {
        if(!isFirst) {
            ss << delimiter;
        } else {
            isFirst = false;
        }
        ss << item;
    }
    return ss.str();
}

template <typename First, typename Second>
std::ostream& operator <<(std::ostream& stream, const std::pair<First, Second>& p) {
    // requires import "utility"
    return stream << '(' << p.first << ", " << p.second << ')';
}

template <typename Key, typename Value>
std::ostream& operator <<(std::ostream& stream, const std::map<Key, Value>& m) {
    return stream << "{" << join(m, ", ") << "}";
}

template <typename Value>
std::ostream& operator <<(std::ostream& stream, const std::vector<Value>& m) {
    return stream << "[" << join(m, ", ") << "]";
}

template <typename Value>
std::ostream& operator <<(std::ostream& stream, const std::set<Value>& m) {
    return stream << "(" << join(m, ", ") << ")";
}

template <typename Value>
std::ostream& operator <<(std::ostream& stream, const std::deque<Value>& m) {
    return stream << "[" << join(m, ", ") << "]";
}

// Helper function for printing tuple elements
template<typename Tuple, size_t... Is>
void print_tuple(std::ostream& os, const Tuple& t, std::index_sequence<Is...>) {
    ((os << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
}

// Output operator overload for std::tuple
template<typename... Args>
std::ostream& operator<<(std::ostream& os, const std::tuple<Args...>& t) {
    os << "(";
    if constexpr (sizeof...(Args) > 0) {
        print_tuple(os, t, std::index_sequence_for<Args...>{});
    }
    os << ")";
    return os;
}

template<typename T>
void print(const T& value) {
    std::cout << value << std::endl;
}

template<typename T>
void print(const std::string &message, const T& value) {
    std::cout << message << ": " << value << std::endl;
}
