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
inline int maxPreviewSize;
inline double previewScale;
inline double projectLength;
inline int projectFramerate = 30;
inline bool nowindow;

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

struct LuaSurfacePtr {
	std::shared_ptr<unique_surface_ptr> surface;

	std::string ToString() {
		if (!surface) return "<EMPTY LuaSurfacePtr>";
		if (!*surface) return "<INVALID LuaSurfacePtr>";
		return "<LuaSurfacePtr: " + std::to_string((std::size_t)(surface->get())) + ">";
	}
};

inline std::filesystem::path exePath;

/*
	State of the Lua VM. Doesn't change
	during the program runtime
*/
inline kaguya::State luaState;
