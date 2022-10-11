#include "json.h"

using namespace std;

namespace json {

	namespace {
		Node LoadNode(istream& input);

		Node LoadArray(istream& input) {
			Array result;
			char c;

			for (c = input.peek(); input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
				c = c;
			}
			if (c != ']') {
				throw ParsingError("");
			}
			return Node(move(result));
		}

		Node LoadNum(std::istream& input) {
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
			}
			else {
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
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return Node(std::stod(parsed_num));
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		/*
		Node LoadNum(istream& input) {
			std::string in;
			double res_d = 0;
			char c;
			bool doub = false;
			c = input.peek();
			while (c != ',' && c != ']' && c != '}' && c != -1) {
				in += input.get();
				c = input.peek();
			}
			res_d = std::stod(in);
			if (std::count(in.begin(), in.end(), '.')) {
				doub = true;
			}
			if (doub == true) {
				return Node(res_d);
			}
			return Node(static_cast<int>(res_d));
		}
		*/

		Node LoadString(std::istream& input) {
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
				}
				else if (ch == '\\') {
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
				}
				else if (ch == '\n' || ch == '\r') {
					// Строковый литерал внутри- JSON не может прерываться символами \r или \n
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					// Просто считываем очередной символ и помещаем его в результирующую строку
					s.push_back(ch);
				}
				++it;
			}
			return Node(s);
		}

		Node LoadDict(istream& input) {
			Dict result;
			char c;

			for (c = input.peek(); input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}
				string key = LoadString(input).AsString();
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}

			if (c != '}') {
				throw ParsingError("");
			}

			return Node(move(result));
		}

		Node LoadNode(istream& input) {
			char c;
			input >> c;

			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (c == 'n') {
				std::string buf = "n";
				buf += input.get();
				buf += input.get();
				buf += input.get();
				if (buf == "null") {
					Node node = { nullptr };
					return node;
				}
				else {
					throw ParsingError("");
				}
			}
			else if (c == 't') {
				std::string buf = "t";
				buf += input.get();
				buf += input.get();
				buf += input.get();
				if (buf == "true") {
					Node node = { true };
					return node;
				}
				else {
					throw ParsingError("");
				}
			}
			else if (c == 'f') {
				std::string buf = "f";
				buf += input.get();
				buf += input.get();
				buf += input.get();
				buf += input.get();
				if (buf == "false") {
					Node node = { false };
					return node;
				}
				else {
					throw ParsingError("");
				}
			}
			else {
				input.putback(c);
				return LoadNum(input);
			}
		}

	}  // namespace

	Node::Node() {
		data_ = nullptr;
	}
	Node::Node(nullptr_t) {
		data_ = nullptr;
	}
	Node::Node(int num) :data_(num) {
	}
	Node::Node(double num) : data_(num) {
	}
	Node::Node(std::string text) : data_(text) {
	}
	Node::Node(bool boo) : data_(boo) {
	}
	Node::Node(Array array) : data_(array) {
	}
	Node::Node(Map map) : data_(map) {
	}
	Node::Node(NodeData data) : data_(data) {
	}

	bool Node::IsInt() const {
		return std::holds_alternative<int>(data_);
	}
	bool Node::IsDouble() const {
		return std::holds_alternative<int>(data_) || std::holds_alternative<double>(data_);;
	}// Возвращает true, если в Node хранится int либо double.
	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(data_);
	}// Возвращает true, если в Node хранится double.
	bool Node::IsBool() const {
		return std::holds_alternative<bool>(data_);
	}
	bool Node::IsString() const {
		return std::holds_alternative<std::string>(data_);
	}
	/*
	bool Node::IsNull() const {
		if (IsDouble()) {
			return AsDouble() == 0;
		}
		else {
			return std::holds_alternative<nullptr_t>(data_);
		}
	}
	*/
	bool Node::IsNull() const {
		return std::holds_alternative<nullptr_t>(data_);
	}
	bool Node::IsArray() const {
		return std::holds_alternative<Array>(data_);
	}
	bool Node::IsMap() const {
		return std::holds_alternative<Map>(data_);
	}

	int Node::AsInt() const {
		if (IsInt()) {
			return std::get<int>(data_);
		}
		else {
			throw std::logic_error("");
		}
	}
	bool Node::AsBool() const {
		if (IsBool()) {
			return std::get<bool>(data_);
		}
		else {
			throw std::logic_error("");
		}
	}
	double Node::AsDouble() const {
		if (IsInt()) {
			return static_cast<double>(std::get<int>(data_));
		}
		else if (IsPureDouble()) {
			return std::get<double>(data_);
		}
		else {
			throw std::logic_error("");
		}
	}// .Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
	const std::string& Node::AsString() const {
		if (IsString()) {
			return std::get<std::string>(data_);
		}
		else {
			throw std::logic_error("");
		}
	}
	const Array& Node::AsArray() const {
		if (IsArray()) {
			return std::get<Array>(data_);
		}
		else {
			throw std::logic_error("");
		}
	}
	const Map& Node::AsMap() const { // map
		if (IsMap()) {
			return std::get<Map>(data_);
		}
		else {
			throw std::logic_error("");
		}
	}

	int Node::VarIndex() const
	{
		return data_.index();
	}

	NodeData Node::ReadData() const
	{
		return data_;
	}

	bool operator==(const Node f, const Node s)
	{
		int buf = f.VarIndex();
		if (f.VarIndex() == s.VarIndex()) {
			switch (buf)
			{
			case 0:
				return true;
			case 1:
				return f.AsInt() == s.AsInt();
			case 2:
				return f.AsDouble() == s.AsDouble();
			case 3:
				return f.AsString() == s.AsString();
			case 4:
				return f.AsBool() == s.AsBool();
			case 5:
				return f.AsArray() == s.AsArray();
			case 6:
				return f.AsMap() == s.AsMap();
			default:
				return false;
			}
		}
		return false;
	}

	bool operator!=(const Node f, const Node s)
	{
		return !(f == s);
	}

	Document::Document(Node root)
		: root_(move(root)) {
	}

	bool operator==(const Document f, const Document s) {
		return f.GetRoot() == s.GetRoot();
	}

	bool operator!=(const Document f, const Document s) {
		return f.GetRoot() != s.GetRoot();
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		(void)&doc;
		(void)&output;
		std::visit(OstreamPrinter{ output }, doc.GetRoot().ReadData());
	}

	std::ostream& operator<<(std::ostream& out, Node node) {
		visit(OstreamPrinter{ out }, node.ReadData());
		return out;
	}

	void OstreamPrinter::operator()(nullptr_t) const {
		out << (std::string)"null";
	}
	void OstreamPrinter::operator()(int num) const {
		out << num;
	}
	void OstreamPrinter::operator()(double num) const {
		out << std::setprecision(6);
		out << num;
	}
	void OstreamPrinter::operator()(std::string text) const {
		out << "\"";
		size_t buf1 = 0;
		size_t buf2 = text.find_first_of("\"\\\r\n", buf1);
		if (buf2 > text.size() - 1) {
			out << text;
		}
		else {
			while (buf2 != text.size() - 1) {
				buf2 = text.find_first_of("\"\\\r\n", buf1);
				out << text.substr(buf1, buf2 - buf1);
				if (text[buf2] == '\"') {
					out << "\\" << "\"";
				}
				else if (text[buf2] == '\\') {
					out << "\\" << "\\";
				}
				else if (text[buf2] == '\r') {
					out << "\\" << "r";
				}
				else if (text[buf2] == '\n') {
					out << "\\" << "n";
				}
				buf1 = buf2 + 1;
			}
		}
		out << "\"";
	}
	void OstreamPrinter::operator()(bool boo) const {
		out << std::boolalpha;
		out << boo;
	}
	void OstreamPrinter::operator()(Array array) const {
		bool first = true;
		out << "[";
		for (auto& part : array) {
			if (first) {
				out << part;
				first = false;
				continue;
			}
			out << "," << part;
		}
		out << "]";
	}
	void OstreamPrinter::operator()(Map map) const {
		bool first = true;
		out << "{";
		for (auto& part : map) {
			if (first) {
				out << " \"" << part.first << "\"" << ": " << part.second;
				first = false;
				continue;
			}
			out << "," << " \"" << part.first << "\"" << ": " << part.second;
		}
		out << " }";
	}
}  // namespace json