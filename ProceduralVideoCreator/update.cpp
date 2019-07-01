#include "pch.h"
#include "constants.h"
#include "globals.h"

std::filesystem::file_time_type fileLastModified;

void loadLua(const std::filesystem::path& filePath, std::filesystem::file_time_type& fileLastModified);

bool updateLoop() {
	loadLua(filePath, fileLastModified);
	while (true) {

		

	}
}