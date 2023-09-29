// ReSharper disable CommentTypo
#include <format>
#include <iostream>
#include <vector>
#include <kfr/all.hpp>

// ReSharper disable IdentifierTypo

int main()
{
	using namespace kfr;

	auto src_file = open_file_for_reading(KFR_FILEPATH("C:\\temp\\white-noise.wav"));
	auto tgt_file = open_file_for_writing(KFR_FILEPATH("C:\\temp\\white-noise-filter.wav"));

	auto wav_reader = audio_reader_wav<double>(src_file);

	auto src_format = wav_reader.format();

	auto src_rate = src_format.samplerate;
	auto tgt_rate = static_cast<cometa::fmax>(22050);

	auto tgt_format = src_format;

	tgt_format.samplerate = tgt_rate;

	auto wav_writer = audio_writer_wav<double>(tgt_file, tgt_format);

	auto buffer_length = static_cast<size_t>(1024);

	auto blocks = static_cast<size_t>(
		std::ceil(static_cast<double>(src_format.length) / static_cast<double>(buffer_length)));

	auto channels = src_format.channels;

	auto resamplers = std::vector(
		channels,
		resampler<double>(
			sample_rate_conversion_quality::high,
			static_cast<size_t>(tgt_rate),
			static_cast<size_t>(src_rate))
	);

	// TODO pre-buffer resamplers to avoid silence at beginning

	auto src_size = buffer_length;
	auto src_data = univector2d<double>(channels, univector<double>(src_size));
	auto src_temp = univector<double>(src_size * channels);

	auto& resampler1 = resamplers[0];

	auto tgt_size = resampler1.output_size_for_input(static_cast<itype>(src_size));
	auto tgt_data = univector2d<double>(channels, univector<double>(tgt_size));
	auto tgt_temp = univector<double>(tgt_size * channels);

	for (size_t block = 0; block < blocks; ++block)
	{
		auto read = wav_reader.read(src_temp);

		deinterleave(src_data, src_temp);

		std::cout << std::format("block {} of {}, read: {}\n", block + 1, blocks, read);

		for (size_t channel = 0; channel < channels; ++channel)
		{
			auto& resampler = resamplers[channel];

			auto process = resampler.process(tgt_data[channel], src_data[channel]);

			std::cout << std::format("channel: {}, process: {}\n", channel, process);
		}

		interleave(tgt_temp, tgt_data);

		wav_writer.write(tgt_temp); // TODO last chunk may be smaller
	}
}
