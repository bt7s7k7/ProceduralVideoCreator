#include "pch.h"
#include "rendering.h"
#include "globals.h"
#include "constants.h"
#include "font.h"

std::vector<std::unique_ptr<RenderTask>>* tempTasks = nullptr;

/*
	Structure to store 2D coordinates,
	also accesible from Lua but all
	functions are camel case
*/
struct Vector {
	coordinate_t x, y;

	Vector(coordinate_t x_, coordinate_t y_) : x(x_), y(y_) {};

	coordinate_t X() const {
		return x;
	}

	coordinate_t Y() {
		return y;
	}

	Vector Mul(coordinate_t value) const {
		return Vector(x * value, y * value);
	};

	Vector Add(const Vector& other)  const {
		return Vector(x + other.x, y + other.y);
	}

	std::string ToString()  const {
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
	}

	coordinate_t Size()  const {
		return std::hypot(x, y);
	}

	Vector Normalize() const {
		return Mul(1 / Size());
	}

	static Vector FromAngle(coordinate_t angle) {
		return Vector(std::sin(angle), std::cos(angle));
	}

	std::pair<int, int> ScaleAndFloor(double scale) const {
		return { static_cast<int>(std::floor(x * scale)), static_cast<int>(std::floor(y * scale)) };
	}

};

/*
	This struct will execute the functor provided
	during costruction on Render
*/
template <typename T>
class GenericTask : public RenderTask {
	T functor;

	// Inherited via RenderTask
	virtual const char* GetName() override {
		return "TestTask";
	}
	virtual void Render(SDL_Surface* surface, double scale) override {
		// Calling provided functor
		functor(surface, scale);
	}
public:
	GenericTask(T f) : functor(f) {};
};

/*
	This function creates a GenericTask using the provided
	functor and puts it inside a unique_ptr automaticaly.
*/
template<typename T>
std::unique_ptr<GenericTask<T>> makeGenericTask(T func) {
	auto task = new GenericTask(func);
	return std::unique_ptr<GenericTask<T>>(task);
}

std::vector<std::unique_ptr<RenderTask>> updateLua(double time) {
	std::vector<std::unique_ptr<RenderTask>> ret;

	tempTasks = &ret;
#pragma warning( push )
#pragma warning( disable : 26444 )
	if (luaState["update"].isType<kaguya::LuaFunction>()) luaState["update"](time);
#pragma warning( pop ) 

	tempTasks = nullptr;

	return std::move(ret);
}

/*
	This functions multiplies all color values by 255, floors them and limits
	them 0...255 inclusive then returns [r, g, b]. Use capture:
	auto [r, g, b] = limitColors(dr, dg, db);
*/
std::tuple<Uint8, Uint8, Uint8> limitColors(double dr, double dg, double db) {

	dr = std::min(1.0, std::max(dr, 0.0));
	dg = std::min(1.0, std::max(dg, 0.0));
	db = std::min(1.0, std::max(db, 0.0));

	Uint8 r = static_cast<Uint8>(std::floor(dr * 255));
	Uint8 g = static_cast<Uint8>(std::floor(dg * 255));
	Uint8 b = static_cast<Uint8>(std::floor(db * 255));


	return { r,g,b };
}

void limit10(double& value) {
	value = std::min(1.0, std::max(value, 0.0));
}

void setupLuaTasks(kaguya::State& state) {

	// Creating bindings for Vector class inside Lua
	state["Vector"].setClass(kaguya::UserdataMetatable<Vector>()
		.setConstructors<Vector(double, double)>()
		.addFunction("x", &Vector::X)
		.addFunction("y", &Vector::Y)
		.addFunction("mul", &Vector::Mul)
		.addFunction("add", &Vector::Add)
		.addFunction("toString", &Vector::ToString)
		.addFunction("normalize", &Vector::Normalize)
		.addFunction("size", &Vector::Size)
		.addStaticFunction("fromAngle", &Vector::FromAngle)
	);

	state["Vector"]["zero"] = Vector(0, 0);
	state["Vector"]["one"] = Vector(1, 1);
	state["Vector"]["center"] = Vector(0.5, 0.5);
	state["Vector"]["up"] = Vector(0, -1);
	state["Vector"]["down"] = Vector(0, 1);
	state["Vector"]["left"] = Vector(-1, 0);
	state["Vector"]["right"] = Vector(1, 0);
	state["Vector"]["none"] = Vector(-1, -1);

	state["tasks"] = kaguya::NewTable(); // Creating global table for all tasks

	/*
		Tests if tempTasks is void so you can push your task. Use:
		if (testVoid()) return;
	*/
	auto testVoid = [&state]() {
		if (tempTasks == nullptr) {
			state("error('Task can only be called from update', 2)");
			return true;
		} else {
			return false;
		}
	};

	state["tasks"]["test"].setFunction([&testVoid]() { // Testing function, will probably be removed 
		if (testVoid()) return;
		tempTasks->push_back(makeGenericTask([](SDL_Surface* surface, double scale) {
			int w = surface->w, h = surface->h;
			for (int x = 0; x < w; x++) {
				for (int y = 0; y < h; y++) {
					*(Uint32*)((Uint8*)surface->pixels + y * surface->pitch + x * 4) = rendering::mapRGBA(surface->format, x % 255, 0, 0);
				}
			}
		}));
	});

	// tasks.fill
	state["tasks"]["fill"].setFunction([&testVoid](double dr, double dg, double db) {
		if (testVoid()) return;
		auto [r, g, b] = limitColors(dr, dg, db);

		tempTasks->push_back(makeGenericTask([r = r, g = g, b = b](SDL_Surface* surface, double scale) {
			SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, r, g, b));
		}));
	});

	// tasks.rect
	state["tasks"]["rect"].setFunction([&testVoid](Vector pos, Vector size, bool center, bool fill, double dr, double dg, double db) {
		if (testVoid()) return;
		auto [r, g, b] = limitColors(dr, dg, db);

		if (center) {
			pos = pos.Add(size.Mul(-0.5));
		}

		tempTasks->push_back(makeGenericTask([r = r, g = g, b = b, pos, size, fill](SDL_Surface* surface, double scale) {
			auto [x, y] = pos.ScaleAndFloor(scale);
			auto [w, h] = size.ScaleAndFloor(scale);
			if (!fill) {
				// If we want not filled, just draw the edge lines
				SDL_Rect
					top{ x,y,w,1 },
					left{ x,y,1,h },
					bottom{ x,y + h - 1, w, 1 },
					right{ x + w - 1, y, 1, h };
				auto color = SDL_MapRGB(surface->format, r, g, b);
				SDL_FillRect(surface, &top, color);
				SDL_FillRect(surface, &left, color);
				SDL_FillRect(surface, &bottom, color);
				SDL_FillRect(surface, &right, color);
			} else {
				SDL_Rect rect{ x,y,w,h };
				SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, r, g, b));
			}
		}));

	});

	// tasks.circle
	state["tasks"]["circle"].setFunction([&testVoid](Vector pos, coordinate_t radius, bool fill, double dr, double dg, double db) {
		if (testVoid()) return;
		auto [r, g, b] = limitColors(dr, dg, db);

		if (radius < 1) return;

		tempTasks->push_back(makeGenericTask([rr = r, g = g, b = b, pos, r_ = radius, fill](SDL_Surface* surface, double scale) {
			int r = static_cast<int>(std::floor(r_ * scale));
			auto color = SDL_MapRGB(surface->format, rr, g, b);

			auto draw = [&]() {
				auto [cx, cy] = pos.ScaleAndFloor(scale);
				int r2 = r * r;
				int x = 0, x2 = 0, dx2 = 1;
				int y = r, y2 = y * y, dy2 = 2 * y - 1;
				int sum = r2;

				// Allocate space for rects
				std::vector<SDL_Rect> rects{};
				int count;

				if (!fill) {
					rects.resize(8);
					count = 8;
				} else {
					rects.resize(4);
					count = 4;
				}

				auto rectsPtr = rects.data();

				while (x <= y) {
					if (!fill) {
						// Render a point for each octet
						rectsPtr[0] = { cx + x, cy + y, 1, 1 };
						rectsPtr[1] = { cx + x, cy - y, 1, 1 };
						rectsPtr[2] = { cx - x, cy + y, 1, 1 };
						rectsPtr[3] = { cx - x, cy - y, 1, 1 };
						rectsPtr[4] = { cx + y, cy + x, 1, 1 };
						rectsPtr[5] = { cx + y, cy - x, 1, 1 };
						rectsPtr[6] = { cx - y, cy + x, 1, 1 };
						rectsPtr[7] = { cx - y, cy - x, 1, 1 };
					} else {
						// Draw lines for a full circle
						rectsPtr[0] = { cx - x, cy - y, cx + x - (cx - x), 1 };
						rectsPtr[1] = { cx - y, cy + x, cx + y - (cx - y), 1 };
						rectsPtr[2] = { cx - y, cy - x, cx + y - (cx - y), 1 };
						rectsPtr[3] = { cx - x, cy + y, cx + x - (cx - x), 1 };
					}
					// Draw points / lines
					SDL_FillRects(surface, rectsPtr, count, color);

					sum -= dx2;
					x2 += dx2;
					x++;
					dx2 += 2;
					if (sum <= y2) {
						y--; y2 -= dy2; dy2 -= 2;
					}
				}
			};
			draw();
		}));
	});

	//tasks.line
	state["tasks"]["line"].setFunction([&testVoid](Vector pos1, Vector pos2, double dr, double dg, double db) {
		if (testVoid()) return;
		auto [r, g, b] = limitColors(dr, dg, db);

		tempTasks->push_back(makeGenericTask([r = r, g = g, b = b, pos1, pos2](SDL_Surface* surface, double scale) {
			auto [x1, y1] = pos1.ScaleAndFloor(scale);
			auto [x2, y2] = pos2.ScaleAndFloor(scale);

			// Make sure the coordinates change the right direction
			bool flipH = false, flipV = false;
			if (x1 > x2) {
				std::swap(x1, x2);
				flipH = true;
			}
			if (y1 > y2) {
				std::swap(y1, y2);
				flipV = true;
			}

			/* If the main direction is vertical */
			bool lerpVertical = (x2 - x1) > (y2 - y1);
			// If we should flip coordinates
			/* Flip other */
			bool flipOther = false;
			/* Flip main direction */
			bool flip = false;
			// Testing for conditions
			if (lerpVertical && flipH && !flipV) flipOther = true;
			if (!lerpVertical && flipH && !flipV) flip = true;
			if (lerpVertical && !flipH && flipV) flipOther = true;
			if (!lerpVertical && !flipH && flipV) flip = true;
			// Getting the color
			auto color = SDL_MapRGB(surface->format, r, g, b);
			/* Size of the line in the main direction */
			int dist = lerpVertical ? y2 - y1 : x2 - x1;
			/* Size of the line in the other direction */
			int otherDist = !lerpVertical ? y2 - y1 : x2 - x1;

			for (int i = 0; i < dist; i++) {
				int start, end;
				if (flipOther) { // Should we flip the other direction
					start = static_cast<int>((1 - ((double)i + 1) / dist) * otherDist);
					end = static_cast<int>((1 - (i + 0) / (double)dist) * otherDist);
				} else {
					start = static_cast<int>(((i + 0) / (double)dist) * otherDist);
					end = static_cast<int>((((double)i + 1) / (double)dist) * otherDist);
				}

				int ai;
				if (flip) ai = dist - i; // Should we flip the main direction
				else ai = i;

				// Rendering line segments
				SDL_Rect rect;
				if (lerpVertical) rect = { start + x1, ai + y1, end - start, 1 };
				else rect = { ai + x1, start + y1, 1, end - start };
				SDL_FillRect(surface, &rect, color);
			}
		}));

	});


	state["tasks"]["text"].setFunction([&testVoid](Vector pos, coordinate_t height, Vector aling, std::string text, int maxWidth, double dr, double dg, double db) {
		if (testVoid()) return;
		auto [r, g, b] = limitColors(dr, dg, db);

		if (text.length() == 0 || height < 1) return;

		tempTasks->push_back(makeGenericTask([r = r, g = g, b = b, pos, h_ = height, aling, text, mW_ = maxWidth](SDL_Surface* surface, double scale) {
			auto [x, y] = pos.ScaleAndFloor(scale);

			int height = static_cast<int>(std::floor(h_ * scale));
			int maxWidth = static_cast<int>(std::floor(mW_ * scale));

			auto font = getOrLoadFont(height);
			unique_surface_ptr rendered;
			if (maxWidth > 0) {
				rendered.reset(handleSDLError(TTF_RenderText_Blended_Wrapped(font, text.data(), SDL_Color{ r, g, b, 255 }, maxWidth)));
			} else {
				rendered.reset(handleSDLError(TTF_RenderText_Blended(font, text.data(), SDL_Color{ r, g, b, 255 })));
			}

			int oX = static_cast<int>(std::floor(rendered->w * aling.x));
			int oY = static_cast<int>(std::floor(rendered->h * aling.y));

			SDL_Rect rect{ x - oX, y - oY, rendered->w, rendered->h };
			SDL_BlitSurface(rendered.get(), nullptr, surface, &rect);

		}));
	});


	state["tasks"]["image"].setFunction([&testVoid](Vector pos, Vector size, Vector fill, bool center, LuaSurfacePtr image) {
		if (testVoid()) return;
		if (size.x < 1 || size.y < 1 || fill.x <= 0 || fill.y <= 0) return;

		if (!image.surface) throw std::runtime_error("image is empty /* Do you have enough arguments? (5 expected) */");

		limit10(fill.x);
		limit10(fill.y);

		Vector oSize = size;
		oSize.x *= fill.x;
		oSize.y *= fill.y;

		SDL_Point iSize = { image.surface->get()->w, image.surface->get()->h };
		SDL_Point iSizeFull = iSize;
		iSize.x = static_cast<int>(std::floor(fill.x * iSize.x));
		iSize.y = static_cast<int>(std::floor(fill.y * iSize.y));

		tempTasks->push_back(makeGenericTask([pos, oSize, iSize, image = image.surface, center, iSizeFull](SDL_Surface* surface, double scale) {
			auto [x, y] = pos.ScaleAndFloor(scale);
			auto [iw, ih] = iSize;
			auto [ow, oh] = oSize.ScaleAndFloor(scale);

			SDL_Rect srcrect;
			SDL_Rect dstrect;

			if (center) {
				srcrect = { iSizeFull.x / 2 - iw / 2, iSizeFull.y / 2 - ih / 2, iw, ih };
				dstrect = { x - ow / 2, y - oh / 2, ow, oh };
			} else {
				srcrect = { 0,0,iw,ih };
				dstrect = { x,y,ow,oh };
			}

			SDL_BlitScaled(image->get(), &srcrect, surface, &dstrect);

		}));
	});
}