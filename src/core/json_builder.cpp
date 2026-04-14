#include "../../include/json_builder.h"
#include <utility>

using namespace std::literals;

namespace json
{
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
    return builder_.Value(std::move(value));
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

Builder::DictItemContext Builder::StartDict()
{
    AddObject(Dict{}, false);

    return BaseContext{*this};
}

Builder::ArrayItemContext Builder::StartArray()
{
    AddObject(Array{}, false);

    return BaseContext{*this};
}

Builder::DictValueContext Builder::Key(std::string key)
{
    if (!GetTopNode()->IsDict())
    {
        throw std::logic_error("Key() outside a dict"s);
    }

    nodes_stack_.push_back(&std::get<Dict>(GetCurrentValue())[std::move(key)]);

    return BaseContext{*this};
}

Builder::BaseContext Builder::Value(Node::Value value)
{
    AddObject(std::move(value), true);

    return *this;
}

void Builder::AddObject(Node::Value value, bool one_shot)
{
    if (GetTopNode()->IsArray())
    {
        Node &node = std::get<Array>(GetCurrentValue()).emplace_back(std::move(value));

        if (!one_shot)
        {
            nodes_stack_.push_back(&node);
        }
    }

    else
    {
        if (!GetTopNode()->IsNull())
        {
            throw std::logic_error("New object in wrong context"s);
        }

        nodes_stack_.back()->GetValue() = std::move(value);

        if (one_shot)
        {
            nodes_stack_.pop_back();
        }
    }
}

Builder::BaseContext Builder::EndDict()
{
    if (!GetTopNode()->IsDict())
    {
        throw std::logic_error("EndDict() outside a dict"s);
    }

    nodes_stack_.pop_back();

    return *this;
}

Builder::BaseContext Builder::EndArray()
{
    if (!GetTopNode()->IsArray())
    {
        throw std::logic_error("EndDict() outside an array"s);
    }

    nodes_stack_.pop_back();

    return *this;
}

Node *Builder::GetTopNode()
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }

    return nodes_stack_.back();
}

/*
    Текущее значение может быть:
    * Dict, тогда последующими ожидаются .Key().Value() или EndDict()
    * Array, тогда последующими ожидаются .Value() или EndArray()
    * nullptr (default), когда вызывается впервые или ожидается .Value()
*/
Node::Value &Builder::GetCurrentValue()
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }

    return nodes_stack_.back()->GetValue();
}

// Приводит значение Node к const и возвращает const Node::Value&
const Node::Value &Builder::GetCurrentValue() const
{
    return const_cast<Builder *>(this)->GetCurrentValue();
}

// Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами
// методов. К этому моменту для каждого Start* должен быть вызван
// соответствующий End*. При этом сам объект должен быть определён, то есть
// вызов json::Builder{}.Build() недопустим.
Node Builder::Build()
{
    if (!nodes_stack_.empty())
    {
        throw std::logic_error("Attempt to build JSON which isn't finalized"s);
    }

    return std::move(root_);
}
} // end namespace json