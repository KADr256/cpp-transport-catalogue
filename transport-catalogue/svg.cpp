#include "svg.h"

namespace svg {

	using namespace std::literals;

	svg::Rgb::Rgb(uint16_t red, uint16_t green, uint16_t blue) :red(red), green(green), blue(blue) {
	}

	svg::Rgba::Rgba(uint16_t red, uint16_t green, uint16_t blue, double opacity) : red(red), green(green), blue(blue), opacity(opacity) {
	}

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}



	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\""sv;//???????????????"\" "
		RenderAttrs(out);
		out << "/>"sv;
	}
	//Polyline-----------------------
	Polyline& svg::Polyline::AddPoint(Point point) {
		polyline_storage_.push_back(point);
		return *this;
	}

	void svg::Polyline::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "  <polyline points=\""sv;
		out << std::setprecision(6);
		bool first = true;
		for (auto& point : this->polyline_storage_) {
			if (first) {
				out << point.x << ","sv << point.y;
				first = false;
			}
			else {
				out << " "sv << point.x << ","sv << point.y;
			}
		}
		out << "\"";
		RenderAttrs(out);
		out << "/>"sv;
	}
	//Text---------------------------
	// Задаёт координаты опорной точки (атрибуты x и y)
	Text& svg::Text::SetPosition(Point pos) {
		this->start_.x = pos.x;
		this->start_.y = pos.y;
		return *this;
	}

	// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
	Text& svg::Text::SetOffset(Point offset) {
		this->offset_.x = offset.x;
		this->offset_.y = offset.y;
		return *this;
	}

	// Задаёт размеры шрифта (атрибут font-size)
	Text& svg::Text::SetFontSize(uint32_t size) {
		this->font_size_ = size;
		return *this;
	}

	// Задаёт название шрифта (атрибут font-family)
	Text& svg::Text::SetFontFamily(std::string font_family) {
		this->font_family_ = font_family;
		return *this;
	}

	// Задаёт толщину шрифта (атрибут font-weight)
	Text& svg::Text::SetFontWeight(std::string font_weight) {
		this->font_weight_ = font_weight;
		return *this;
	}

	// Задаёт текстовое содержимое объекта (отображается внутри тега text)
	Text& svg::Text::SetData(std::string data) {
		this->text_ = data;
		return *this;
	}

	void svg::Text::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "  <text";
		RenderAttrs(out);
		out << " x=\"" << start_.x << "\" y=\"" << start_.y <<
			"\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y <<
			"\" font-size=\"" << font_size_ << "\"";
		if (!font_family_.empty()) {
			out << " font-family=\"" << font_family_ << "\"";
		}
		if (!font_weight_.empty()) {
			out << " font-weight=\"" << font_weight_ << "\"";
		}
		out << ">" << text_ << "</text>";
	}
	//Document------------------------

	void svg::Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects_.push_back(std::move(obj));
	}

	void svg::Document::Render(std::ostream& out) const {
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
		for (auto& obj : objects_) {
			obj.get()->Render(out);
		}
		out << "</svg>"sv << std::endl;
	}

}  // namespace svg