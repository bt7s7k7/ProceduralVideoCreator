#include "pch.h"
#include "rendering.h"
#include "globals.h"
#include "constants.h"

std::vector<std::unique_ptr<RenderTask>>* tempTasks = nullptr;

struct Vector {
	coordinate x, y;

	Vector(coordinate x_, coordinate y_) : x(x_), y(y_) {};

	coordinate X() const {
		return x;
	}

	coordinate Y() {
		return y;
	}

	Vector Mul(coordinate value) const {
		return Vector(x * value, y * value);
	};

	Vector Add(const Vector& other)  const {
		return Vector(x + other.x, y + other.y);
	}

	std::string ToString()  const {
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
	}

	coordinate Size()  const {
		return std::hypot(x, y);
	}

	Vector Normalize() const {
		return Mul(1 / Size());
	}

	static Vector FromAngle(coordinate angle) {
		return Vector(std::sin(angle), std::cos(angle));
	}

	std::pair<int, int> ScaleAndFloor(double scale) const {
		return { static_cast<int>(std::floor(x * scale)), static_cast<int>(std::floor(y * scale)) };
	}

};

template <typename T>
class GenericTask : public RenderTask {
	T functor;

	// Inherited via RenderTask
	virtual const char* GetName() override {
		return "TestTask";
	}
	virtual void Render(SDL_Surface* surface, double scale) override {
		functor(surface, scale);
	}
public:
	GenericTask(T f) : functor(f) {};
};

template<typename T>
std::unique_ptr<GenericTask<T>> makeGenericTask(T func) {
	auto task = new GenericTask(func);
	return std::unique_ptr<GenericTask<T>>(task);
}

std::vector<std::unique_ptr<RenderTask>> updateLua(double time) {
	std::vector<std::unique_ptr<RenderTask>> ret;

	tempTasks = &ret;

	if (luaState["update"].isType<kaguya::LuaFunction>()) luaState["update"](time);

	tempTasks = nullptr;

	return std::move(ret);
}

std::tuple<int, int, int> limitColors(double dr, double dg, double db) {

	int r = static_cast<int>(std::floor(dr));
	int g = static_cast<int>(std::floor(dg));
	int b = static_cast<int>(std::floor(db));

	r = std::min(255, std::max(r, 0));
	g = std::min(255, std::max(g, 0));
	b = std::min(255, std::max(b, 0));

	return { r,g,b };
}

void setupLuaTasks(kaguya::State& state) {

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

	state["tasks"] = kaguya::NewTable();

	auto testVoid = [&state]() {
		if (tempTasks == nullptr) {
			state("error('Task can only be called from update', 2)");
			return true;
		} else {
			return false;
		}
	};

	state["tasks"]["test"].setFunction([&testVoid]() {
		if (testVoid()) return;
		tempTasks->push_back(makeGenericTask([](SDL_Surface* surface, double scale) {
			int w = surface->w, h = surface->h;
			for (int x = 0; x < w; x++) {
				for (int y = 0; y < h; y++) {
					*(Uint32*)((Uint8*)surface->pixels + y * surface->pitch + x * 4) = rendering::mapRGBA(surface->format, x % 255, 0, 0);////0xffffffff;
				}
			}
		}));
	});

	state["tasks"]["fill"].setFunction([&testVoid](double dr, double dg, double db) {
		if (testVoid()) return;
		auto [r, g, b] = limitColors(dr, dg, db);

		tempTasks->push_back(makeGenericTask([r = r, g = g, b = b](SDL_Surface* surface, double scale) {
			SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, r, g, b));
		}));
	});

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
				SDL_Rect
					top{ x,y,w,1 },
					left{ x,y,1,h },
					bottom{ x,y + h - 1, w, 1 },
					right{x + w - 1, y, 1, h};
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
}