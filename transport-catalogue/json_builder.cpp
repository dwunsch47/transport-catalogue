#include <vector>
#include <string>
#include <utility>
#include <stdexcept>

#include "json.h"
#include "json_builder.h"

using namespace std;

namespace json {
    
KeyItemContext Builder::Key(string key) {
    if (root_ == nullptr)
    {
        throw std::logic_error("Key() called for empty document.");
    }
    else if (nodes_stack_.empty())
    {
        
        throw std::logic_error("Key() called outside of any container element.");
    }
    
    if (nodes_stack_.back()->IsDict() && !key_opened_) {
        current_key_ = key;
        key_opened_ = true;
    } else {
        throw logic_error("Key used outside Dict"s);
    }
    return *this;
}
    
BaseContext Builder::Value(Node::Value value) {
    if (root_ == nullptr) {
        root_.GetValue() = move(value);
        return *this;
    }
    
    if (nodes_stack_.empty()) {
        throw logic_error("Cannot add value outside of Dict or Array"s);
    }
    
    Node* parent = nodes_stack_.back();
    
    if (parent->IsArray()) {
        parent->AsArray().push_back(move(value));
    } else if (parent->IsDict()) {
        if (key_opened_) {
            parent->AsDict()[current_key_] = move(value);
            //current_key_.clear();
            key_opened_ = false;
        } else {
            throw logic_error("No key was opened"s);
        }
    } else {
        throw logic_error("Unknown container"s);
    }
    
    return *this;
}
    
DictValueItemContext Builder::StartDict() {
    if (root_ == nullptr) {
        root_ = Dict();
        nodes_stack_.push_back(&root_);
        return *this;
    }
    if (nodes_stack_.empty()) {
        throw logic_error("Cannot construct dict"s);
    }
    
    Node* parent = nodes_stack_.back();
    
    if (parent->IsArray()) {
        parent->AsArray().push_back(Dict{});
        nodes_stack_.push_back(&parent->AsArray().back());
    } else if (parent->IsDict()) {
        if (key_opened_) {
            parent->AsDict()[current_key_] = Dict{};
            nodes_stack_.push_back(&parent->AsDict().at(current_key_));
            //current_key_.clear();
            key_opened_ = false;
        } else {
            throw logic_error("No opened key"s);
        }
    } else {
        throw logic_error("Cannot create Dict here"s);
    }
    return *this;
}
    
ArrayItemContext Builder::StartArray() {
    if (root_ == nullptr) {
        root_ = Array();
        nodes_stack_.push_back(&root_);
        return *this;
    }
    
    if (nodes_stack_.empty()) {
        throw logic_error("Cannot construct dict"s);
    }
    
    Node* parent = nodes_stack_.back();
    
    if (parent->IsArray()) {
        parent->AsArray().push_back(Array{});
        nodes_stack_.push_back(&parent->AsArray().back());
    } else if (parent->IsDict()) {
        if (key_opened_) {
            parent->AsDict()[current_key_] = Array{};
            nodes_stack_.push_back(&parent->AsDict().at(current_key_));
            //current_key_.clear();
            key_opened_ = false;
        } else {
            throw logic_error("No opened key"s);
        }
    } else {
        throw logic_error("Cannor create Array here"s);
    }
    return *this;
}
    
BaseContext Builder::EndDict() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !key_opened_) {
        nodes_stack_.pop_back();
    } else {
        throw logic_error("Cannot EndDict() a non-Dict object"s);
    }
    return *this;
}
    
BaseContext Builder::EndArray() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        nodes_stack_.pop_back();
    } else {
        throw logic_error("Cannon EndArray() a non-Array object"s);
    }
    return *this;
}
    
Node Builder::Build() {
    if (nodes_stack_.empty() && root_ != nullptr) {
        return root_;
    } else {
        throw logic_error("Cannot build incomplete node"s);
    }
}
    
KeyItemContext BaseContext::Key(std::string key) {
    return builder_.Key(move(key));
}
BaseContext BaseContext::Value(Node::Value value) {
    return builder_.Value(move(value));
}  
DictValueItemContext BaseContext::StartDict() {
    return builder_.StartDict();
}  
ArrayItemContext BaseContext::StartArray() {
    return builder_.StartArray();
}
BaseContext BaseContext::EndDict() {
    return builder_.EndDict();
}
BaseContext BaseContext::EndArray() {
    return builder_.EndArray();
}
Node BaseContext::Build() {
    return builder_.Build();
}
    
DictValueItemContext KeyItemContext::Value(Node::Value value) {
    return BaseContext::Value(move(value));
}
DictValueItemContext KeyItemContext::StartDict() {
    return BaseContext::StartDict();
}
ArrayItemContext KeyItemContext::StartArray() {
    return BaseContext::StartArray();
}
    
KeyItemContext DictValueItemContext::Key(std::string key) {
    return BaseContext::Key(move(key));
}
    
ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return BaseContext::Value(move(value));
}
DictValueItemContext ArrayItemContext::StartDict() {
    return BaseContext::StartDict();
}
ArrayItemContext ArrayItemContext::StartArray() {
    return BaseContext::StartArray();
}
    
}