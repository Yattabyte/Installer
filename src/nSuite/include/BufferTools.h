#pragma once
#ifndef BUFFER_TOOLS_H
#define BUFFER_TOOLS_H


/** Namespace to keep buffer-related operations grouped together. */
namespace BFT {
	/** Compresses a source buffer into an equal or smaller-sized destination buffer.
	After compression, it applies a small header describing how large the uncompressed buffer is.
	destinationBuffer format:
	------------------------------------------------------------------
	| header: identifier title, uncompressed size | compressed data  |
	------------------------------------------------------------------
	@note						caller expected to clean-up destinationBuffer on their own
	@param	sourceBuffer		the original buffer to read from.
	@param	sourceSize			the size of the source buffer in bytes.
	@param	destinationBuffer	pointer to the destination buffer, which will hold compressed contents.
	@param	destinationSize		reference updated with the size in bytes of the compressed destinationBuffer.
	@return						true if compression success, false otherwise. */
	bool CompressBuffer(char * sourceBuffer, const size_t & sourceSize, char ** destinationBuffer, size_t & destinationSize);
	/** Decompressess a source buffer into an equal or larger-sized destination buffer.
	@note						caller expected to clean-up destinationBuffer on their own
	@param	sourceBuffer		the original buffer to read from.
	@param	sourceSize			the size of the source buffer in bytes.
	@param	destinationBuffer	pointer to the destination buffer, which will hold decompressed contents.
	@param	destinationSize		reference updated with the size in bytes of the decompressed destinationBuffer.
	@return						true if decompression success, false otherwise. */
	bool DecompressBuffer(char * sourceBuffer, const size_t & sourceSize, char ** destinationBuffer, size_t & destinationSize);
	/** Parses the header of an nSuite compressed buffer, updating parameters like the data portion of the buffer if successfull.
	@param	buffer				the buffer to parse.
	@param	bufferSize			the size of the buffer, in bytes.
	@param	uncompressedSize	reference updated with the uncompressed size of the package.
	@param	dataPointer			pointer to the data portion of the buffer.
	@param	dataSize			size of the remaining data portion of the buffer (package size - header size).
	@return						true if the nSuite compressed buffer is formatted correctly and could be parsed, false otherwise. */
	bool ParseHeader(char * buffer, const size_t & bufferSize, size_t & uncompressedSize, char ** dataPointer, size_t & dataSize);
	/** Processes two input buffers, diffing them.
	Generates a compressed instruction set dictating how to get from the old buffer to the new buffer.
	buffer_diff format:
	-----------------------------------------------------------------------------------
	| header: identifier title, final target file size | compressed instruction data  |
	-----------------------------------------------------------------------------------
	@note						caller expected to clean-up buffer_diff on their own
	@param	buffer_old			the older of the 2 buffers.
	@param	size_old			the size of the old buffer.
	@param	buffer_new			the newer of the 2 buffers.
	@param	size_new			the size of the new buffer.
	@param	buffer_diff			pointer to store the diff buffer at.
	@param	size_diff			reference updated with the size of the compressed diff buffer.
	@param	instructionCount	(optional) pointer to update with the number of instructions processed.
	@return						true if diff success, false otherwise. */
	bool DiffBuffers(char * buffer_old, const size_t & size_old, char * buffer_new, const size_t & size_new, char ** buffer_diff, size_t & size_diff, size_t * instructionCount = nullptr);
	/** Reads from a compressed instruction set, uses it to patch the 'older' buffer into the 'newer' buffer
	@note						caller expected to clean-up buffer_new on their own
	@param	buffer_old			the older of the 2 buffers.
	@param	size_old			the size of the old buffer.
	@param	buffer_new			pointer to store the newer of the 2 buffers.
	@param	size_new			reference updated with the size of the new buffer.
	@param	buffer_diff			the compressed diff buffer (instruction set).
	@param	size_diff			the size of the compressed diff buffer.
	@param	instructionCount	(optional) pointer to update with the number of instructions processed.
	@return						true if patch success, false otherwise. */
	bool PatchBuffer(char * buffer_old, const size_t & size_old, char ** buffer_new, size_t & size_new, char * buffer_diff, const size_t & size_diff, size_t * instructionCount = nullptr);
	/** Generates a hash value for the buffer provided, using the buffers' contents.
	@param	buffer				pointer to the buffer to hash.
	@param	size				the size of the buffer.
	@return						hash value for the buffer. */
	size_t HashBuffer(char * buffer, const size_t & size);
	/** Increment a pointer's address by the offset provided.
	@param	ptr					the pointer to increment by the offset amount.
	@param	offset				the offset amount to apply to the pointer's address.
	@return						the modified pointer address. */
	void * PTR_ADD(void *const ptr, const size_t & offset);
};

#endif // BUFFER_TOOLS_H