#pragma once

#include <iostream>
#include <string>
#include <string_view>

#include "transport_catalogue.h"

namespace TrancportCatalogue {
	namespace output {

		void Read(Catalogue& catalogue);

		void Bus(std::string& text, Catalogue& catalogue);

		void Stop(std::string& text, Catalogue& catalogue);
	}
}