#include "pch.h"
#include "rendering.h"
#include "globals.h"

std::vector<std::unique_ptr<RenderTask>>* _tempTasks = nullptr;


class TestTask : public RenderTask {
	// Inherited via RenderTask
	virtual const char* GetName() override {
		return "TestTask";
	}
	virtual void Render(SDL_Surface* surface, double scale) override {
		int w = surface->w, h = surface->h;
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				*(Uint32*)((Uint8*)surface->pixels + y * surface->pitch + x * 4) = rendering::mapRGBA(surface->format, x % 255, 0, 0);////0xffffffff;
			}
		}
	}
};

std::vector<std::unique_ptr<RenderTask>> updateLua(double time) {
	std::vector<std::unique_ptr<RenderTask>> ret;

	_tempTasks = &ret;

	if (luaState["update"].isType<kaguya::LuaFunction>()) luaState["update"](time);
	
	_tempTasks = nullptr;

	return std::move(ret);
}

void setupLuaTasks(kaguya::State& state) {
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
		_tempTasks->push_back(std::make_unique<TestTask>());
	});
}