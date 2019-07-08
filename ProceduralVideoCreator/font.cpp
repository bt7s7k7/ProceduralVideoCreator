#include "pch.h"
#include "font.h"
#include "SDLHelper.h"
#include "globals.h"


std::unordered_map<std::string, std::unordered_map<int, unique_font_ptr>> fonts = {};

TTF_Font* getOrLoadFont(int size, std::string name) {
	auto& font = fonts[name];
	auto iter = font.find(size);
	if (iter == font.end()) {
		auto fontPtr = handleSDLError(TTF_OpenFont((exePath.parent_path() / (name + ".ttf")).string().data(), size));
		auto [niter, success] = font.try_emplace(size, fontPtr);
		iter = niter;
	}
	return iter->second.get();
}
