#pragma once
#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace TrancportCatalogue {
	namespace json_processing {
		namespace input {
			void ProcessBus(Catalogue& catalog, const json::Map& map);

			void ProcessStop(Catalogue& catalog, const json::Map& map);

			SvgSetting MapSettings(const json::Document& document);

			void Process(Catalogue& catalog, const json::Document& document);
		}
		namespace output {
			void ProcessBus(Catalogue& catalog, const json::Map& map, std::ostream& out);

			void ProcessStop(Catalogue& catalog, const json::Map& map, std::ostream& out);

			void ProcessMap(Catalogue& catalog, const json::Map& map, const json::Document& document, std::ostream& out);

			void Process(Catalogue& catalog, const json::Document& document, std::ostream& out);
		}

		svg::Color ColorUndefine(const json::Node& node);

		json::Document Read(std::istream& in);
	}
}