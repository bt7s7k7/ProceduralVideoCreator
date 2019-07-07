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
inline unique_window_ptr sliderWindow;
/*
	This window should contain the preview of the frame at the current time specified by the slider
	in sliderWindow. When rendering is in progress this window should contain the progress bar.
*/
inline unique_window_ptr previewWindow;
inline unique_renderer_ptr sliderWindowRenderer;
