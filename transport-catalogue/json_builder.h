#include "json.h"
#include <vector>

namespace json {
class Builder
{
    class BaseContext;
    class DictValueContext;
    class DictItemContext;
    class ArrayItemContext;
    
public:
    Builder();
    DictValueContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();

private:
    Node root_;
    std::vector <Node*> nodes_stack_;

    class BaseContext {
    public:
        BaseContext(Builder& builder);
        Node Build();
        DictValueContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();

    private:
        Builder& builder_;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base);
        Node Build() = delete;
        BaseContext Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        BaseContext EndArray() = delete;
    };

    class DictValueContext :public BaseContext {
    public:
        DictValueContext(BaseContext base);
        Node Build() = delete;
        DictItemContext Value(Node::Value value);
        BaseContext EndArray() = delete;
        BaseContext EndDict() = delete;
        DictValueContext Key(std::string key) = delete;
    };


    class ArrayItemContext :public BaseContext {
    public:
        ArrayItemContext(BaseContext base);
        Node Build() = delete;
        ArrayItemContext Value(Node::Value value);
        BaseContext EndDict() = delete;
        DictValueContext Key(std::string key)=delete;
    };



};
}
