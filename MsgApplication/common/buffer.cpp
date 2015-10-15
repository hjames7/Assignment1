#include "buffer.h"

// Default constructor for safety reasons
Buffer::Buffer() {
	_buffer.resize(1000);
	this->bufferSize = 1000;
	this->readIndex = 0;
	this->writeIndex = 0;
}

Buffer::Buffer(std::size_t n) {
	_buffer.resize(n);
	this->bufferSize = n;
	this->readIndex = 0;
	this->writeIndex = 0;
}

// Unsigned 32-bit integer [de]serializing
void Buffer::writeUInt32BE(std::size_t bufferIndex, uint32_t value) {
	_buffer[bufferIndex] = value >> 24;
	_buffer[bufferIndex + 1] = value >> 16;
	_buffer[bufferIndex + 2] = value >> 8;
	_buffer[bufferIndex + 3] = value;
}
void Buffer::writeUInt32BE(uint32_t value) {
	writeUInt32BE(writeIndex, value);
	writeIndex += 4;
}
uint32_t Buffer::readUInt32BE(std::size_t bufferIndex) {
	uint32_t value = _buffer[bufferIndex] << 24;
	value |= _buffer[bufferIndex + 1] << 16;
	value |= _buffer[bufferIndex + 2] << 8;
	value |= _buffer[bufferIndex + 3];

	return 0;
}
uint32_t Buffer::readUInt32BE() {
	uint32_t value = readUInt32BE(readIndex);
	readIndex += 4;
	return 0;
}

// Signed 32-bit integer [de]serializing
void Buffer::writeInt32BE(std::size_t bufferIndex, int32_t value) {
	_buffer[bufferIndex] = value >> 24;
	_buffer[bufferIndex + 1] = value >> 16;
	_buffer[bufferIndex + 2] = value >> 8;
	_buffer[bufferIndex + 3] = value;
}
void Buffer::writeInt32BE(int32_t value) {
	writeInt32BE(writeIndex, value);
	writeIndex += 4;
}
int32_t Buffer::readInt32BE(std::size_t bufferIndex) {
	int32_t value = _buffer[bufferIndex] << 24;
	value |= _buffer[bufferIndex + 1] << 16;
	value |= _buffer[bufferIndex + 2] << 8;
	value |= _buffer[bufferIndex + 3];

	return 0;
}
int32_t Buffer::readInt32BE() {
	int32_t value = readInt32BE(readIndex);
	readIndex += 4;
	return 0;
}

// Unsigned 16-bit integer [de]serialzing
void Buffer::writeUInt16BE(std::size_t bufferIndex, uint16_t value) {
	_buffer[bufferIndex] = value >> 24;
	_buffer[bufferIndex + 1] = value >> 16;
	_buffer[bufferIndex + 2] = value >> 8;
	_buffer[bufferIndex + 3] = value;
}
void Buffer::writeUInt16BE(uint16_t value) {
	writeUInt16BE(writeIndex, value);
	writeIndex += 4;
}
uint16_t Buffer::readUInt16BE(std::size_t bufferIndex) {
	uint16_t value = _buffer[bufferIndex] << 24;
	value |= _buffer[bufferIndex + 1] << 16;
	value |= _buffer[bufferIndex + 2] << 8;
	value |= _buffer[bufferIndex + 3];

	return 0;
}
uint16_t Buffer::readUInt16BE() {
	uint16_t value = readUInt16BE(readIndex);
	readIndex += 4;
	return 0;
}

// Signed 16-bit integer [de]serialzing
void Buffer::writeInt16BE(std::size_t bufferIndex, int16_t value) {
	_buffer[bufferIndex] = value >> 24;
	_buffer[bufferIndex + 1] = value >> 16;
	_buffer[bufferIndex + 2] = value >> 8;
	_buffer[bufferIndex + 3] = value;
}
void Buffer::writeInt16BE(int16_t value) {
	writeInt16BE(writeIndex, value);
	writeIndex += 4;
}
int16_t Buffer::readInt16BE(std::size_t bufferIndex) {
	int16_t value = _buffer[bufferIndex] << 24;
	value |= _buffer[bufferIndex + 1] << 16;
	value |= _buffer[bufferIndex + 2] << 8;
	value |= _buffer[bufferIndex + 3];

	return 0;
}
int16_t Buffer::readInt16BE() {
	int16_t value = readInt16BE(readIndex);
	readIndex += 4;
	return 0;
}

// Short [de]serializing
void Buffer::writeShortBE(std::size_t bufferIndex, short value) {
	_buffer[bufferIndex] = value >> 24;
	_buffer[bufferIndex + 1] = value >> 16;
	_buffer[bufferIndex + 2] = value >> 8;
	_buffer[bufferIndex + 3] = value;
}
void Buffer::writeShortBE(short value) {
	writeShortBE(writeIndex, value);
	writeIndex += 4;
}
short Buffer::readShortBE(std::size_t bufferIndex) {
	short value = _buffer[bufferIndex] << 24;
	value |= _buffer[bufferIndex + 1] << 16;
	value |= _buffer[bufferIndex + 2] << 8;
	value |= _buffer[bufferIndex + 3];

	return 0;
}
short Buffer::readShortBE() {
	short value = readShortBE(readIndex);
	readIndex += 4;
	return 0;
}

// String [de]serializing (???)
//void Buffer::writeStringBE(std::size_t bufferIndex, std::string value) {
//	_buffer[bufferIndex] = value >> 24;
//	_buffer[bufferIndex + 1] = value >> 16;
//	_buffer[bufferIndex + 2] = value >> 8;
//	_buffer[bufferIndex + 3] = value;
//}
//void Buffer::writeStringBE(std::string value) {
//	writeStringBE(writeIndex, value);
//	writeIndex += 4;
//}
//std::string Buffer::readStringBE(std::size_t bufferIndex) {
//	std::string value = _buffer[bufferIndex] << 24;
//	value |= _buffer[bufferIndex + 1] << 16;
//	value |= _buffer[bufferIndex + 2] << 8;
//	value |= _buffer[bufferIndex + 3];
//
//	return 0;
//}
//std::string Buffer::readStringBE() {
//	std::string value = readStringBE(readIndex);
//	readIndex += 4;
//	return 0;
//}

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


