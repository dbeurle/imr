/*
 * For licensing please refer to the LICENSE.md file
 */

#ifndef IO_GMSHREADEREXCEPTION_HPP_
#define IO_GMSHREADEREXCEPTION_HPP_

#include <exception>

class GmshReaderException: public std::exception
{
public:
	GmshReaderException() = default;
};

class GmshMeshFormatException: public GmshReaderException
{
public:
    GmshMeshFormatException(std::string partName, std::string fileExtension):
            partName(partName),
            fileExtension(fileExtension)
    {
    }
    const char* what() const _GLIBCXX_USE_NOEXCEPT;

protected:
    std::string partName;
    std::string fileExtension;
};

class GmshFileOpenException: public GmshReaderException
{
public:
	GmshFileOpenException(std::string gmshFile) :
			gmshFile(gmshFile)
	{
	}
	const char* what() const _GLIBCXX_USE_NOEXCEPT;

protected:
	std::string gmshFile;
};

class GmshFileParseException: public GmshReaderException
{
public:
	GmshFileParseException(std::string gmshFile) :
			gmshFile(gmshFile)
	{
	}

	const char* what() const _GLIBCXX_USE_NOEXCEPT;

protected:
	std::string gmshFile;
};

class GmshTagException: public GmshReaderException
{
public:
	GmshTagException() = default;

	const char* what() const _GLIBCXX_USE_NOEXCEPT;

};

class GmshElementCodeException: public GmshReaderException
{
public:
	GmshElementCodeException(const short& elementType) :
			elementType(elementType)
	{
	}

	const char* what() const _GLIBCXX_USE_NOEXCEPT;

protected:
	short elementType;
};

#endif /* IO_GMSHREADEREXCEPTION_HPP_ */
