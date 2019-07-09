#include "pch.h"
#include "rendering.h"
#include "globals.h"
#include "constants.h"

std::vector<std::unique_ptr<RenderTask>>* _tempTasks = nullptr;

struct Vector {
	coordinate x, y;

	Vector(coordinate x_, coordinate y_) : x(x_), y(y_) {};

	coordinate X() {
		return x;
	}

	coordinate Y() {
		return y;
	}

	Vector Mul(coordinate value) {
		return Vector(x * value, y * value);
	};

	Vector Add(const Vector& other) {
		return Vector(x + other.x, y + other.y);
	}

	std::string ToString() {
		return std::string("[") + std::to_string(x) + ", " + std::to_string(y) + "]";
	}

	coordinate Size() {
		return std::hypot(x, y);
	}

	Vector Normalize() {
		return Mul(1 / Size());
	}

	static Vector FromAngle(coordinate angle) {
		return Vector(std::sin(angle), std::cos(angle));
	}

	std::pair<int, int> ScaleAndFloor(double scale) {
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

	_tempTasks = &ret;

	if (luaState["update"].isType<kaguya::LuaFunction>()) luaState["update"](time);

	_tempTasks = nullptr;

	return std::move(ret);
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
		if (_tempTasks == nullptr) {
			state("error('Task can only be called from update', 2)");
			return true;
		} else {
			return false;
		}
	};

	state["tasks"]["test"].setFunction([&testVoid]() {
		if (testVoid()) return;
		_tempTasks->push_back(makeGenericTask([](SDL_Surface* surface, double scale) {
			int w = surface->w, h = surface->h;
			for (int x = 0; x < w; x++) {
				for (int y = 0; y < h; y++) {
					*(Uint32*)((Uint8*)surface->pixels + y * surface->pitch + x * 4) = rendering::mapRGBA(surface->format, x % 255, 0, 0);////0xffffffff;
				}
			}
		}));
	});
}