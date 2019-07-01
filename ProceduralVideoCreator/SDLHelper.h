#pragma once
#include "pch.h"

class SDLException : public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

void handleSDLError(int errorCode) {
	if (errorCode == -1) {
		throw SDLException(SDL_GetError());
	}
}