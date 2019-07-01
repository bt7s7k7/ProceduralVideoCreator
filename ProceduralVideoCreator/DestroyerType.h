#pragma once

template <typename T, void (*DESTROYER)(T*)>
struct DestroyerType {
	void operator()(T* ptr) {
		DESTROYER(ptr);
	}
};