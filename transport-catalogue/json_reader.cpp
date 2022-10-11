#include "json_reader.h"

namespace TrancportCatalogue {
	namespace json_processing {
		namespace input {
			void ProcessBus(Catalogue& catalog, const json::Map& map) {
				std::string bus_name = map.at("name").AsString();
				detail::RouteType type;
				std::deque<std::string> bus_way;

				if (map.at("is_roundtrip").AsBool()) {
					type = detail::RouteType::oneway;
				}
				else {
					type = detail::RouteType::twoway;
				}

				for (auto& stop : map.at("stops").AsArray()) {
					bus_way.push_back(stop.AsString());
				}
				catalog.AddBus(bus_name, bus_way, type);
			}

			void ProcessStop(Catalogue& catalog, const json::Map& map) {
				std::string stop_name = map.at("name").AsString();
				geo::Coordinates coord = { map.at("latitude").AsDouble(),map.at("longitude").AsDouble() };

				catalog.AddStop(stop_name, coord);

				if (map.count("road_distances")) {
					for (auto& part : map.at("road_distances").AsMap()) {
						catalog.AddWay(stop_name, part.first, part.second.AsInt());
					};
				}
			}

			SvgSetting MapSettings(const json::Document& document) {
				SvgSetting setting;
				if (document.GetRoot().AsMap().count("render_settings")) {
					auto iter = document.GetRoot().AsMap().find("render_settings");
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
					setting.underlayer_color = TrancportCatalogue::json_processing::ColorUndefine(map.at("underlayer_color"));
					setting.underlayer_width = map.at("underlayer_width").AsDouble();
					{
						auto& color_palette = map.at("color_palette").AsArray();
						for (auto& part : color_palette) {
							setting.color_palette.push_back(TrancportCatalogue::json_processing::ColorUndefine(part));
						}
					}
				}
				return setting;
			}

			void Process(Catalogue& catalog, const json::Document& document) {
				{
					if (document.GetRoot().AsMap().count("base_requests")) {
						auto iter = document.GetRoot().AsMap().find("base_requests");
						if ((*iter).second.AsArray().size() != 0) {
							for (auto& part : (*iter).second.AsArray()) {
								if (part.AsMap().at("type").AsString() == "Stop") {
									input::ProcessStop(catalog, part.AsMap());
								}
							}
							for (auto& part : (*iter).second.AsArray()) {
								if (part.AsMap().at("type").AsString() == "Bus") {
									input::ProcessBus(catalog, part.AsMap());
								}
							}

						}
					}
				}
			}
		}

		namespace output {
			void ProcessBus(Catalogue& catalog, const json::Map& map, std::ostream& out) {
				auto data = catalog.ProcessBusData(map.at("name").AsString());
				if (data.lenght == -1) {
					out << "\"request_id\": " << map.at("id").AsInt() << "," << "\"error_message\": \"not found\"";
				}
				else {
					out.precision(6);
					out << "\"curvature\": " << data.curvatur << ","
						<< "\"request_id\" : " << map.at("id").AsInt() << ","
						<< "\"route_length\" : " << data.lenght << ","
						<< "\"stop_count\" : " << data.stops_count << ","
						<< "\"unique_stop_count\" : " << data.unique_stops_count;
				}
			}

			void ProcessStop(Catalogue& catalog, const json::Map& map, std::ostream& out) {
				std::string stop = map.at("name").AsString();
				if (!catalog.KnownStop(stop)) {
					out << "\"request_id\": " << map.at("id").AsInt() << "," << "\"error_message\": \"not found\"";
					return;
				}
				auto& buses = catalog.FindStopData(stop).buses;
				if (buses.size() == 0) {
					out << "\"buses\": [], \"request_id\": " << map.at("id").AsInt();
				}
				else {
					bool first = true;
					out.precision(6);
					out << "\"buses\": [";
					for (auto& part : buses) {
						if (first) {
							first = false;
						}
						else {
							out << ",";
						}
						out << "\"" << part << "\"";
					}
					out << "], \"request_id\": " << map.at("id").AsInt();
				}
			}

			void ProcessMap(Catalogue& catalog, const json::Map& map, const json::Document& document, std::ostream& out) {
				std::stringstream buf1;
				auto setting = input::MapSettings(document);
				auto data = TrancportCatalogue::map::DataForMap(catalog);
				out << "\"map\": ";
				MapOut(setting, data, buf1);

				auto buf2 = json::Document(buf1.str());

				json::Print(buf2, out);

				out << ", \"request_id\": " << map.at("id").AsInt();
			}

			void Process(Catalogue& catalog, const json::Document& document, std::ostream& out) {
				bool first = true;
				if (document.GetRoot().AsMap().count("stat_requests")) {
					auto iter = document.GetRoot().AsMap().find("stat_requests");
					if ((*iter).second.AsArray().size() != 0) {
						out << "[";
						for (auto& part : (*iter).second.AsArray()) {
							if (first) {
								first = false;
							}
							else {
								out << ",";
							}
							out << "{";
							if (part.AsMap().at("type").AsString() == "Bus") {
								output::ProcessBus(catalog, part.AsMap(), out);
							}
							else if (part.AsMap().at("type").AsString() == "Stop") {
								output::ProcessStop(catalog, part.AsMap(), out);
							}
							else if (part.AsMap().at("type").AsString() == "Map") {
								output::ProcessMap(catalog, part.AsMap(), document, out);
							}
							else {
								out << "\"request_id\": " << part.AsMap().at("id").AsInt() << "," << "\"error_message\": \"not found\"";
							}
							out << "}";
							if (first) {
								first = false;
								continue;
							}
						}
						out << "]";
					}
				}
			}
		}

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
	}
}