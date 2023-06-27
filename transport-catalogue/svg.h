#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <iomanip>
#include <optional>
#include <variant>

namespace svg {

	inline std::monostate NoneColor;

	class Rgb {
	public:
		Rgb() = default;
		Rgb(uint16_t red, uint16_t green, uint16_t blue);
		uint16_t red = 0;
		uint16_t green = 0;
		uint16_t blue = 0;
	};

	class Rgba {
	public:
		Rgba() = default;
		Rgba(uint16_t red, uint16_t green, uint16_t blue, double opacity);
		uint16_t red = 0;
		uint16_t green = 0;
		uint16_t blue = 0;
		double opacity = 1.0;
	};

	using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

	struct ColorPrinter {
		std::ostream& out;
		void operator()(std::monostate) const {
			out << "none";
		}
		void operator()(std::string text) const {
			out << text;
		}
		void operator()(svg::Rgb rgb) const {
			out << "rgb(" << rgb.red << "," << rgb.green << "," << rgb.blue << ")";
		}
		void operator()(svg::Rgba rgba) const {
			out << "rgba(" << rgba.red << "," << rgba.green << "," << rgba.blue << "," << rgba.opacity << ")";
		}
	};

	inline std::ostream& operator<< (std::ostream& out, const Color color) {
		std::visit(ColorPrinter{ out }, color);
		return out;
	}

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};

	inline std::vector<std::string> line_cap_string = { "butt","round","square" };

	inline std::ostream& operator<< (std::ostream& out, const StrokeLineCap& line_cap) {
		out << line_cap_string[static_cast<int>(line_cap)];
		return out;
	}

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};

	inline std::vector<std::string> line_join_string = { "arcs","bevel","miter","miter-clip","round" };

	inline std::ostream& operator<< (std::ostream& out, const StrokeLineJoin& line_join) {
		out << line_join_string[static_cast<int>(line_join)];
		return out;
	}

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	class ObjectContainer {
	public:
		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

		template <typename Obj>
		void Add(Obj obj);
	};

	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color) {
			fill_ = color;
			return AsOwner();
		}

		Owner& SetStrokeColor(Color color) {
			stroke_color_ = color;
			return AsOwner();
		}

		Owner& SetStrokeWidth(double width) {
			stroke_width_ = width;
			return AsOwner();
		}

		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			line_cap_ = line_cap;
			return AsOwner();
		}

		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			line_join_ = line_join;
			return AsOwner();
		}

		void RenderAttrs(std::ostream& out) const {
			if (fill_.has_value()) {
				out << " fill=\"" << fill_.value() << "\"";
			}
			if (stroke_color_.has_value()) {
				out << " stroke=\"" << stroke_color_.value() << "\"";
			}
			if (stroke_width_.has_value()) {
				out << " stroke-width=\"" << stroke_width_.value() << "\"";
			}
			if (line_cap_.has_value()) {
				out << " stroke-linecap=\"" << line_cap_.value() << "\"";
			}
			if (line_join_.has_value()) {
				out << " stroke-linejoin=\"" << line_join_.value() << "\"";
			}
		};
	private:
		Owner& AsOwner() {
			return static_cast<Owner&>(*this);
		}

		std::optional<Color> fill_;
		std::optional<Color> stroke_color_;
		std::optional<double> stroke_width_;
		std::optional<StrokeLineCap> line_cap_;
		std::optional<StrokeLineJoin> line_join_;
	};

	class Drawable {
	public:
		virtual void Draw(ObjectContainer& obj_cont) const = 0;
		virtual ~Drawable() = default;
	};

	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_ = { 0.0,0.0 };
		double radius_ = 1.0;
	};

	class Polyline : public Object, public PathProps<Polyline> {
	public:
		Polyline& AddPoint(Point point);

	private:
		void RenderObject(const RenderContext& context) const override;

		std::vector<Point> polyline_storage_;
	};

	class Text : public Object, public PathProps<Text> {
	public:
		Text& SetPosition(Point pos);

		Text& SetOffset(Point offset);

		Text& SetFontSize(uint32_t size);

		Text& SetFontFamily(std::string font_family);

		Text& SetFontWeight(std::string font_weight);

		Text& SetData(std::string data);
	private:
		void RenderObject(const RenderContext& context) const override;

		Point start_ = { 0.0,0.0 };
		Point offset_ = { 0.0,0.0 };
		uint32_t font_size_ = 1;
		std::string font_family_;
		std::string font_weight_;
		std::string text_;
	};

	class Document : public ObjectContainer {
	public:
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		void Render(std::ostream& out) const;
	private:
		std::vector<std::unique_ptr<Object>> objects_;
	};

}  // namespace svg

template <typename Obj>
void svg::ObjectContainer::Add(Obj obj) {
	AddPtr(std::make_unique<Obj>(std::move(obj)));
}