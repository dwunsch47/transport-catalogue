#pragma once

#include <string>
#include <vector>

#include "json.h"

namespace json {
    
class BaseContext;
class KeyItemContext;
class ValueAfterKeyContext;
class DictItemContext;
class ArrayItemContext;
class ArrayValueContext;
    
class Builder {
public:
    Builder() = default;
    KeyItemContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
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
    DictItemContext StartDict();
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
    
    ValueAfterKeyContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
    KeyItemContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
    
class ValueAfterKeyContext : public BaseContext {
public:
    ValueAfterKeyContext(Builder& builder) : BaseContext(builder) {}
    ValueAfterKeyContext(BaseContext base) : BaseContext(base) {}
    
    KeyItemContext Key(std::string key);
    
    BaseContext Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
    
class DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder) : BaseContext(builder) {}
    
    KeyItemContext Key(std::string key);
    
    BaseContext Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
    
class ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder) : BaseContext(builder) {}
 
    ArrayValueContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
    KeyItemContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    Node Build() = delete;
};
    
class ArrayValueContext : public BaseContext {
public: 
    ArrayValueContext(Builder& builder) : BaseContext(builder) {}
    ArrayValueContext(BaseContext base) : BaseContext(base) {}
    
    ArrayValueContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    
    KeyItemContext Key(std::string) = delete;
    BaseContext EndDict() = delete;
    Node Build() = delete;
};
}