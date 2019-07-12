#pragma once
#include "pch.h"
#include "DestroyerType.h"

class SDLException : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

inline void handleSDLError(int errorCode) {
	if (errorCode == -1) {
		throw SDLException("[SDL] " + std::string(SDL_GetError()));
	}
}

template <typename T>
T* handleSDLError(T* pointer) {
	if (pointer != nullptr) {
		return pointer;
	} else {
		throw SDLException("[SDL] " + std::string(SDL_GetError()));
	}
}

struct ConvertedTextureData {
	unique_texture_ptr texture;
	SDL_Rect rect;

	ConvertedTextureData() : texture(nullptr), rect{ 0,0,0,0 } {};
};

/*
	Creates a texture from a surface, returned struct contains a rect with the texture
	dimensions, offset by [offX, offY], for easy copying
*/
inline ConvertedTextureData surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface, int offX = 0, int offY = 0) {
	ConvertedTextureData ret;
	ret.texture.reset(handleSDLError(SDL_CreateTextureFromSurface(renderer, surface)));
	ret.rect = { offX, offY, surface->w, surface->h };
	return ret;
}

inline std::unordered_map<SDL_Surface*, ConvertedTextureData> _renderCopySurfaceCachedTextures;

inline void renderCopySurface(SDL_Renderer* renderer, SDL_Surface* surface, int offX = 0, int offY = 0, bool dontCache = false) {
	if (dontCache) {
		auto textureData = surfaceToTexture(renderer, surface, offX, offY);
		handleSDLError(SDL_RenderCopy(renderer, textureData.texture.get(), nullptr, &textureData.rect));
	} else {
		auto iter = _renderCopySurfaceCachedTextures.find(surface);
		if (iter == _renderCopySurfaceCachedTextures.end()) {
			auto ret = _renderCopySurfaceCachedTextures.insert_or_assign(surface, surfaceToTexture(renderer, surface, offX, offY));
			iter = ret.first;
		}
		handleSDLError(SDL_RenderCopy(renderer, iter->second.texture.get(), nullptr, &iter->second.rect));
	}
}

inline void renderCopySurfaceAndFree(SDL_Renderer* renderer, SDL_Surface* surface, int offX = 0, int offY = 0) {
	renderCopySurface(renderer, surface, offX, offY, true);
	SDL_FreeSurface(surface);
}