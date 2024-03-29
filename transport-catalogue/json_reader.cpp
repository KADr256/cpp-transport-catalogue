#include "json_reader.h"

namespace TransportCatalogue {
	namespace json_processing {
		svg::Color ColorUndefine(const json::Node& node) {
			svg::Color result;
			if (node.IsString()) {
				result = node.AsString();
			}
			else {
				auto& underlayer_color_array = node.AsArray();
				size_t u_c = underlayer_color_array.size();
				switch (u_c)
				{
				case 3:
					result = svg::Rgb{ static_cast<uint16_t>(underlayer_color_array[0].AsInt()),
						static_cast<uint16_t>(underlayer_color_array[1].AsInt()),
						static_cast<uint16_t>(underlayer_color_array[2].AsInt()) };
					break;
				case 4:
					result = svg::Rgba{ static_cast<uint16_t>(underlayer_color_array[0].AsInt()),
						static_cast<uint16_t>(underlayer_color_array[1].AsInt()),
						static_cast<uint16_t>(underlayer_color_array[2].AsInt()),
						underlayer_color_array[3].AsDouble() };
					break;
				}
			}
			return result;
		}

		json::Document Read(std::istream& in) {
			return json::Load(in);
		}

		void TransportCatalogue::json_processing::JSONReaderMB::AddJSONStop(const json::Map& json_stop)
		{
			std::string stop_name = json_stop.at("name").AsString();
			geo::Coordinates coord = { json_stop.at("latitude").AsDouble(),json_stop.at("longitude").AsDouble() };

			catalogue.AddStop(stop_name, coord);

			if (json_stop.count("road_distances")) {
				for (auto& part : json_stop.at("road_distances").AsMap()) {
					catalogue.AddWay(stop_name, part.first, part.second.AsInt());
				};
			}
		}

		void TransportCatalogue::json_processing::JSONReaderMB::AddJSONBus(const json::Map& json_bus)
		{
			std::string bus_name = json_bus.at("name").AsString();
			detail::RouteType type;
			std::deque<std::string> bus_way;

			if (json_bus.at("is_roundtrip").AsBool()) {
				type = detail::RouteType::oneway;
			}
			else {
				type = detail::RouteType::twoway;
			}

			for (auto& stop : json_bus.at("stops").AsArray()) {
				bus_way.push_back(stop.AsString());
			}
			catalogue.AddBus(bus_name, bus_way, type);
		}

		void TransportCatalogue::json_processing::JSONReaderMB::ProcessBaseRequests()
		{
			{
				if (document_.GetRoot().AsMap().count("base_requests")) {
					auto iter = document_.GetRoot().AsMap().find("base_requests");
					if ((*iter).second.AsArray().size() != 0) {
						for (auto& part : (*iter).second.AsArray()) {
							if (part.AsMap().at("type").AsString() == "Stop") {
								AddJSONStop(part.AsMap());
							}
						}
						for (auto& part : (*iter).second.AsArray()) {
							if (part.AsMap().at("type").AsString() == "Bus") {
								AddJSONBus(part.AsMap());
							}
						}

					}
				}
			}
		}

		json::NodeData TransportCatalogue::json_processing::JSONReader::AnswerJSONStop(const json::Map& json_request) {
			std::string stop = json_request.at("name").AsString();
			if (!catalogue.KnownStop(stop)) {
				return json::Builder{}
					.StartDict()
					.Key("request_id").Value(json_request.at("id").AsInt())
					.Key("error_message").Value((std::string)"not found")
					.EndDict()
					.Build().AsMap();
			}
			auto& buses = catalogue.FindStopData(stop).buses;
			if (buses.size() == 0) {
				return json::Builder{}
					.StartDict()
					.Key("buses")
					.StartArray()
					.EndArray()
					.Key("request_id").Value(json_request.at("id").AsInt())
					.EndDict()
					.Build().AsMap();
			}
			else {
				json::Builder Answer;
				Answer.StartDict()
					.Key("buses")
					.StartArray();
				for (auto& part : buses) {
					Answer.Value(static_cast<std::string>(part));
				}
				Answer.EndArray()
					.Key("request_id").Value(json_request.at("id").AsInt())
					.EndDict();
				return Answer.Build().AsMap();
			}
		}

		json::NodeData TransportCatalogue::json_processing::JSONReader::AnswerJSONBus(const json::Map& json_request)
		{
			auto data = catalogue.ProcessBusData(json_request.at("name").AsString());
			if (data.lenght == -1) {
				return json::Builder{}.StartDict()
					.Key("request_id").Value(json_request.at("id").AsInt())
					.Key("error_message").Value((std::string)"not found")
					.EndDict()
					.Build().AsMap();
			}
			else {
				return json::Builder{}.StartDict()
					.Key("curvature").Value(data.curvatur)
					.Key("request_id").Value(json_request.at("id").AsInt())
					.Key("route_length").Value(data.lenght)
					.Key("stop_count").Value(static_cast<int>(data.stops_count))
					.Key("unique_stop_count").Value(static_cast<int>(data.unique_stops_count))
					.EndDict()
					.Build().AsMap();
			}
		}

		json::NodeData TransportCatalogue::json_processing::JSONReader::AnswerJSONRoute(const json::Map& json_request) {
			std::string stop_from = json_request.at("from").AsString();
			std::string stop_to = json_request.at("to").AsString();

			if (!catalogue.stops.count(stop_from) || !catalogue.stops.count(stop_to)) {
				return json::Builder{}.StartDict()
					.Key("request_id").Value(json_request.at("id").AsInt())
					.Key("error_message").Value((std::string)"not found")
					.EndDict()
					.Build().AsMap();
			}

			auto way = tr_.value().BuildRoute(stop_from, stop_to);

			json::Builder result{};
			result.StartDict();
			if (way.has_value()) {
				result.Key("items").StartArray();
				for (auto& el1 : way.value().edges) {
					auto& el2 = tr_->AccessEdgeData(el1);
					if (el2.type == detail::EdgeDataType::Wait) {
						result.StartDict()
							.Key("type").Value((std::string)"Wait")
							.Key("stop_name").Value((std::string)el2.stop_from.value())
							.Key("time").Value(el2.time.value())
							.EndDict();
					}
					else {
						result.StartDict()
							.Key("type").Value((std::string)"Bus")
							.Key("bus").Value((std::string)el2.bus.value())
							.Key("span_count").Value(el2.stop_count.value())
							.Key("time").Value(el2.time.value())
							.EndDict();
					}
				}
				result.EndArray()
					.Key("request_id").Value(json_request.at("id").AsInt())
					.Key("total_time").Value(way.value().weight);
			}
			else {
				result.Key("error_message").Value((std::string)"not found")
					.Key("request_id").Value(json_request.at("id").AsInt());
			}
			return result.EndDict().Build().AsMap();
		}

		SvgSetting TransportCatalogue::json_processing::JSONReaderMB::ProcessRenderSettings()
		{
			SvgSetting setting;
			if (document_.GetRoot().AsMap().count("render_settings")) {
				auto iter = document_.GetRoot().AsMap().find("render_settings");
				auto& map = (*iter).second.AsMap();
				setting.width = map.at("width").AsDouble();
				setting.height = map.at("height").AsDouble();
				setting.padding = map.at("padding").AsDouble();
				setting.line_width = map.at("line_width").AsDouble();
				setting.stop_radius = map.at("stop_radius").AsDouble();
				setting.bus_label_font_size = map.at("bus_label_font_size").AsInt();
				setting.bus_label_offset = { map.at("bus_label_offset").AsArray()[0].AsDouble(),map.at("bus_label_offset").AsArray()[1].AsDouble() };
				setting.stop_label_font_size = map.at("stop_label_font_size").AsInt();
				setting.stop_label_offset = { map.at("stop_label_offset").AsArray()[0].AsDouble(),map.at("stop_label_offset").AsArray()[1].AsDouble() };
				setting.underlayer_color = TransportCatalogue::json_processing::ColorUndefine(map.at("underlayer_color"));
				setting.underlayer_width = map.at("underlayer_width").AsDouble();
				{
					auto& color_palette = map.at("color_palette").AsArray();
					for (auto& part : color_palette) {
						setting.color_palette.push_back(TransportCatalogue::json_processing::ColorUndefine(part));
					}
				}
			}
			return setting;
		}

		detail::RoutingSettingsStorage TransportCatalogue::json_processing::JSONReaderMB::ProcessRoutingSettings()
		{
			detail::RoutingSettingsStorage result;
			if (document_.GetRoot().AsMap().count("routing_settings")) {
				auto& map = (*document_.GetRoot().AsMap().find("routing_settings")).second.AsMap();
				result = { map.at("bus_wait_time").AsDouble(),
					(map.at("bus_velocity").AsDouble() / 60.0) * 1000.0 };
			}
			if (result.bus_velocity < 0 || result.bus_wait_time < 0) {
				throw std::logic_error("");
			}
			return result;
		}

		json::NodeData TransportCatalogue::json_processing::JSONReader::AnswerJSONMap(const json::Map& map)
		{
			json::Builder Answer;
			std::stringstream buf1;
			MapRenderer map_ren = MapRenderer(svg_set,catalogue, buf1);

			Answer.StartDict()
				.Key("map").Value(buf1.str())
				.Key("request_id").Value(map.at("id").AsInt())
				.EndDict();
			return Answer.Build().AsMap();
		}

		void TransportCatalogue::json_processing::JSONReader::ProcessStatRequests() {
			if (document_.GetRoot().AsMap().count("stat_requests")) {
				auto iter = document_.GetRoot().AsMap().find("stat_requests");
				if ((*iter).second.AsArray().size() != 0) {

					json::Builder Answer;
					Answer.StartArray();
					for (auto& part : (*iter).second.AsArray()) {
						if (part.AsMap().at("type").AsString() == "Bus") {
							Answer.Value(AnswerJSONBus(part.AsMap()));
						}
						else if (part.AsMap().at("type").AsString() == "Stop") {
							Answer.Value(AnswerJSONStop(part.AsMap()));
						}
						else if (part.AsMap().at("type").AsString() == "Map") {
							Answer.Value(AnswerJSONMap(part.AsMap()));
						}
						else if (part.AsMap().at("type").AsString() == "Route") {
							Answer.Value(AnswerJSONRoute(part.AsMap()));
						}
						else {
							Answer.StartDict()
								.Key("request_id").Value(part.AsMap().at("id").AsInt())
								.Key("error_message").Value((std::string)"not found")
								.EndDict();
						}
					}
					Answer.EndArray()
						.Build();
					//out_ << std::setprecision(6);
					json::Print(json::Document(Answer.Build()), out_);
				}
			}
		}
	}
}