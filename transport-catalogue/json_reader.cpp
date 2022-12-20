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

		void TransportCatalogue::json_processing::JSONReader::AddJSONStop(const json::Map& json_stop)
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

		void TransportCatalogue::json_processing::JSONReader::AddJSONBus(const json::Map& json_bus)
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

		void TransportCatalogue::json_processing::JSONReader::ProcessBaseRequests()
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
		/*
		void TransportCatalogue::json_processing::JSONReader::FormGraph()
		{
			//std::map<std::string_view, size_t> stop_id;
			edges_.reserve(catalogue.stops.size()*catalogue.bus.size());
			size_t counter = 0;
			for (auto& el : catalogue.stops) {
				if (el.second.buses.size() != 0) {
					stop_id_.insert({ el.first,counter++ });
				}
			}
			for (auto& el1 : catalogue.bus) {
				for (auto iter1 = el1.second.stations.begin(); iter1 != el1.second.stations.end(); ++iter1) {
					graph_.AddEdge(graph::Edge<double>{ stop_id_[*iter1] * 2, stop_id_[*iter1] * 2 + 1, routing_settings_.bus_wait_time });
					edges_.push_back({ detail::EdgeDataType::Wait,*iter1 });
					double dist = 0.0;
					int depth = -1;
					if (iter1 + 1 != el1.second.stations.end()) {
						auto last_iter = iter1;
						for (auto iter2 = iter1; iter2 != el1.second.stations.end(); ++iter2) {
							++depth;
							auto& stop_data1 = catalogue.FindStopData(*last_iter);
							auto& stop_data2 = catalogue.FindStopData(*iter2);
							if (stop_data1.another_ways.count(*iter2)) {
								dist += stop_data1.another_ways[*iter2];
							}
							else if (stop_data2.another_ways.count(*last_iter)) {
								dist += stop_data2.another_ways[*last_iter];
							}
							else {
								dist += geo::ComputeDistance(stop_data1.coord, stop_data2.coord);
							}
							double copy = dist / routing_settings_.bus_velocity;
							graph_.AddEdge(graph::Edge<double>{ stop_id_[*iter1] * 2 + 1, stop_id_[*iter2] * 2, copy});
							edges_.push_back({detail::EdgeDataType::Bus,std::nullopt,depth,el1.first,copy});
							last_iter = iter2;
						}
					}
				}
				if (el1.second.type == detail::RouteType::twoway) {
					for (auto iter3 = el1.second.stations.rbegin(); iter3 != el1.second.stations.rend(); ++iter3) {
						graph_.AddEdge(graph::Edge<double>{ stop_id_[*iter3] * 2, stop_id_[*iter3] * 2 + 1, routing_settings_.bus_wait_time });
						edges_.push_back({ detail::EdgeDataType::Wait,*iter3 });
						double dist = 0.0;
						size_t depth = 0;
						if (iter3 + 1 != el1.second.stations.rend()) {
							auto last_iter = iter3;
							for (auto iter4 = iter3; iter4 != el1.second.stations.rend(); ++iter4) {
								auto& stop_data1 = catalogue.FindStopData(*last_iter);
								auto& stop_data2 = catalogue.FindStopData(*iter4);
								if (stop_data1.another_ways.count(*iter4)) {
									dist += stop_data1.another_ways[*iter4];
								}
								else if (stop_data2.another_ways.count(*last_iter)) {
									dist += stop_data2.another_ways[*last_iter];
								}
								else {
									dist += geo::ComputeDistance(stop_data1.coord, stop_data2.coord);
								}
								double copy = dist / routing_settings_.bus_velocity;
								graph_.AddEdge(graph::Edge<double>{ stop_id_[*iter3] * 2 + 1, stop_id_[*iter4] * 2, copy});
								edges_.push_back({ detail::EdgeDataType::Bus,std::nullopt,depth,el1.first,copy});
								last_iter = iter4;
							}
						}
					}
				}
			}
		}
		*/

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

			if (tr_ == NULL) {
				auto set = ProcessRoutingSettings();
				tr_ = new transport_router::TransportRouter(catalogue, set);
			}

			auto way = tr_->BuildRoute(stop_from, stop_to);

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

		SvgSetting TransportCatalogue::json_processing::JSONReader::ProcessRenderSettings()
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

		detail::RoutingSettingsStorage TransportCatalogue::json_processing::JSONReader::ProcessRoutingSettings()
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
			auto setting = ProcessRenderSettings();
			MapRenderer map_ren = MapRenderer(setting, catalogue, buf1);

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