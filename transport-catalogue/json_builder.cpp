#include "json_builder.h"

using namespace json;

BuilderRaw& json::BuilderRaw::Key(std::string key)
{
	if (nodes_stack_.size() != 0 && nodes_stack_.back()->IsMap()) {
		if (!last_key_.second) {
			last_key_.first = key;
			//Dict& buf = std::get<Dict>(nodes_stack_.back()->AccessData());
			//buf.insert({ key,{}});
			//buf[key] = {};
			last_key_.second = true;
		}
		else {
			throw std::logic_error("key after key");
		}
	}
	else {
		throw std::logic_error("Not a directory");
	}
	return *this;
}

BuilderRaw& json::BuilderRaw::Value(json::NodeData data)
{
	if (root_.IsNull()) {
		root_.AccessData() = data;
	}
	else if (nodes_stack_.size() == 0) {
		throw std::logic_error("No space for Value");
	}
	else if (last_key_.second && nodes_stack_.back()->IsMap()) {
		auto& buf1 = std::get<Map>(nodes_stack_.back()->AccessData());
		buf1[last_key_.first].AccessData() = data;
		//last_key_.first.clear();
		last_key_.second = false;
	}
	else if (nodes_stack_.back()->IsArray()) {
		auto& buf1 = std::get<Array>(nodes_stack_.back()->AccessData());
		buf1.push_back({});
		buf1.back().AccessData() = data;
	}
	else {
		throw std::logic_error("Value error");
	}
	return *this;
}

BuilderRaw& json::BuilderRaw::StartDict()
{
	if (root_.IsNull()) {
		root_.AccessData().emplace<Dict>();
		nodes_stack_.push_back(&root_);
	}
	else if (nodes_stack_.size() == 0) {
		throw std::logic_error("No space for Dict");
	}
	else if (last_key_.second && nodes_stack_.back()->IsMap()) {
		auto& buf1 = std::get<Dict>(nodes_stack_.back()->AccessData());
		buf1[last_key_.first].AccessData().emplace<Dict>();
		auto& buf2 = buf1[last_key_.first];
		//last_key_.clear();
		last_key_.second = false;
		nodes_stack_.push_back(&buf2);
	}
	else if (nodes_stack_.back()->IsArray()) {
		auto& buf1 = std::get<Array>(nodes_stack_.back()->AccessData());
		buf1.push_back({});
		buf1.back().AccessData().emplace<Dict>();
		auto& buf2 = buf1.back();
		nodes_stack_.push_back(&buf2);
	}
	else {
		throw std::logic_error("StartDict error");
	}
	return *this;
}

BuilderRaw& json::BuilderRaw::StartArray()
{
	if (root_.IsNull()) {
		root_.AccessData().emplace<Array>();
		nodes_stack_.push_back(&root_);
	}
	else if (nodes_stack_.size() == 0) {
		throw std::logic_error("No space for Array");
	}
	else if (last_key_.second && nodes_stack_.back()->IsMap()) {
		auto& buf1 = std::get<Dict>(nodes_stack_.back()->AccessData());
		buf1[last_key_.first].AccessData().emplace<Array>();
		auto& buf2 = buf1[last_key_.first];
		//last_key_.clear();
		last_key_.second = false;
		nodes_stack_.push_back(&buf2);
	}
	else if (nodes_stack_.back()->IsArray()) {
		auto& buf1 = std::get<Array>(nodes_stack_.back()->AccessData());
		buf1.push_back({});
		buf1.back().AccessData().emplace<Array>();

		auto& buf2 = buf1.back();
		nodes_stack_.push_back(&buf2);
	}
	else {
		throw std::logic_error("StartArray error");
	}
	return *this;
}

BuilderRaw& json::BuilderRaw::EndDict()
{
	if (nodes_stack_.size() == 0 || !nodes_stack_.back()->IsMap()) {
		throw std::logic_error("Not found Dict");
	}
	else if (last_key_.second) {
		throw std::logic_error("EndDict before Value for Key");
	}
	nodes_stack_.pop_back();
	return *this;
}

BuilderRaw& json::BuilderRaw::EndArray()
{
	if (nodes_stack_.size() == 0 || !nodes_stack_.back()->IsArray()) {
		throw std::logic_error("Not found Array");
	}
	nodes_stack_.pop_back();
	return *this;
}

json::Node json::BuilderRaw::Build()
{
	if (root_.IsNull()) {
		throw std::logic_error("Root empty");
	}
	else if (nodes_stack_.size() != 0) {
		throw std::logic_error("Container not ended");
	}
	else if (last_key_.second) {
		throw std::logic_error("Key isnt used");
	}
	return root_;
}

json::KeyContext json::BuilderSupport::Key(std::string text) {
	builder_raw_.Key(text);
	return *this;
}

json::ValueContext json::BuilderSupport::Value(json::NodeData data) {
	builder_raw_.Value(data);
	return *this;
}

json::StartDictContext json::BuilderSupport::StartDict() {
	builder_raw_.StartDict();
	return *this;
}

json::StartArrayContext json::BuilderSupport::StartArray() {
	builder_raw_.StartArray();
	return *this;
}

json::EndDictContext json::BuilderSupport::EndDict() {
	builder_raw_.EndDict();
	return *this;
}

json::EndArrayContext json::BuilderSupport::EndArray() {
	builder_raw_.EndArray();
	return *this;
}

json::Node json::BuilderSupport::Build() {
	return (builder_raw_.Build());
}

ValueArrayContext json::StartArrayContext::Value(json::NodeData data)
{
	return ValueArrayContext(builder_raw_.Value(data));
}

ValueDictContext json::KeyContext::Value(json::NodeData data)
{
	return ValueDictContext(builder_raw_.Value(data));
}

ValueArrayContext json::ValueArrayContext::Value(json::NodeData data)
{
	builder_raw_.Value(data);
	return *this;
}