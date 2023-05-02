#include "json.h"

using namespace std;

namespace json {

namespace {
    
Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input)
    {
        throw ParsingError("Array parsing error"s);
    }

    return Node(move(result));
}


Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input)
{
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true)
    {
        if (it == end)
        {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"')
        {
            ++it;
            break;
        }
        else if (ch == '\\')
        {
            ++it;
            if (it == end)
            {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char)
            {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            throw ParsingError("Unexpected end of line"s);
        }
        else
        {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}
    
Node LoadBool(istream& input) {
    string result;
    char c = input.peek();
    int num_of_reads = (c == 't' ? 4 : 5);

    for (int i = 0; i < num_of_reads; ++i) {
        if(input.get(c)) {
            result += c;
        }
    }

    if (result != "true"s && result != "false"s) {
        throw ParsingError("Failed to parse bool");
    }
    
    return (result == "true" ? Node(true) : Node(false));
}
    
Node LoadNull(istream& input) {
    string result;
    char c;
    for (int i = 0; i < 4; ++i) {
        if (input.get(c)) {
            result += c;
        }
    }
    
    if (result != "null") {
        throw ParsingError("Failed to parse null");
    }
    
    return Node(nullptr);
}

Node LoadDict(std::istream& input)
{
    Dict dict;

    for (char c; input >> c && c != '}';)
    {
        if (c == '"')
        {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':')
            {
                if (dict.find(key) != dict.end())
                {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            }
            else
            {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        }
        else if (c != ',')
        {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input)
    {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == '-' || isdigit(c)) {
        input.putback(c);
        return LoadNumber(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        throw ParsingError("Cannot parse document"s);
    }
}

}  // namespace

Node::Node(Value value) : variant(move(value)) {}
    
bool Node::IsInt() const {
    return holds_alternative<int>(*this);
}
    
bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this);
}
    
bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}
bool Node::IsBool() const {
    return holds_alternative<bool>(*this);
}
bool Node::IsString() const {
    return holds_alternative<string>(*this);
}
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(*this);
}
bool Node::IsArray() const {
    return holds_alternative<Array>(*this);
}
bool Node::IsMap() const {
    return holds_alternative<Dict>(*this);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("not an int"s);
    }
    return get<int>(*this);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("not a bool"s);
    }
    return get<bool>(*this);
}
    
double Node::AsDouble() const {
    if (!IsDouble()) {
        throw logic_error("not a double or int"s);
    }
    if (IsPureDouble()) {
        return get<double>(*this);
    } else {
        return static_cast<double>(get<int>(*this));
    }
}
    
const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("not a string"s);
    }
    return get<string>(*this);
}
    
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("not an array"s);
    }
    return get<Array>(*this);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("not a dict"s);
    }
    return get<Dict>(*this);
}
    
Node::Value& Node::GetValue() {
    return *this;
}
    
const Node::Value& Node::GetValue() const {
    return *this;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}
    
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};
    
void PrintNode(const Node& value, const PrintContext& ctx); 

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx)
{
    ctx.out << value;
}

void PrintString(const std::string& value, std::ostream& out)
{
    out.put('"');
        for (const char c : value) {
            switch (c) {
            case '"':
                out << "\\\""sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\t"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out << c;
                break;
            }
    }
    out.put('"');
}

void PrintValue(const std::string& value, const PrintContext& ctx)
{
    PrintString(value, ctx.out);
}

void PrintValue(std::nullptr_t, const PrintContext& ctx)
{
    ctx.out << "null"sv;
}

void PrintValue(bool value, const PrintContext& ctx)
{
    ctx.out << (value ? "true"sv : "false"sv);
}

void PrintValue(const Array& nodes, const PrintContext& ctx)
{
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

void PrintValue(const Dict& nodes, const PrintContext& ctx)
{
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx)
{
    std::visit(
        [&ctx](const auto& value)
        {
            PrintValue(value, ctx);
        },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output)
{
    PrintNode(doc.GetRoot(), PrintContext{ output });
}

}  // namespace json