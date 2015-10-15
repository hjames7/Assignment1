#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdint.h>
#include <string>
#include <vector>


class Buffer {
public:
	bufferSize = 600;
	bufferIndex = 1000;

	// initializing buffer size
	Buffer (std::size_t bufferSize);

	void writeUInt32BE(std::size_t bufferIndex, uint32_t value);
	void writeUInt32BE(uint32_t value);
	void writeInt32BE(std::size_t bufferIndex, int32_t value);
	void writeInt32BE(int32_t value);

	void writeUInt16BE(std::size_t bufferIndex, uint16_t value);
	void writeUInt16BE(uint16_t value);
	void writeInt16BE(std::size_t index, int16_t value);
	void writeInt16BE(int16_t value);


	uint32_t readUInt32BE(std::size_t bufferIndex);
	uint32_t readUInt32BE();
	int32_t readInt32BE(std::size_t bufferIndex);
	int32_t readInt32BE();

	uint16_t readUInt16BE(std::size_t bufferIndex);
	uint16_t readUInt16BE();
	int16_t readInt16BE(std::size_t bufferIndex);
	int16_t readInt16BE();

	std::size_t getReadIndex();
	std::size_t getWriteIndex();

	void setReadIndex(std::size_t bufferIndex);
	void setWriteIndex(std::size_t bufferIndex);

	void printInHex();

private:

	std::vector<uint8_t> _buffer;
	int writeIndex;
	int readIndex;
};


#endif
