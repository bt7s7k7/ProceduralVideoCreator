#include "pch.h"
#include "font.h"
#include "SDLHelper.h"
#include "globals.h"


struct font_t {
	std::vector<Uint8> data;
	std::unordered_map<int, unique_font_ptr> loadedSizes;

	TTF_Font* operator[](int size) {
		if (data.size() == 0) {
			throw std::runtime_error("Font data not initialized. // Forgot to call Init()?");
		}

		auto iter = loadedSizes.find(size);
		if (iter == loadedSizes.end()) {
			auto rwops = handleSDLError(SDL_RWFromMem(data.data(), data.size()));
			auto fontPtr = handleSDLError(TTF_OpenFontRW(rwops, 1, size));
			auto [niter, success] = loadedSizes.try_emplace(size, fontPtr);
			iter = niter;
		}
		return iter->second.get();
	}

	font_t & Init(std::string name) {
		if (data.size() > 0) return *this;
		auto rwops = handleSDLError(SDL_RWFromFile((exePath.parent_path() / (name + ".ttf")).string().data(), "rb"));

		data.resize(static_cast<std::size_t>(handleSDLError(SDL_RWsize(rwops))));

		if (SDL_RWread(rwops, data.data(), data.size(), 1) <= 0) {
			handleSDLError(-1);
		}

		return *this;
	}
};

std::unordered_map<std::string, font_t> fonts = {};

TTF_Font* getOrLoadFont(int size, std::string name) {
	return fonts[name].Init(name)[size];
}
