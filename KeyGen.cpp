#include "stdafx.h"

#include <boost/crc.hpp>  // for boost::crc_32_type
#include <boost/multiprecision/cpp_int.hpp>

// #include <boost/multiprecision/detail/functions/pow.hpp>
#include "KeyGen.h"
#include "dev/util.h"

// #include "3rd_party/crc/crc.h"

#define USAGE_DAYS 0 //0 is unlimited

namespace KeyGen{

	_int16 getCRC(TCHAR name[], size_t nameSize, BYTE bytes[12], int seed);
	size_t encodeGroups(const boost::multiprecision::cpp_int& cpp_int_, char * output, size_t size);
	size_t encodeGroup(const unsigned _int64 int_, char * output, size_t size);

	template<size_t size>
	void bigEndianByteArrayToInt(boost::multiprecision::cpp_int* cpp_int_, BYTE const (&array)[size]){
		boost::multiprecision::cpp_int& ref_int = *cpp_int_;
		boost::multiprecision::cpp_int pow = 1;
		ref_int = 0;

		int digit = size;
		while (digit > 0){
			digit--;
			BYTE cDigit = array[digit];
			ref_int += (pow * cDigit);
			pow *= 256;
		}
	};

	_int16 getCRC(TCHAR name[], size_t nameSize, BYTE bytes[12], int seed){
		boost::crc_32_type  crc;

		// This converts TCHAR* to char*, regard code page translation for now.
		TCHAR * tcharEnd = name + nameSize;
		TCHAR * tcharPtr = name;
		char multiCharBuffer [10];
		char char_;
		char defaultCharValue = '?';

		// CRC name		
		while (tcharPtr != tcharEnd){
			int outputsize = 0;
			#ifdef UNICODE
				outputsize = WideCharToMultiByte(CP_UTF8, 0, tcharPtr, 1, multiCharBuffer, 10, NULL, NULL);
				multiCharBuffer[outputsize] = 0;
				char_ = *multiCharBuffer;
			#else
				char_ = (char)*tcharPtr;
			#endif	

			if (char_ < 32 || char_ > 176) { // need from ' ' to '~'
				tcharPtr++;
				continue;
			}
			
			crc.process_byte(char_);

			tcharPtr++;
		}

		// CRC 
		crc.process_byte(seed & 0xFF);
		crc.process_byte((seed >> 8) & 0xFF);
		crc.process_byte((seed >> 16) & 0xFF);
		crc.process_byte((seed >> 24) & 0xFF);

		for (int k = 0; k < 12 - 2; k++)
		{
			crc.process_byte(bytes[k]);
		}

		return crc.checksum() & 0xFFFF;
	}

	size_t encodeGroups(const boost::multiprecision::cpp_int& cpp_int_, char * output, size_t size){
		boost::multiprecision::cpp_int int_(cpp_int_);
		const _int64 value = 0x39aa400L;

		size_t wroteLength = 0;
		while (int_.compare(0) != 0){
			int mod_ = integer_modulus(int_, value);			
			int wrote = encodeGroup(mod_, (output + wroteLength), (size - wroteLength));
			if (wrote < 0){ //out of space
				return -1;
			}
			wroteLength += wrote;
			
			if (size > wroteLength) {
				output[wroteLength++] = '-';
			}
			int_ /= value;
		}
		if (output[wroteLength - 1] == '-'){ // get rid of the last '-'
			wroteLength--;
		}
		return wroteLength;
	};

	size_t encodeGroup(const unsigned _int64 int_, char * output, size_t size){
		unsigned _int64 int_cpy = int_;
		size_t wroteLength = 0;
		for (int j = 0; j < 5; j++){
			if (wroteLength >= size) {
				return -1;
			}

			int digitValue = int_cpy % 36; //convert to 0-9A-Z
			char c;
			if (digitValue < 10)
			{
				c = (char)(48 + digitValue);
			}
			else
			{
				c = (char)((65 + digitValue) - 10);
			}
			output[wroteLength++] = c;
			int_cpy /= 36;
		}
		return wroteLength;
	};

	size_t generate(TCHAR input[], size_t inputLength, TCHAR output[], size_t outputLength){
		std::cout << "generate() with: " << input << std::endl;

		BYTE bytes[12];
		int days = USAGE_DAYS;
		int seed = DevUtil::nextRandom(100000);
		seed %= 100000;
		__int64 time = DevUtil::getTimestamp();

		bytes[0] = (BYTE)1; // Product type
		bytes[1] = 13; // version
		__int64 ld = (time >> 16);
		bytes[2] = (BYTE)(ld & 255);
		bytes[3] = (BYTE)((ld >> 8) & 255);
		bytes[4] = (BYTE)((ld >> 16) & 255);
		bytes[5] = (BYTE)((ld >> 24) & 255);
		days &= 0xffff;
		bytes[6] = (BYTE)(days & 255);
		bytes[7] = (BYTE)((days >> 8) & 255);
		bytes[8] = 105;
		bytes[9] = -59;
		bytes[10] = 0;
		bytes[11] = 0;
		WORD crc = getCRC(input, inputLength, bytes, seed);
		bytes[10] = crc & 0xFF;
		bytes[11] = ((crc >> 8) & 0xFF);

		boost::multiprecision::cpp_int pow("891262723010319840325551252027888001981");
		boost::multiprecision::cpp_int mod("0x86f71688cdd2612c9b7d1f54bdae029");
		boost::multiprecision::cpp_int base(0);
		bigEndianByteArrayToInt(&base, bytes);

		boost::multiprecision::cpp_int result = 0;
		result = powm(base, pow, mod);

		// printf("test:%s\n", result);
#define STRING_SIZE 100
		char sn[STRING_SIZE+1];
		size_t string_size = 0;
		string_size += _snprintf_s(sn, STRING_SIZE, STRING_SIZE, "%05d", seed);
		sn[string_size++] = '-';

		string_size+=encodeGroups(result, (sn + string_size), (STRING_SIZE - string_size));

		sn[string_size] = 0;

		std::cout << "KeyCode generate successfully: " << sn << std::endl;

		int result_ = MultiByteToWideChar(CP_UTF8, 0, sn, string_size, output, outputLength);
		if (result_ > 0) { 
			output[result_] = 0;
			return 0; // success
		}
		else{
			// return required size
			return MultiByteToWideChar(CP_UTF8, 0, sn, string_size, output, 0);
		}
	};
}