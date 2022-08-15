#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "transport_catalogue.h"

namespace TrancportCatalogue {
	namespace input {
		namespace line {
			void Buses(Catalogue& cataloge, std::unordered_set<std::string>& bus_wait);

			void Stop(Catalogue& cataloge, std::string& text);
		}

		void Read(Catalogue& cataloge, std::istream& in);
	}
}