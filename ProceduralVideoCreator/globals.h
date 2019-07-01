#include "pch.h"
#include "SDLHelper.h"
#include "DestroyerType.h"

/*
	Path to the specified file.
*/
inline std::filesystem::path filePath;
/*
	If any changes to the 
	specified file should be ignored. 
	Set when the file becomes
	missing or using --cold
*/
inline bool ignoreChanges;
inline int projectW;
inline int projectH;
inline int previewScale;



/*
	This window should contain all controlls, including the time slider,
	media controlls and render button.
*/
inline std::unique_ptr<SDL_Window, DestroyerType<SDL_Window, SDL_DestroyWindow>> sliderWindow;
/*
	This window should contain the preview of the frame at the current time specified by the slider
	in sliderWindow. When rendering is in progress this window should contain the progress bar.
*/
inline std::unique_ptr<SDL_Window, DestroyerType<SDL_Window, SDL_DestroyWindow>> previewWindow;
inline std::unique_ptr<SDL_Renderer, DestroyerType<SDL_Renderer, SDL_DestroyRenderer>> sliderWindowRenderer;
