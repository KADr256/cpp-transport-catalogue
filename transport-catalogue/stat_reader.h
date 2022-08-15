#pragma once

#include <iostream>
#include <string>
#include <string_view>

#include "transport_catalogue.h"

namespace TrancportCatalogue {
	namespace output {

		void Read(Catalogue& catalogue, std::istream& in, std::ostream& out);

		void Bus(std::string& text, Catalogue& catalogue, std::ostream& out);

		void Stop(std::string& text, Catalogue& catalogue, std::ostream& out);
	}
}