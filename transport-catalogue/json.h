#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <initializer_list>
#include <iomanip>
#include <algorithm>

namespace json {

	class Node;
	// Сохраните объявления Dict и Array без изменения
	using Dict = std::map<std::string, Node>;
	using Map = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using NodeData = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Map>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node {
	public:
		/* Реализуйте Node, используя std::variant */

		Node();
		Node(nullptr_t);
		Node(int num);
		Node(double num);
		Node(std::string text);
		Node(bool boo);
		Node(Array array);
		Node(Map map);
		Node(NodeData data);

		bool IsInt() const;
		bool IsDouble() const;// Возвращает true, если в Node хранится int либо double.
		bool IsPureDouble() const;// Возвращает true, если в Node хранится double.
		bool IsBool() const;
		bool IsString() const;
		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;

		int AsInt() const;
		bool AsBool() const;
		double AsDouble() const;// .Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
		const std::string& AsString() const;
		const Array& AsArray() const;
		const Map& AsMap() const;

		int VarIndex() const;
		NodeData& AccessData();
		const NodeData& ReadData() const;
	private:
		NodeData data_;
	};

	bool operator==(const Node f, const Node s);

	bool operator!=(const Node f, const Node s);

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	bool operator==(const Document f, const Document s);

	bool operator!=(const Document f, const Document s);

	Document Load(std::istream& input);

	inline std::ostream& operator<<(std::ostream& out, const Node node);

	struct OstreamPrinter {
		std::ostream& out;

		void operator()(nullptr_t) const;
		void operator()(int num) const;
		void operator()(double num) const;
		void operator()(std::string text) const;
		void operator()(bool boo) const;
		void operator()(Array array) const;
		void operator()(Map map) const;
	};

	std::ostream& operator<<(std::ostream& out, Node node);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json