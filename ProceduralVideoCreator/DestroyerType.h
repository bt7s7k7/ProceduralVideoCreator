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
//using unique_surface_ptr = std::unique_ptr<SDL_Surface, DestroyerType<SDL_Surface, SDL_FreeSurface>>;
using unique_texture_ptr =  std::unique_ptr<SDL_Texture, DestroyerType<SDL_Texture, SDL_DestroyTexture>>;

struct unique_surface_ptr {
	SDL_Surface* ptr;

	inline SDL_Surface* get() { return ptr; };
	inline SDL_Surface* operator->() { return ptr; };

	unique_surface_ptr(SDL_Surface* ptr_) : ptr(ptr_) {};

	~unique_surface_ptr() {
		SDL_FreeSurface(ptr);
	}
};

#endif
#ifdef SDL_TTF_H_
using unique_font_ptr = std::unique_ptr<TTF_Font, DestroyerType<TTF_Font, TTF_CloseFont>>;
#endif // SDL_TTF_H_
