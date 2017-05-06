#pragma once
#include <stdint.h>
#include <string>
enum FilesumAlm {
	kFilesumMD5,
	kFilesumSHA1,
	kFilesumSHA1DC,/// sha1 collision detection
	kFilesumSHA2,
	kFilesumSHA3
};

class Hashsum {
public:
	virtual void Initialize(int width)=0;
	virtual void Update(const unsigned char *buf, size_t len) = 0;
	virtual void Final(bool ucase,std::wstring &hash) = 0;
	virtual const wchar_t *Name() = 0;
private:
};


class Filesum {
public:
	Filesum(const Filesum &) = delete;
	Filesum &operator=(const Filesum &) = delete;
	Filesum() = default;
	~Filesum();
	bool Initialize(int alg, uint32_t width);
	void Update(const char *buf,size_t bufsize);
	void Final(bool ucase);
	const wchar_t *Name() { return sum->Name(); }
	const std::wstring &Hash()const { return hex; }
private:
	std::wstring hex;
	Hashsum *sum{nullptr};
};