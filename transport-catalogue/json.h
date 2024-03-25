#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    const Value& GetValue() const { return value_; }
    
//    Node(const std::initializer_list<Node>& init_list)
//    : value_(init_list) {}

    
    Node()= default;
    
    
    Node(Array array);
     Node(Dict map);
     Node(int value);
     Node(std::string value);
     Node(std::nullptr_t null);
     Node(bool log);
     Node(double value);
    
//    Array(Node item);
//    const Array& AsArray() const;
//    const Dict& AsMap() const;
//    int AsInt() const;
//    double AsDouble() const;
//    bool AsBool() const;
//    const std::string& AsString() const;
//
//
//    bool IsInt() const;
//    bool IsDouble() const;
//    bool IsPureDouble() const;
//    bool IsBool() const;
//    bool IsString() const;
//    bool IsNull() const;
//    bool IsArray() const;
//    bool IsMap() const;
    bool IsInt() const;
    int AsInt() const;
    bool IsPureDouble() const;
    bool IsDouble() const;
    double AsDouble() const;
    bool IsBool() const;
    bool AsBool() const;
    bool IsNull() const;
    bool IsArray() const;
    const Array& AsArray() const;
    bool IsString() const;
    const std::string& AsString() const;
    bool IsMap() const;
    const Dict& AsMap() const;
    
    bool operator==(const Node& other) const {
           // Здесь вы можете определить логику сравнения двух узлов
           // Например, если у вас есть какие-то данные, которые определяют узлы,
           // вы можете сравнивать эти данные здесь
           // В примере ниже, предполагается, что узлы равны, если они оба пусты
        return GetValue() == other.GetValue();
       }

       // Оператор != может быть реализован также, если вам нужно
       bool operator!=(const Node& other) const {
           return !(*this == other);
       }
    
   

private:

    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator==(const Document& other){
        return GetRoot().GetValue() == other.GetRoot().GetValue();
    }

    bool operator!=(const Document& other){
        return !(*this == other);
    }
    
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
