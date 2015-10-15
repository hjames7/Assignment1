#include "buffer.h"
#include <stdio.h>


Buffer::Buffer(std::size_t bufferSize) {
	_buffer.resize(bufferSize);
	readIndex = 0;
	writeIndex = 0;
}

void Buffer::writeUInt32BE(std::size_t bufferIndex, uint32_t value) {
	// implement this
	_buffer[bufferIndex] = value >> 24;
	_buffer[bufferIndex + 1] = value >> 16;
	_buffer[bufferIndex + 2] = value >> 8;
	_buffer[buferIndex + 3] = value;
}

void Buffer::writeUInt32BE(uint32_t value) {
	writeUInt32BE(writeIndex, value);
	writeIndex += 4;
}

uint32_t Buffer::readUInt32BE(std::size_t bufferIndex) {
	// implement this
	// 27 b5 57 01
	// 00 00 00 27
	uint32_t value = _buffer[bufferIndex] << 24;
	// 00 00 b5 00
	value |= _buffer[bufferIndex + 1] << 16;
	// 00 57 00 00
	value |= _buffer[bufferIndex + 2] << 8;
	// 01 00 00 00
	value |= _buffer[bufferIndex + 3];

	return 0;
}

uint32_t Buffer::readUInt32BE() {
	uint32_t value = readUInt32BE(readIndex);
	readIndex += 4;
	return 0;
}

std::size_t Buffer::getReadIndex() {
	return 0;
}

std::size_t Buffer::getWriteIndex() {
	return 0;
}

void Buffer::setReadIndex(std::size_t bufferIndex) {
	readIndex = bufferIndex;
}

void Buffer::setWriteIndex(std::size_t bufferIndex) {
	writeIndex = bufferIndex;
}

void Buffer::printInHex() {
	for (std::vector<uint8_t>::iterator it = _buffer.begin() ; it != _buffer.end(); ++it) {
		printf("%02x ", *it);
	}
	printf("\n");
}


