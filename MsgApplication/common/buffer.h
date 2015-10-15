#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdint.h>
#include <string>
#include <vector>
#include <stdio.h>

class Buffer {
public:
	// initializing buffer size
	Buffer ();	// Default constructor so nobody gets hurt
	Buffer (std::size_t n);

	// Serializing integers
	void writeUInt32BE(std::size_t bufferIndex, uint32_t value);
	void writeUInt32BE(uint32_t value);
	void writeInt32BE(std::size_t bufferIndex, int32_t value);
	void writeInt32BE(int32_t value);
	void writeUInt16BE(std::size_t bufferIndex, uint16_t value);
	void writeUInt16BE(uint16_t value);
	void writeInt16BE(std::size_t index, int16_t value);
	void writeInt16BE(int16_t value);

	// Serializing shorts
	void writeShortBE(std::size_t bufferIndex, short value);
	void writeShortBE(short value);

	// Serializing strings (???)
	/*void writeStringBE(std::size_t bufferIndex, std::string value);
	void writeStringBE(std::string value);*/

	// Deserializing integers
	uint32_t readUInt32BE(std::size_t bufferIndex);
	uint32_t readUInt32BE();
	int32_t readInt32BE(std::size_t bufferIndex);
	int32_t readInt32BE();
	uint16_t readUInt16BE(std::size_t bufferIndex);
	uint16_t readUInt16BE();
	int16_t readInt16BE(std::size_t bufferIndex);
	int16_t readInt16BE();

	// Deserializing shorts
	short readShortBE(std::size_t bufferIndex);
	short readShortBE();

	// Deserializing strings (???)
	/*std::string readStringBE(std::size_t bufferIndex);
	std::string readStringBE();*/

	std::size_t getReadIndex();
	std::size_t getWriteIndex();

	void setReadIndex(std::size_t bufferIndex);
	void setWriteIndex(std::size_t bufferIndex);

	void printInHex();

private:

	std::vector<uint8_t> _buffer;
	size_t bufferSize;
	int writeIndex;
	int readIndex;
};


#endif
