#pragma once
#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace TransportCatalogue {
	namespace json_processing {
		svg::Color ColorUndefine(const json::Node& node);

		json::Document Read(std::istream& in);

		class JSONReader {
		public:
			JSONReader(Catalogue& catalogue, std::istream& in, std::ostream& out) :catalogue(catalogue), out_(out), document_(Read(in)) {
				ProcessBaseRequests();
				ProcessStatRequests();
			}

			Catalogue& catalogue;
		private:
			void AddJSONStop(const json::Map& json_stop);

			void AddJSONBus(const json::Map& json_bus);

			void ProcessBaseRequests();

			void AnswerJSONStop(const json::Map& map);

			void AnswerJSONBus(const json::Map& map);

			SvgSetting ProcessRenderSettings();

			void AnswerJSONMap(const json::Map& map);

			void ProcessStatRequests();

			std::ostream& out_;
			json::Document document_;
		};
	}
}