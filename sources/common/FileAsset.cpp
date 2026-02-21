/*
** ETIB PROJECT, 2025
** evan
** File description:
** FileAsset
*/

#include "FileAsset.hpp"

evan::FileAsset::FileAsset(const std::string &content)
	: _content(content)
{
}

evan::FileAsset::~FileAsset()
{
}

size_t evan::FileAsset::write(const void *ptr, size_t size, size_t nmemb)
{
	size_t lenBefore = _content.size();
	size_t newLen	 = lenBefore + size * nmemb;
	if (newLen > _content.capacity()) {
		_content.reserve(newLen);
	}
	_content.insert(_pos, static_cast<const char *>(ptr), size * nmemb);
	_pos += size * nmemb;
	return (_content.size() - lenBefore) / size;
}

size_t evan::FileAsset::read(void *ptr, size_t size, size_t count)
{
	size_t toRead = size * count;
	if (_pos + toRead > _content.size())
		toRead = _content.size() - _pos;
	std::memcpy(ptr, _content.c_str() + _pos, toRead);
	_pos += toRead;
	return toRead / size;
}

size_t evan::FileAsset::read(std::string &str, size_t size, size_t count)
{
	return this->read(&str[0], size, count);
}

int evan::FileAsset::seek(long offset, Seek whence)
{
	switch (whence) {
		case FileAsset::Seek::SET:
			_pos = offset;
			break;
		case FileAsset::Seek::CUR:
			_pos += offset;
			break;
		case FileAsset::Seek::END:
			_pos = _content.size() + offset;
			break;
		default:
			return -1;
	}
	return 0;
}

size_t evan::FileAsset::tell() const
{
	return _pos;
}
