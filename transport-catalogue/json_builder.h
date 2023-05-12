#pragma once

#include <string>
#include <vector>

#include "json.h"

namespace json {
    
class BaseContext;
class KeyItemContext;
class ValueAfterKeyContext;
class DictValueItemContext;
class ArrayItemContext;
    
class Builder {
public:
    Builder() = default;
    KeyItemContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictValueItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::string current_key_;
    bool key_opened_ = false;
};
    
class BaseContext {
public:
    BaseContext(Builder& builder) : builder_(builder) {}
    
    KeyItemContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictValueItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();
private:
    Builder& builder_;
};
    
class KeyItemContext : public BaseContext {
public:
    KeyItemContext(Builder& builder) : BaseContext(builder) {}
    
    DictValueItemContext Value(Node::Value value);
    DictValueItemContext StartDict();
    ArrayItemContext StartArray();
    
    KeyItemContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
    
class DictValueItemContext : public BaseContext {
public:
    DictValueItemContext(Builder& builder) : BaseContext(builder) {}
    DictValueItemContext(BaseContext base) : BaseContext(base) {}
    
    KeyItemContext Key(std::string key);
    
    BaseContext Value(Node::Value value) = delete;
    DictValueItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
    
class ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {}
    ArrayItemContext(BaseContext base) : BaseContext(base) {}
 
    ArrayItemContext Value(Node::Value value);
    DictValueItemContext StartDict();
    ArrayItemContext StartArray();
    
    KeyItemContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    Node Build() = delete;
};
}
