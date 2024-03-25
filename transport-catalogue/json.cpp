#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {

    std::vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
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
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}


Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }
    return Node(result);
}

Node LoadString1(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(std::move(line));
}

Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node{nullptr};
    } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}

Node LoadBool(std::istream& input) {
    const auto s = LoadLiteral(input);
    if (s == "true"sv) {
        return Node{true};
    } else if (s == "false"sv) {
        return Node{false};
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}




std::string LoadString(std::istream& input);


Node LoadDict(istream& input) {

    Dict dict;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString1(input).AsString();
            if (input >> c && c == ':') {
                if (dict.find(key) != dict.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            }
            else {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        }
        else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}


std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
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
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    
    return s;
}

Node LoadNode(istream& input) {
    char c;
    
    
    input >> c;
    std::string num1;
    
    if(c == ']' || c =='}')
        throw ParsingError("Can't add ending brake");
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
       
        return LoadDict(input);
    } else if (c == '"') {
        return Node(LoadString(input));
    } else if(c == 'n') {
       
        input.putback(c);
        return LoadNull(input);

        
    }
    else if(c == 't')
    {
        //return Node{true};
        input.putback(c);
        return LoadBool(input);

    }
    else if(c == 'f')
    {
        input.putback(c);
        return LoadBool(input);

    }
    else if((isdigit(c)) || c == '+' || c =='-')
    {
        input.putback(c);
        auto num = LoadNumber(input);
        if(std::holds_alternative<int>(num) == true)
            return Node(std::get<int>(num));
        else
            return Node(std::get<double>(num));
    }
    else{
        input.putback(c);
        return LoadInt(input);
    }
    return Node();
}


}  // namespace

std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
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
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    
    return s;
}



Node::Node(Array array)
: value_(std::move(array)) {
    
}

Node::Node(Dict map)
:  value_(std::move(map)) {
}

Node::Node(int value)
:  value_(std::move(value)) {
}
Node::Node(double value)
: value_(std::move(value)) {
}

Node::Node(string value)
: value_(std::move(value)) {
}
Node::Node(std::nullptr_t null)
: value_(std::move(null)){}

Node::Node(bool log)
: value_(std::move(log)){}


void PrintNode(const Node &node, std::ostream &out);

template<typename Value>
void PrintValue(const Value& val, std::ostream& out) {
    out << val;
}

template<>
void PrintValue<std::string>(const std::string& str, std::ostream& out) {
   
    out<<"\"";
 
    for(int i =0; i < static_cast<int>(str.size());i++)
    {
        
        if (str[i] == '"') {
            out << "\\";
            out << str[i];
    
        }
        else if(str[i] == '\r'){
            out << "\\r";
     
        }
        else if(str[i] == '\n')
        {
            out<<"\\n";
     
        }
        else if(str[i] == '\t')
        {
            out<<"\\t";
         
        }
        else if(str[i] == '\\')
        {
            out<<"\\";
            out<<str[i];
            
       
        }
        else {
            out << str[i];
          
        }

    }
    
    out<<"\"";
    

}



void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}


void PrintValue(bool value, std::ostream& out) {
//    out << (value ? "true"sv : "false"sv);
    PrintValue(value ? "true"sv : "false", out);
}

template<>
void PrintValue<Array>(const Array &arr, std::ostream &out) {
    out<<"[";

    bool first = true;
    for(auto const& node :arr){
        if (first)
        {
            first = false;
            PrintNode(node, out);
        }
        else
        {
            
            out << ","sv;
            PrintNode(node, out);
        }
        
    }
    out<<"]";
   
}

template<>
void PrintValue<Dict>(const Dict& dict, std::ostream& out) {
    

    out << "{"sv;
    bool first = true;
    //auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : dict) {
        if (first) {
            first = false;
        }
        else {
            out << ","sv;
        }
        
        PrintValue(key, out);
        out << ": "sv;
        PrintNode(node, out);
    }

    out.put('}');
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit([&out](const auto &value) { PrintValue(value, out); }, node.GetValue());

}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
int Node::AsInt() const {
    using namespace std::literals;
    if (!IsInt()) {
        throw std::logic_error("Not an int"s);
    }
    return std::get<int>(value_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}
bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}
double Node::AsDouble() const {
    using namespace std::literals;
    if (!IsDouble()) {
        throw std::logic_error("Not a double"s);
    }
    return IsPureDouble() ? std::get<double>(value_) : AsInt();
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}
bool Node::AsBool() const {
    using namespace std::literals;
    if (!IsBool()) {
        throw std::logic_error("Not a bool"s);
    }

    return std::get<bool>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}
const Array& Node::AsArray() const {
    using namespace std::literals;
    if (!IsArray()) {
        throw std::logic_error("Not an array"s);
    }

    return std::get<Array>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}
const std::string& Node::AsString() const {
    using namespace std::literals;
    if (!IsString()) {
        throw std::logic_error("Not a string"s);
    }

    return std::get<std::string>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}
const Dict& Node::AsMap() const {
    using namespace std::literals;
    if (!IsMap()) {
        throw std::logic_error("Not a map"s);
    }

    return std::get<Dict>(value_);
}




Document::Document(Node root)
: root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    (void) &doc;
    (void) &output;
    auto node = doc.GetRoot();
    
    PrintNode(node, output);
    // Реализуйте функцию самостоятельно
}


}  // namespace json

