#pragma once

#include "svg.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace TransportCatalogue {
	namespace Serialization {
		void TransferColorIn(const SaveLoad::Color& color_in, svg::Color& color);

		void TransferColorOut(const svg::Color& color_out, SaveLoad::Color& color);

		void SaveSvgSettings(const SvgSetting& svg_set,SaveLoad::SvgSetting& svg_set_out);

		void LoadSvgSettings(SvgSetting& svg_set,const SaveLoad::SvgSetting& svg_set_in);

		void SaveTransportCatalogue(const Catalogue& catalogue,SaveLoad::Catalogue& catalogue_out);

		void LoadTransportCatalogue(Catalogue& catalogue,const SaveLoad::Catalogue& catalogue_in,std::vector<std::string_view>& bus_table, std::vector<std::string_view>& stop_table);

		void SaveTransportRouter(const Catalogue& catalogue,const transport_router::TransportRouter& tr_router,SaveLoad::TransportRouter& tr_router_out,const detail::RoutingSettingsStorage rss);

		void LoadTransportRouter(Catalogue& catalogue, std::optional<transport_router::TransportRouter>& tr_router,const SaveLoad::TransportRouter& tr_router_in,std::vector<std::string_view>& bus_table, std::vector<std::string_view>& stop_table);
		//
		void SaveAll(std::ostream& out,const TransportCatalogue::Catalogue& catalogue,const SvgSetting& svg_set, const transport_router::TransportRouter& tr_router, const detail::RoutingSettingsStorage rss);
		//
		void LoadAll(std::istream& in, TransportCatalogue::Catalogue& catalogue, SvgSetting& svg_set, std::optional<transport_router::TransportRouter>& tr_router);
	}
}