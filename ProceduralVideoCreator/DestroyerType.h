#pragma once

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