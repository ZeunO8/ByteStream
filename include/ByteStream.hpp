/*
 */
#pragma once
#include <memory>
#include <vector>
#include <tuple>
#include <mutex>
#include <limits>
/*
 */
namespace bs
{
	struct ByteStream
	{
       public:
        unsigned long bytesSize = 0;
		std::shared_ptr<char> bytes;
		unsigned long chunkIndex = 0;
		unsigned long writeIndex = 0;
		unsigned long readIndex = 0;
		bool byteLength = true;
		bool newByteLength = false;
		bool validPayload = false;
		static unsigned long CHUNK_SIZE;
		std::mutex mutex;

       public:
		ByteStream() = default;
		ByteStream(const ByteStream &byteStream);
		/**
		 * @brief Constructs a new ByteStream from bytes, offset and count. If count is default (unsigned long max)
		 * then all bytes from the offset are copied
		 */
		ByteStream(
			const unsigned long &_bytesSize,
            const std::shared_ptr<char> &bytes,
            const unsigned long &copyBytesOffset = 0,
			const unsigned long &copyBytesCount = (std::numeric_limits<unsigned long>::max)()
		);
		/**
		 * @brief Constructs a new ByteStream from bytes, offset and count. If count is default (unsigned long max)
		 * then all bytes from the offset are copied
		 */
		ByteStream(
			const unsigned long &_bytesSize,
			const std::shared_ptr<char> &bytes,
			std::shared_ptr<char>& remainingBytes,
			unsigned long &remainingBytesSize,
			const unsigned long &copyBytesOffset = 0,
			const unsigned long &copyBytesCount = (std::numeric_limits<unsigned long>::max)()
		);
		ByteStream &operator=(const ByteStream &byteStream);
	private:
        /**
		 *
		 */
		void resize(const unsigned long& newSize, const unsigned long& oldSize);
	public:
		/**
		 * @brief Writes the contents of pointer, write 'size' amount of bytes
		 *
		 * @param pointer The data pointer
		 * @param size The size to write
		 * @return const unsigned long The amount of bytes written
		 */
		const unsigned long write(const void *pointer, const unsigned long &size);
		/**
		 * @brief Reads from bytes into pointer, reads 'size' amount of bytes
		 *
		 * @param pointer The pointer to read into
		 * @param size The size to read
		 * @param bytesRead Tracking variable of bytes read
		 * @param removeBytes Whether or not to remove the read bytes from bytes
		 * @return const bool true if read successful, false if not
		 */
		const bool read(void *pointer, const unsigned long &size, unsigned long &bytesRead,
				   const bool &removeBytes);
		/**
		 * @brief Writes a Type T value to the stream
		 *
		 * @param value The Type T value to write
		 */
		template <typename T>
		const unsigned long write(T value);
		/**
		 * @brief Reads a given Type T into value from the stream, returns true if read correctly, false if
		 */
		template <typename T>
		const bool read(T &value, unsigned long &bytesRead, const bool &removeBytes = true);

       private:
		/**
		 * @brief Helper function for writeTuple
		 */
		template <std::size_t Index, typename... Args>
		void writeTupleHelper(const std::tuple<Args...> &tuple, unsigned long &bytesWritten)
		{
			if constexpr (Index < sizeof...(Args))
			{
				using ElementType = std::tuple_element_t<Index, std::tuple<Args...>>;
				bytesWritten += write<const ElementType &>(std::get<Index>(tuple));
				writeTupleHelper<Index + 1>(tuple, bytesWritten);
			}
		};
		/**
		 * @brief Helper function for readTuple
		 */
		template <std::size_t Index, typename... Args>
		void readTupleHelper(std::tuple<Args...> &tuple, unsigned long &bytesRead, const bool &removeBytes)
		{
			if constexpr (Index < sizeof...(Args))
			{
				using ElementType = std::tuple_element_t<Index, std::tuple<Args...>>;
				ElementType value;
				if (read(value, bytesRead, removeBytes))
				{
					std::get<Index>(tuple) = value;
					readTupleHelper<Index + 1>(tuple, bytesRead, removeBytes);
				}
			}
		};

       public:
		/**
		 * @brief Writes a tuple of ArgTypes into the stream
		 */
		template <typename... Args>
		unsigned long writeTuple(const std::tuple<Args...> &tuple)
		{
			unsigned long bytesWritten = 0;
			writeTupleHelper<0>(tuple, bytesWritten);
			return bytesWritten;
		};
		/**
		 * @brief Reads a list of ArgTypes into a std::tuple
		 */
		template <typename... Args>
		std::tuple<Args...> readTuple(unsigned long &bytesRead, const bool &removeBytes = true)
		{
			std::tuple<Args...> tuple;
			readTupleHelper<0>(tuple, bytesRead, removeBytes);
			return tuple;
		};
		/**
		 * @brief Returns a boolean value indicating empty
		 */
		const bool empty();
	};
/*
*/
#define BYTE_STREAM_READ_VECTOR(TYPE)                                                                     \
template <>                                                                                               \
const bool ByteStream::read(std::vector<TYPE> &vector, unsigned long &bytesRead, const bool &removeBytes) \
{                                                                                                         \
	unsigned long _size = 0;                                                                                \
	if (!read(_size, bytesRead, removeBytes))																															  \
	{                                                                                                       \
		return false;                                                                                         \
	}                                                                                                       \
	for (unsigned long count = 1; count <= _size; count++)                                                  \
	{                                                                                                       \
		TYPE value;                                                                                           \
		if (!read(value, bytesRead, removeBytes))                                                             \
		{                                                                                                     \
			return false;                                                                                       \
		}                                                                                                     \
		vector.push_back(value);                                                                              \
	}                                                                                                       \
	return true;                                                                                            \
}
}
/*
 */
