/*
 * CircularBuffer.h
 *
 *  Created on: 15/11/2012
 *      Author: Ignacio Mellado-Bataller
 */

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

#include <atlante.h>

template<class _T> class CircularBuffer {
public:
	CircularBuffer();
	CircularBuffer(cvg_int size);
	inline virtual ~CircularBuffer() {}

	void resize(cvg_int newSize);
	void push_back(_T elem);

	template<class _T> class iterator {
	public:
		inline iterator(CircularBuffer<_T> *cb, cvg_int index) { buffer = cb; this->index = index; }
		iterator &operator ++ ();
		cvg_bool operator != (const iterator &it);
		inline cvg_bool operator == (const iterator &it) { return !((*this) != it); }

	private:
		CircularBuffer<_T> *buffer;
		cvg_int index;
	};

	iterator<_T> begin();
	iterator<_T> end();

protected:
	cvg_int mod(cvg_int a, cvg_int b);
private:
	std::vector<_T> buffer;
	cvg_int writeIndex;
	cvg_int count;
};

#include "../sources/CircularBuffer.hh"

#endif /* CIRCULARBUFFER_H_ */
