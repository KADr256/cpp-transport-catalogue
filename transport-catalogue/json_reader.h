#pragma once
#include <sstream>
#include <filesystem>
#include <fstream>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

namespace TransportCatalogue {
	namespace json_processing {
		svg::Color ColorUndefine(const json::Node& node);

		json::Document Read(std::istream& in);

		class JSONReaderMB {
		public:
			JSONReaderMB(std::istream& in): document_(Read(in)) {
				ProcessBaseRequests();
				rss=ProcessRoutingSettings();
				svg_set=ProcessRenderSettings();
				catalogue.CalculateAll();
				tr_ = new transport_router::TransportRouter(catalogue, rss);
				if (document_.GetRoot().AsMap().count("serialization_settings")) {
					auto ofs_file = std::ofstream(document_.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString(), std::ios::binary);
					Serialization::SaveAll(ofs_file, catalogue, svg_set,*tr_,rss);
				}
			}

			~JSONReaderMB() {
				if (tr_ != NULL) {
					delete tr_;
				}
			}

			Catalogue catalogue;
			SvgSetting svg_set;
			detail::RoutingSettingsStorage rss;
			json::Document document_;
			transport_router::TransportRouter* tr_ = NULL;
		private:
			void AddJSONStop(const json::Map& json_stop);

			void AddJSONBus(const json::Map& json_bus);

			void ProcessBaseRequests();

			detail::RoutingSettingsStorage ProcessRoutingSettings();

			SvgSetting ProcessRenderSettings();

			//std::ostream& ProcessPath();
			//json::Document document_;
		};

		class JSONReader {
		public:
			JSONReader(std::istream& in, std::ostream& out) :out_(out),document_(Read(in)) {
				if (document_.GetRoot().AsMap().count("serialization_settings")) {
					auto ifs_file = std::ifstream(document_.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString(), std::ios::binary);
					Serialization::LoadAll(ifs_file, catalogue, svg_set,tr_);
				}
				ProcessStatRequests();
			}

			Catalogue catalogue;
			SvgSetting svg_set;
			detail::RoutingSettingsStorage rss;
		private:
			json::NodeData AnswerJSONStop(const json::Map& map);

			json::NodeData AnswerJSONBus(const json::Map& map);

			json::NodeData AnswerJSONRoute(const json::Map& map);

			json::NodeData AnswerJSONMap(const json::Map& map);

			void ProcessStatRequests();

			//std::istream& ProcessPath();

			std::ostream& out_;
			std::optional<transport_router::TransportRouter> tr_;
			json::Document document_;
		};
	}
}