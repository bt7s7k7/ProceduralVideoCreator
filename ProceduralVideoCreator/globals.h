#include "pch.h"
#include "SDLHelper.h"
#include "DestroyerType.h"

inline std::filesystem::path filePath;
inline bool ignoreChanges;

inline std::unique_ptr<SDL_Window, DestroyerType<SDL_Window, SDL_DestroyWindow>> sliderWindow;
inline std::unique_ptr<SDL_Window, DestroyerType<SDL_Window, SDL_DestroyWindow>> previewWindow;
inline std::unique_ptr<SDL_Renderer, DestroyerType<SDL_Renderer, SDL_DestroyRenderer>> sliderWindowRenderer;