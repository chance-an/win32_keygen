#include "stdafx.h"
#include "../stdafx.h"

#include <chrono>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "util.h"


namespace DevUtil{
	// Randomness generator with a seed.
	boost::random::mt19937 gen(getTimestamp() & 0xFFFFFFFF);

	void dumpCharArray(char data[], size_t size){
		char output[256];
		int index = 0, numberOfSpace = 255;

		char * output_ptr = output;
		char tmp[10];

		for (UINT i = 0; i < size; i++) {
			sprintf_s(tmp, "%02x ", data[i] & 0xFF);
			size_t byteLength = strlen(tmp);

			strcpy_s(output_ptr, numberOfSpace, tmp);
			output_ptr += byteLength;
			numberOfSpace -= byteLength;

			if (numberOfSpace <= 0) {
				break;
			}
		}
		*output_ptr = 0;
		printf("%s\n", output);
	};

	unsigned __int64  getTimestamp(){
		auto now = std::chrono::steady_clock::now();
		auto duration = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		return millis;
	};

	int nextRandom(int max){
		boost::random::uniform_int_distribution<> dist(1, max);
		return dist(gen);
	};
}