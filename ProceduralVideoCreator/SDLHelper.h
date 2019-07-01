#pragma once
#include "pch.h"

class SDLException : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

inline void handleSDLError(int errorCode) {
	if (errorCode == -1) {
		throw SDLException(SDL_GetError());
	}
}

template <typename T>
T* handleSDLError(T* pointer) {
	if (pointer != nullptr) {
		return pointer;
	} else {
		throw SDLException(SDL_GetError());
	}
}