/*
 */
#include <ByteStream.hpp>
#include <string.h>
#include <string>
#include <map>
#include <cassert>
using namespace bs;
/*
 */
unsigned long ByteStream::CHUNK_SIZE = 16000;
/*
 */
ByteStream::ByteStream(const ByteStream& byteStream) :
	bytes(byteStream.bytes),
	chunkIndex(byteStream.chunkIndex),
	writeIndex(byteStream.writeIndex),
	readIndex(byteStream.readIndex),
	byteLength(byteStream.byteLength),
	newByteLength(byteStream.newByteLength),
	validPayload(byteStream.validPayload)
{
};
/*
 */
ByteStream::ByteStream(
	const unsigned long& _bytesSize,
	const std::shared_ptr<char>& _bytes,
	const unsigned long& copyBytesOffset,
	const unsigned long& copyBytesLength
):
	writeIndex(_bytesSize)
{
	unsigned long newBytesSize =
		copyBytesLength == (std::numeric_limits<unsigned long>::max)() ? _bytesSize : copyBytesLength;
	std::shared_ptr<char> newBytes((char*)malloc(newBytesSize), free);
	auto _bytesPointer = _bytes.get();
	bytes.swap(newBytes);
	memcpy(bytes.get(), _bytesPointer + copyBytesOffset, newBytesSize);
	bytesSize = newBytesSize;
};
// unsigned long bytesRemainingSize = _bytesSize - (bytesActualSize + copyBytesOffset);
// if (bytesRemainingSize)
// {
// 	auto bytesRemainingIndex = remainingBytesSize;
// 	Memory<char>::allocate(bytesRemainingIndex, bytesRemainingSize, (char *&)remainingBytes.pointer,
// 					 (unsigned long &)remainingBytesSize, 0, false, true);
// 	memCpy(remainingBytes.pointer + bytesRemainingIndex, _bytesPointer + copyBytesLength + copyBytesOffset, bytesRemainingSize);
// }
/*
 */
ByteStream& ByteStream::operator=(const ByteStream& byteStream)
{
	bytesSize = byteStream.bytesSize;
	bytes = byteStream.bytes;
	chunkIndex = byteStream.chunkIndex;
	writeIndex = byteStream.writeIndex;
	readIndex = byteStream.readIndex;
	byteLength = byteStream.byteLength;
	newByteLength = byteStream.newByteLength;
	validPayload = byteStream.validPayload;
	return *this;
};
/*
 */
void ByteStream::resize(const unsigned long& newSize, const unsigned long& oldSize)
{
	auto oldPointer = bytes.get();
	char *newPointer = (char *)malloc(newSize);
	if (newSize > oldSize)
	{
		memcpy(newPointer, oldPointer, oldSize);
	}
	else
	{
		memcpy(newPointer, oldPointer, newSize);
	}
	std::shared_ptr<char> newBytes(newPointer, free);
	bytes = newBytes;
	bytesSize = newSize;
}

/*
 */
const unsigned long ByteStream::write(const void* pointer, const unsigned long& size)
{
	std::lock_guard<std::mutex> lock(mutex);
	resize(bytesSize + size, bytesSize);
	memcpy(&bytes.get()[writeIndex], pointer, size);
	writeIndex += size;
	return size;
};
/// ByteStream operator (T)s
/*
 */
#define BYTESTREAM_WRITE_TYPE(TYPE)                                                                                                                                \
	template <>                                                                                                                                                \
	const unsigned long ByteStream::write(const TYPE &value)                                                                                                      \
	{                                                                                                                                                          \
		return write(&value, sizeof(value));                                                                                                               \
	}
BYTESTREAM_WRITE_TYPE(bool);
BYTESTREAM_WRITE_TYPE(unsigned char);
BYTESTREAM_WRITE_TYPE(char);
BYTESTREAM_WRITE_TYPE(unsigned short);
BYTESTREAM_WRITE_TYPE(short);
BYTESTREAM_WRITE_TYPE(unsigned int);
BYTESTREAM_WRITE_TYPE(int);
BYTESTREAM_WRITE_TYPE(unsigned long);
BYTESTREAM_WRITE_TYPE(long);
BYTESTREAM_WRITE_TYPE(unsigned long long);
BYTESTREAM_WRITE_TYPE(long long);
BYTESTREAM_WRITE_TYPE(float);
BYTESTREAM_WRITE_TYPE(double);
BYTESTREAM_WRITE_TYPE(long double);
/*
 */
template <>
const unsigned long ByteStream::write(const std::pair<std::shared_ptr<char>, unsigned long>& pointerPair)
{
	unsigned long bytesWritten = 0;
	auto pointer = std::get<0>(pointerPair).get();
	const auto& pointerSize = std::get<1>(pointerPair);
	if (byteLength)
	{
		if (!newByteLength && bytesSize > sizeof(unsigned long))
		{
			unsigned long& size = *(unsigned long*)(&bytes.get()[0]);
			size += pointerSize;
		}
		else
		{
			newByteLength = false;
			bytesWritten += write<const unsigned long&>(pointerSize);
		}
	}
	bytesWritten += write(pointer, pointerSize + (byteLength ? 0 : 1));
	return bytesWritten;
};
/*
 */
template <>
const unsigned long ByteStream::write(const char* pointer)
{
	const unsigned long size = strlen(pointer);
	return write(pointer, size + 1);
};
/*
 */
template <>
const unsigned long ByteStream::write(char* pointer)
{
	return write((const char*)pointer);
};
/*
 */
template <>
const unsigned long ByteStream::write(const std::string& string)
{
	return write((char*)string.c_str(), string.size() + 1);
};
/*
 */
template <>
const unsigned long ByteStream::write(const ByteStream& byteStream)
{
	return write<const std::pair<std::shared_ptr<char>, unsigned long>&>({byteStream.bytes, byteStream.bytesSize});
};
BYTE_STREAM_WRITE_VECTOR(char);
BYTE_STREAM_WRITE_VECTOR(unsigned char);
BYTE_STREAM_WRITE_VECTOR(short);
BYTE_STREAM_WRITE_VECTOR(unsigned short);
BYTE_STREAM_WRITE_VECTOR(int);
BYTE_STREAM_WRITE_VECTOR(unsigned int);
BYTE_STREAM_WRITE_VECTOR(long);
BYTE_STREAM_WRITE_VECTOR(unsigned long);
BYTE_STREAM_WRITE_VECTOR(long long);
BYTE_STREAM_WRITE_VECTOR(unsigned long long);
BYTE_STREAM_WRITE_VECTOR(float);
BYTE_STREAM_WRITE_VECTOR(double);
BYTE_STREAM_WRITE_VECTOR(long double);
/*
 */
template <typename KeyT, typename ValueT>
const unsigned long mapToByteStream(ByteStream& byteStream, const std::map<KeyT, ValueT>& map)
{
	unsigned long bytesWritten = 0;
	auto mapSize = map.size();
	bytesWritten += byteStream.write<const unsigned long&>(mapSize);
	for (const auto& mapPair : map)
	{
		bytesWritten += byteStream.write<const KeyT&>(mapPair.first);
		byteStream.newByteLength = true;
		bytesWritten += byteStream.write<const ValueT&>(mapPair.second);
		byteStream.newByteLength = true;
		continue;
	}
	return bytesWritten;
};
/*
 */
#define BYTE_STREAM_WRITE_MAP(KEY_TYPE, VALUE_TYPE)                                \
	template <>                                                                      \
	const unsigned long ByteStream::write(const std::map<KEY_TYPE, VALUE_TYPE> &map) \
	{                                                                                \
		return mapToByteStream<KEY_TYPE, VALUE_TYPE>(*this, map);                      \
	}
/*
 */
BYTE_STREAM_WRITE_MAP(std::string, ByteStream);
/*
 */
static void amountLt0MemMove(char *&pointer, const unsigned long &index, const unsigned long &size, const long &amountAbsolute)
{
	memset(&pointer[index], 0, amountAbsolute * 1);
	unsigned long indexPlusAmountAbsolute = index + amountAbsolute;
	if (indexPlusAmountAbsolute < size)
	{
		unsigned long indexMove = (size - indexPlusAmountAbsolute) * 1;
		if (indexMove)
		{
			memmove(&pointer[index], &pointer[indexPlusAmountAbsolute], indexMove);
			long indexBot = size - amountAbsolute;
			memset(&pointer[indexBot], 0, amountAbsolute * 1);
		}
	}
};
/*
 */
const bool ByteStream::read(void* pointer, const unsigned long& size, unsigned long& bytesRead, const bool& removeBytes)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (bytesSize == 0)
	{
		return false;
	}
	auto bytesPointer = bytes.get();
	memcpy(pointer, bytesPointer + readIndex, size);
	if (removeBytes)
	{
		amountLt0MemMove(bytesPointer, readIndex, bytesSize, size);
		resize(bytesSize - size, bytesSize);
		writeIndex -= size;
	}
	else
	{
		readIndex += size;
	}
	bytesRead += size;
	return true;
};
/// ByteStream T operators()
/*
 */
#define BYTESTREAM_READ_TYPE(TYPE)                                                                                                                                 \
	template <>                                                                                                                                                \
	const bool ByteStream::read(TYPE &value, unsigned long &bytesRead, const bool &removeBytes)                                                             \
	{                                                                                                                                                          \
		return read((char *)&value, sizeof(value), bytesRead, removeBytes);                                                                            \
	}
BYTESTREAM_READ_TYPE(bool);
BYTESTREAM_READ_TYPE(unsigned char);
BYTESTREAM_READ_TYPE(char);
BYTESTREAM_READ_TYPE(unsigned short);
BYTESTREAM_READ_TYPE(short);
BYTESTREAM_READ_TYPE(unsigned int);
BYTESTREAM_READ_TYPE(int);
BYTESTREAM_READ_TYPE(unsigned long);
BYTESTREAM_READ_TYPE(long);
BYTESTREAM_READ_TYPE(unsigned long long);
BYTESTREAM_READ_TYPE(long long);
BYTESTREAM_READ_TYPE(float);
BYTESTREAM_READ_TYPE(double);
BYTESTREAM_READ_TYPE(long double);
/*
 */
template <>
const bool ByteStream::read(std::pair<std::shared_ptr<char>, unsigned long>& value, unsigned long& bytesRead,
														const bool& removeBytes)
{
	unsigned long size = 0;
	if (byteLength)
	{
		if (!read(size, bytesRead, removeBytes))
		{
			return false;
		}
	}
	else
	{
		size = strlen(bytes.get() + readIndex);
	}
	auto& readBytes = std::get<0>(value);
	auto& readSize = std::get<1>(value);
	readBytes = {(char*)malloc(size), free};
	readSize = size;
	bool readValue = read(readBytes.get(), size, bytesRead, removeBytes);
	if (readValue && !byteLength)
	{
		char trailingZero = -1;
		readValue = read(&trailingZero, 1, bytesRead, removeBytes);
		assert(trailingZero == 0 && "Must include trailingZero");
	}
	return readValue;
};
/*
 */
template <>
const bool ByteStream::read(char*& pointer, unsigned long& bytesRead, const bool& removeBytes)
{
	if (!bytesSize)
	{
		return false;
	}
	unsigned long size = strlen((const char*)(bytes.get() + readIndex));
	pointer = (char*)malloc(size + 1);
	bool readValue = read(pointer, size + 1, bytesRead, removeBytes);
	return readValue;
};
/*
 */
template <>
const bool ByteStream::read(std::string& value, unsigned long& bytesRead, const bool& removeBytes)
{
	char* bytes = 0;
	unsigned long bytesLength = 0;
	if (!read(bytes, bytesLength, removeBytes))
	{
		bytesRead += bytesLength;
		return false;
	}
	bytesRead += bytesLength;
	bytesLength--;
	value.clear();
	value.append(bytes, bytes + bytesLength);
	free(bytes);
	return true;
};
/*
 */
template <>
const bool ByteStream::read(ByteStream& value, unsigned long& bytesRead, const bool& removeBytes)
{
	std::pair<std::shared_ptr<char>, unsigned long> bytesPair;
	if (!read(bytesPair, bytesRead, removeBytes))
	{
		return false;
	}
	value = {bytesPair.second, bytesPair.first};
	return true;
};
BYTE_STREAM_READ_VECTOR(char);
BYTE_STREAM_READ_VECTOR(unsigned char);
BYTE_STREAM_READ_VECTOR(short);
BYTE_STREAM_READ_VECTOR(unsigned short);
BYTE_STREAM_READ_VECTOR(int);
BYTE_STREAM_READ_VECTOR(unsigned int);
BYTE_STREAM_READ_VECTOR(long);
BYTE_STREAM_READ_VECTOR(unsigned long);
BYTE_STREAM_READ_VECTOR(long long);
BYTE_STREAM_READ_VECTOR(unsigned long long);
BYTE_STREAM_READ_VECTOR(float);
BYTE_STREAM_READ_VECTOR(double);
BYTE_STREAM_READ_VECTOR(long double);
/*
 */
template <typename KeyT, typename ValueT>
const bool mapFromByteStream(ByteStream& byteStream, std::map<KeyT, ValueT>& map, unsigned long& bytesRead,
														const bool& removeBytes)
{
	unsigned long size;
	if (!byteStream.read(size, bytesRead, removeBytes))
	{
		return false;
	}
	for (unsigned long index = 1; index <= size; index++)
	{
		KeyT key;
		if (!byteStream.read(key, bytesRead, removeBytes))
		{
			return false;
		}
		ValueT value;
		if (!byteStream.read(value, bytesRead, removeBytes))
		{
			return false;
		}
		map[key] = value;
	}
	return true;
};
/*
 */
#define BYTE_STREAM_READ_MAP(KEY_TYPE, VALUE_TYPE)                                                                    \
	template <>                                                                                                         \
	const bool ByteStream::read(std::map<KEY_TYPE, VALUE_TYPE> &map, unsigned long &bytesRead, const bool &removeBytes) \
	{                                                                                                                   \
		return mapFromByteStream<KEY_TYPE, VALUE_TYPE>(*this, map, bytesRead, removeBytes);                               \
	}
/*
 */
BYTE_STREAM_READ_MAP(std::string, ByteStream);
/*
 */
const bool ByteStream::empty()
{
	return !bytesSize;
}

/*
 */
