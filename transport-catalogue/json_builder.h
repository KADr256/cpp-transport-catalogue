#pragma once
#include "json.h"

namespace json {
	class BuilderRaw {
	public:
		BuilderRaw() = default;

		BuilderRaw& Key(std::string);
		BuilderRaw& Value(json::NodeData);
		BuilderRaw& StartDict();
		BuilderRaw& StartArray();
		BuilderRaw& EndDict();
		BuilderRaw& EndArray();
		json::Node Build();

	private:
		json::Node root_;
		std::vector<Node*> nodes_stack_;
		std::pair<std::string, bool> last_key_;
	};

	class KeyContext;

	class ValueContext;
	class ValueArrayContext;
	class ValueDictContext;

	class StartDictContext;
	class StartArrayContext;
	class EndDictContext;
	class EndArrayContext;

	class BuilderSupport {
	public:
		BuilderSupport(BuilderRaw& builder_raw) :builder_raw_(builder_raw) {};
		KeyContext Key(std::string text);
		ValueContext Value(json::NodeData data);
		StartDictContext StartDict();
		StartArrayContext StartArray();
		EndDictContext EndDict();
		EndArrayContext EndArray();
		json::Node Build();

		BuilderRaw& builder_raw_;
	};

	class KeyContext : public BuilderSupport
	{
	public:
		KeyContext(BuilderSupport bs) :BuilderSupport(bs.builder_raw_) {};
		ValueDictContext Value(json::NodeData data);

		KeyContext Key(std::string text) = delete;
		EndDictContext EndDict() = delete;
		EndArrayContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class ValueContext : public BuilderSupport
	{
	public:
		ValueContext(BuilderSupport bs) :BuilderSupport(bs.builder_raw_) {};
	};

	class ValueArrayContext : public ValueContext
	{
	public:
		ValueArrayContext(BuilderSupport bs) :ValueContext(bs.builder_raw_) {};
		ValueArrayContext Value(json::NodeData data);

		KeyContext Key(std::string text) = delete;
		EndDictContext EndDict() = delete;
		json::Node Build() = delete;
	};

	class ValueDictContext : public ValueContext
	{
	public:
		ValueDictContext(BuilderSupport bs) :ValueContext(bs.builder_raw_) {};

		ValueDictContext Value(json::NodeData data) = delete;
		StartArrayContext StartArray() = delete;
		EndArrayContext EndArray() = delete;
		StartDictContext StartDict() = delete;
		json::Node Build() = delete;
	};

	class StartDictContext : public BuilderSupport
	{
	public:
		StartDictContext(BuilderSupport bs) :BuilderSupport(bs.builder_raw_) {};

		ValueContext Value(json::NodeData data) = delete;
		StartDictContext StartDict() = delete;
		StartArrayContext StartArray() = delete;
		EndArrayContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class StartArrayContext : public BuilderSupport
	{
	public:
		StartArrayContext(BuilderSupport bs) :BuilderSupport(bs.builder_raw_) {};
		ValueArrayContext Value(json::NodeData data);

		KeyContext Key(std::string text) = delete;
		BuilderSupport EndDict() = delete;
		json::Node Build() = delete;
	};

	class EndDictContext : public BuilderSupport
	{
	public:
		EndDictContext(BuilderSupport bs) :BuilderSupport(bs.builder_raw_) {};
	};

	class EndArrayContext : public BuilderSupport
	{
	public:
		EndArrayContext(BuilderSupport bs) :BuilderSupport(bs.builder_raw_) {};
	};

	class Builder {
	public:
		Builder() :bs(br) {};

		KeyContext Key(std::string data) {
			bs.Key(data);
			return bs;
		}
		ValueContext Value(json::NodeData data) {
			bs.Value(data);
			return bs;
		}
		StartDictContext StartDict() {
			bs.StartDict();
			return bs;
		}
		StartArrayContext StartArray() {
			bs.StartArray();
			return bs;
		}
		EndDictContext EndDict() {
			bs.EndDict();
			return bs;
		}
		EndArrayContext EndArray() {
			bs.EndArray();
			return bs;
		}
		json::Node Build() {
			return bs.Build();
		}

		BuilderRaw br;
		BuilderSupport bs;
	};
}