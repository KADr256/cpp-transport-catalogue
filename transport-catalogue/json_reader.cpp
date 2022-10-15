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

		void TransportCatalogue::json_processing::JSONReader::AnswerJSONStop(const json::Map& json_request) {
			std::string stop = json_request.at("name").AsString();
			if (!catalogue.KnownStop(stop)) {
				out_ << "\"request_id\": " << json_request.at("id").AsInt() << "," << "\"error_message\": \"not found\"";
				return;
			}
			auto& buses = catalogue.FindStopData(stop).buses;
			if (buses.size() == 0) {
				out_ << "\"buses\": [], \"request_id\": " << json_request.at("id").AsInt();
			}
			else {
				bool first = true;
				out_.precision(6);
				out_ << "\"buses\": [";
				for (auto& part : buses) {
					if (first) {
						first = false;
					}
					else {
						out_ << ",";
					}
					out_ << "\"" << part << "\"";
				}
				out_ << "], \"request_id\": " << json_request.at("id").AsInt();
			}
		}

		void TransportCatalogue::json_processing::JSONReader::AnswerJSONBus(const json::Map& json_request)
		{
			auto data = catalogue.ProcessBusData(json_request.at("name").AsString());
			if (data.lenght == -1) {
				out_ << "\"request_id\": " << json_request.at("id").AsInt() << "," << "\"error_message\": \"not found\"";
			}
			else {
				out_.precision(6);
				out_ << "\"curvature\": " << data.curvatur << ","
					<< "\"request_id\" : " << json_request.at("id").AsInt() << ","
					<< "\"route_length\" : " << data.lenght << ","
					<< "\"stop_count\" : " << data.stops_count << ","
					<< "\"unique_stop_count\" : " << data.unique_stops_count;
			}
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

		void TransportCatalogue::json_processing::JSONReader::AnswerJSONMap(const json::Map& map)
		{
			std::stringstream buf1;
			auto setting = ProcessRenderSettings();
			out_ << "\"map\": ";
			MapRenderer map_ren=MapRenderer (setting, catalogue, buf1);

			auto buf2 = json::Document(buf1.str());

			json::Print(buf2, out_);

			out_ << ", \"request_id\": " << map.at("id").AsInt();
		}

		void TransportCatalogue::json_processing::JSONReader::ProcessStatRequests() { //Абсолютно не вижу вариантов перенести определение запросов в request_handler, без превращения всего проекта в паутину зависимостей или без обработки в json_reader 
			bool first = true;
			if (document_.GetRoot().AsMap().count("stat_requests")) {
				auto iter = document_.GetRoot().AsMap().find("stat_requests");
				if ((*iter).second.AsArray().size() != 0) {
					out_ << "[";
					for (auto& part : (*iter).second.AsArray()) {
						if (first) {
							first = false;
						}
						else {
							out_ << ",";
						}
						out_ << "{";
						if (part.AsMap().at("type").AsString() == "Bus") {
							AnswerJSONBus(part.AsMap());
						}
						else if (part.AsMap().at("type").AsString() == "Stop") {
							AnswerJSONStop(part.AsMap());
						}
						else if (part.AsMap().at("type").AsString() == "Map") {
							AnswerJSONMap(part.AsMap());
						}
						else {
							out_ << "\"request_id\": " << part.AsMap().at("id").AsInt() << "," << "\"error_message\": \"not found\"";
						}
						out_ << "}";
					}
					out_ << "]";
				}
			}
		}
	}
}