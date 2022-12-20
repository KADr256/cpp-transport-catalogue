#pragma once
#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

namespace TransportCatalogue {
	namespace json_processing {
		svg::Color ColorUndefine(const json::Node& node);

		json::Document Read(std::istream& in);

		class JSONReader {
		public:
			JSONReader(Catalogue& catalogue, std::istream& in, std::ostream& out) :catalogue(catalogue), out_(out), document_(Read(in)) {
				ProcessBaseRequests();
				ProcessRoutingSettings();
				ProcessStatRequests();
			}

			~JSONReader() {
				if (tr_ != NULL) {
					delete tr_;
				}
			}

			Catalogue& catalogue;
		private:
			void AddJSONStop(const json::Map& json_stop);

			void AddJSONBus(const json::Map& json_bus);

			void ProcessBaseRequests();

			json::NodeData AnswerJSONStop(const json::Map& map);

			json::NodeData AnswerJSONBus(const json::Map& map);

			detail::RoutingSettingsStorage ProcessRoutingSettings();

			json::NodeData AnswerJSONRoute(const json::Map& map);

			SvgSetting ProcessRenderSettings();

			json::NodeData AnswerJSONMap(const json::Map& map);

			void ProcessStatRequests();

			std::ostream& out_;
			transport_router::TransportRouter* tr_ = NULL;
			/*
			detail::RoutingSettingsStorage routing_settings_;
			graph::DirectedWeightedGraph<double> graph_;
			std::map<std::string_view,size_t> stop_id_;
			std::vector<detail::EdgeData> edges_;
			*/
			json::Document document_;
		};
	}
}