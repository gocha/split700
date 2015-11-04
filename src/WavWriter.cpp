
#include <stdint.h>

#include <string>
#include <vector>
#include <iterator>

#include "WavWriter.h"

static void write16(std::vector<uint8_t> & data, uint16_t value)
{
	data.push_back(value & 0xff);
	data.push_back((value >> 8) & 0xff);
}

static void write32(std::vector<uint8_t> & data, uint32_t value)
{
	data.push_back(value & 0xff);
	data.push_back((value >> 8) & 0xff);
	data.push_back((value >> 16) & 0xff);
	data.push_back((value >> 24) & 0xff);
}

static void write(std::vector<uint8_t> & data, const char * value, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		data.push_back(value[i]);
	}
}

WavWriter::WavWriter() :
	channels(1),
	samplerate(44100),
	bitwidth(16),
	loop_sample(0),
	looped(false)
{
}

WavWriter::WavWriter(std::vector<int16_t> samples) :
	channels(1),
	samplerate(44100),
	bitwidth(16),
	loop_sample(0),
	looped(false)
{
	this->samples.assign(samples.begin(), samples.end());
}

WavWriter::~WavWriter()
{
}

void WavWriter::AddSample(int16_t sample)
{
	samples.push_back(sample);
}

void WavWriter::AddSample(std::vector<int16_t> samples)
{
	std::copy(samples.begin(), samples.end(), std::back_inserter(this->samples));
}

bool WavWriter::WriteFile(const std::string & filename)
{
	int16_t bytes_per_sample = bitwidth / 8;
	if (bitwidth != 16) {
		m_message = "Unsupported bitwidth";
		return false;
	}

	FILE * wav_file = fopen(filename.c_str(), "wb");
	if (wav_file == NULL) {
		m_message = "File open error";
		return false;
	}

	std::vector<uint8_t> header;
	header.reserve(36);
	write(header, "RIFF", 4);
	write32(header, 0); // 0x04: put actual file size later
	write(header, "WAVE", 4);
	write(header, "fmt ", 4);
	write32(header, 16);
	write16(header, 1);
	write16(header, channels);
	write32(header, samplerate);
	write32(header, samplerate * bytes_per_sample * channels);
	write16(header, bytes_per_sample * channels);
	write16(header, bitwidth);

	std::vector<uint8_t> data_chunk;
	data_chunk.reserve(8 + samples.size() * 2);
	write(data_chunk, "data", 4);
	write32(data_chunk, (uint32_t)(samples.size() * 2));
	for (auto itr_sample = samples.begin(); itr_sample != samples.end(); ++itr_sample) {
		int16_t sample = *itr_sample;
		write16(data_chunk, sample);
	}

	std::vector<uint8_t> smpl_chunk;
	// add loop point info (smpl chunk) if needed... details:
	// en: http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/wave.htm
	// ja: http://co-coa.sakura.ne.jp/index.php?Sound%20Programming%2FWave%20File%20Format
	if (looped) {
		smpl_chunk.reserve(8 + 60);
		write(smpl_chunk, "smpl", 4);
		write32(smpl_chunk, 60); // chunk size
		write32(smpl_chunk, 0);  // manufacturer
		write32(smpl_chunk, 0);  // product
		write32(smpl_chunk, 1000000000 / samplerate); // sample period
		write32(smpl_chunk, 60); // MIDI uniti note (C5)
		write32(smpl_chunk, 0);  // MIDI pitch fraction
		write32(smpl_chunk, 0);  // SMPTE format
		write32(smpl_chunk, 0);  // SMPTE offset
		write32(smpl_chunk, 1);  // sample loops
		write32(smpl_chunk, 0);  // sampler data
		write32(smpl_chunk, 0);  // cue point ID
		write32(smpl_chunk, 0);  // type (loop forward)
		write32(smpl_chunk, loop_sample); // start sample #
		write32(smpl_chunk, (uint32_t)samples.size() / channels); // end sample #
		write32(smpl_chunk, 0);  // fraction
		write32(smpl_chunk, 0);  // playcount
	}

	uint32_t whole_size = (uint32_t)(header.size() + data_chunk.size() + smpl_chunk.size() - 8);
	header[4] = whole_size & 0xff;
	header[5] = (whole_size >> 8) & 0xff;
	header[6] = (whole_size >> 16) & 0xff;
	header[7] = (whole_size >> 24) & 0xff;

	if (fwrite(&header[0], header.size(), 1, wav_file) != 1) {
		m_message = "File write error";
		fclose(wav_file);
		return false;
	}

	if (fwrite(&data_chunk[0], data_chunk.size(), 1, wav_file) != 1) {
		m_message = "File write error";
		fclose(wav_file);
		return false;
	}

	if (smpl_chunk.size() != 0) {
		if (fwrite(&smpl_chunk[0], smpl_chunk.size(), 1, wav_file) != 1) {
			m_message = "File write error";
			fclose(wav_file);
			return false;
		}
	}

	fclose(wav_file);
	return true;
}
