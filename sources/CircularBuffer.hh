/*
 * CircularBuffer.cpp
 *
 *  Created on: 15/11/2012
 *      Author: Ignacio Mellado-Bataller
 */

#ifndef CIRCULARBUFFER_CPP_
#define CIRCULARBUFFER_CPP_

#define template_def	template<class _T>
#define CircularBuffer_	CircularBuffer<_T>

template_def CircularBuffer_::CircularBuffer() {
	writeIndex = count = 0;
}

template_def CircularBuffer_::CircularBuffer(cvg_int size) {
	resize(size);
}

template_def CircularBuffer_::resize(cvg_int newSize) {
	data.resize(newSize);
	writeIndex = count = 0;
}

template_def CircularBuffer_::push_back(_T elem) {
	data[writeIndex] = elem;
	writeIndex = (writeIndex + 1) % data.size();
	if (count < data.size()) count ++;
}

template_def cvg_int CircularBuffer_::mod(cvg_int a, cvg_int b) {
	if (a > 0) return a % b;
	else if (a < 0) return b - ((-a) % b);
	else return 0;
}

template_def CircularBuffer_::iterator &CircularBuffer_::iterator::operator ++ () {
	if (buffer->count == 0 ||
		mod(buffer->writeIndex - index, buffer->data.size()) > buffer->count
		) throw std::out_of_range();
	index = (index + 1) % buffer.size();
}

template_def CircularBuffer_::iterator &CircularBuffer_::iterator::operator != (const iterator &it) {
	return this->index != it.index;
}

cvg_bool operator != (const iterator &it);

#endif /* CIRCULARBUFFER_CPP_ */
