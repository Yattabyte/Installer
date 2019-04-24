#pragma once
#ifndef DIRECTORY_TOOLS_H
#define DIRECTORY_TOOLS_H

#include <string>
#include <vector>


/** Namespace to keep directory-related operations grouped together. */
namespace DRT {
	/** Compresses all disk contents found within a source directory into an .npack - package formatted buffer.
	After compression, it applies a small header dictating packaged folders' name.
	------------------------------------------------------
	| Directory name header | compressed directory data  |
	------------------------------------------------------
	@note						caller is responsible for cleaning-up packBuffer.
	@param	srcDirectory		the absolute path to the directory to compress.
	@param	packBuffer			pointer to the destination buffer, which will hold compressed contents.
	@param	packSize			reference updated with the size in bytes of the compressed packBuffer.
	@param	byteCount			(optional) pointer updated with the number of bytes written into the package
	@param	fileCount			(optional) pointer updated with the number of files written into the package.
	@param	exclusions			(optional) list of filenames/types to skip. "string" match relative path, ".ext" match extension.
	@return						true if compression success, false otherwise. */
	bool CompressDirectory(const std::string & srcDirectory, char ** packBuffer, size_t & packSize, size_t * byteCount = nullptr, size_t * fileCount = nullptr, const std::vector<std::string> & exclusions = std::vector<std::string>());
	/** Decompresses an .npack - package formatted buffer into its component files in the destination directory.
	@param	dstDirectory		the absolute path to the directory to decompress.
	@param	packBuffer			the buffer containing the compressed package contents.
	@param	packSize			the size of the buffer in bytes.
	@param	byteCount			(optional) pointer updated with the number of bytes written to disk.
	@param	fileCount			(optional) pointer updated with the number of files written to disk.
	@return						true if decompression success, false otherwise. */
	bool DecompressDirectory(const std::string & dstDirectory, char * packBuffer, const size_t & packSize, size_t * byteCount = nullptr, size_t * fileCount = nullptr);
	/** Processes two input directories and generates a compressed instruction set for transforming the old directory into the new directory.
	@note						caller is responsible for cleaning-up diffBuffer.
	@param	oldDirectory		the older directory or path to an .npack file.  
	@param	newDirectory		the newer directory or path to an .npack file.  
	@param	diffBuffer			pointer to the diff buffer, which will hold compressed diff instructions.
	@param	diffSize			reference updated with the size in bytes of the diff buffer.
	@param	instructionCount	(optional) pointer updated with the number of instructions compressed into the diff buffer.
	@return						true if diff success, false otherwise. */
	bool DiffDirectories(const std::string & oldDirectory, const std::string & newDirectory, char ** diffBuffer, size_t & diffSize, size_t * instructionCount = nullptr);
	/** Decompresses and executes the instructions contained within a previously - generated diff buffer.
	Transforms the contents of an 'old' directory into that of the 'new' directory.
	@param	dstDirectory		the destination directory to transform.
	@param	diffBuffer			the buffer containing the compressed diff instructions.
	@param	diffSize			the size in bytes of the compressed diff buffer.
	@param	bytesWritten		(optional) pointer updated with the number of bytes written to disk.
	@param	instructionsUsed	(optional) pointer updated with the number of instructions executed.
	@return						true if patch success, false otherwise. */
	bool PatchDirectory(const std::string & dstDirectory, char * diffBuffer, const size_t & diffSize, size_t * bytesWritten = nullptr, size_t * instructionsUsed = nullptr);
};

#endif // DIRECTORY_TOOLS_H