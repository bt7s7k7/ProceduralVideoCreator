#pragma once
#include "pch.h"

/*
	This type executes the function D
	on operator().
*/
template <typename T, void (*D)(T*)>
struct DestroyerType {
	void operator()(T* ptr) {
		D(ptr);
	}
};


#ifdef SDL_h_
using unique_window_ptr = std::unique_ptr<SDL_Window, DestroyerType<SDL_Window, SDL_DestroyWindow>>;
using unique_renderer_ptr = std::unique_ptr<SDL_Renderer, DestroyerType<SDL_Renderer, SDL_DestroyRenderer>>;
using unique_surface_ptr = std::unique_ptr<SDL_Surface, DestroyerType<SDL_Surface, SDL_FreeSurface>>;
using unique_texture_ptr =  std::unique_ptr<SDL_Texture, DestroyerType<SDL_Texture, SDL_DestroyTexture>>;

#endif
#ifdef SDL_TTF_H_
using unique_font_ptr = std::unique_ptr<TTF_Font, DestroyerType<TTF_Font, TTF_CloseFont>>;
#endif // SDL_TTF_H_
