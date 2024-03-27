#include "json_builder.h"
namespace json {
    Builder::Builder()
    {
        root_ = nullptr;
        nodes_stack_.emplace_back(&root_);
    }
    Builder::DictValueContext Builder::Key(std::string key)
    {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Объект уже готов");
        }
        if (!std::holds_alternative<Dict>(nodes_stack_.back()->GetValue())) {
            throw std::logic_error("Ключ раньше чем построение словаря");
        }
        nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetValue())[std::move(key)]);
        return BaseContext(*this);
    }

    Builder::BaseContext Builder::Value(Node::Value value)
    {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Объект уже готов");
        }
        if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
            Node elem;
            elem.GetValue() = value;
            std::get<Array>(nodes_stack_.back()->GetValue()).push_back(elem);
            return BaseContext(*this);
        }
        if (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue())) {
            nodes_stack_.back()->GetValue() = value;
            nodes_stack_.pop_back();
            return BaseContext(*this);
        }
        throw std::logic_error("Нет места");
    }

    Builder::DictItemContext Builder::StartDict()
    {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Объект уже готов");
        }
        if (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue())) {
            nodes_stack_.back()->GetValue() = Dict{};
            return BaseContext(*this);
        }
        else if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
            auto& emplaced = std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Dict{});
            nodes_stack_.emplace_back(&emplaced);
            return BaseContext(*this);
        }
        throw std::logic_error("Не словарь");
    }

    Builder::ArrayItemContext Builder::StartArray()
    {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Объект уже готов");
        }
        if (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue())) {
            nodes_stack_.back()->GetValue() = Array{};
            return BaseContext(*this);
        }
        else if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
            auto& emplaced = std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Array{});
            nodes_stack_.emplace_back(&emplaced);
            return BaseContext(*this);
        }
        throw std::logic_error("Не список");
    }

    Builder::BaseContext Builder::EndDict()
    {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Объект уже готов");
        }
        if (!std::holds_alternative<Dict>(nodes_stack_.back()->GetValue())) {
            throw std::logic_error("Не словарь");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::BaseContext Builder::EndArray()
    {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Объект уже готов");
        }
        if (!std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
            throw std::logic_error("Не список");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build()
    {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Еще нельзя завершать построение объекта");
        }
        return root_;
    }




Builder::BaseContext::BaseContext(Builder& builder):builder_(builder){
}

Node Builder::BaseContext::Build()
{
    return builder_.Build();
}

Builder::DictValueContext Builder::BaseContext::Key(std::string key)
{
    return builder_.Key(std::move(key));
}

Builder::BaseContext Builder::BaseContext::Value(Node::Value value)
{
    if (builder_.nodes_stack_.empty()) {
        throw std::logic_error("Объект уже закончен");
    }
    if (std::holds_alternative<Array>(builder_.nodes_stack_.back()->GetValue())) {
        Node elem;
        elem.GetValue() = value;
        std::get<Array>(builder_.nodes_stack_.back()->GetValue()).push_back(elem);
        return BaseContext(*this);
    }
    if (std::holds_alternative<std::nullptr_t>(builder_.nodes_stack_.back()->GetValue())) {
        builder_.nodes_stack_.back()->GetValue() = value;
        builder_.nodes_stack_.pop_back();
        return BaseContext(*this);
    }
    throw std::logic_error("Нет места");
}

Builder::DictItemContext Builder::BaseContext::StartDict()
{
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::BaseContext::StartArray()
{
    return builder_.StartArray();
}

Builder::BaseContext Builder::BaseContext::EndDict()
{
    return builder_.EndDict();
}

Builder::BaseContext Builder::BaseContext::EndArray()
{
    return builder_.EndArray();
}

Builder::DictValueContext::DictValueContext(BaseContext base) : BaseContext(base){
}

Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value)
{
        return BaseContext(*this).Value(std::move(value));
}

Builder::DictItemContext::DictItemContext(BaseContext base):BaseContext(base){
}

Builder::ArrayItemContext::ArrayItemContext(BaseContext base):BaseContext(base){
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value){
    return BaseContext(*this).Value(value);
}

}
