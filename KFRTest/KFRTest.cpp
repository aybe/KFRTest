#include <format>
#include <iostream>
#include <vector>
#include <kfr/all.hpp>

// ReSharper disable IdentifierTypo

int main()
{
	using namespace kfr;

	auto source_file = open_file_for_reading(KFR_FILEPATH("C:\\temp\\messij.wav"));
	auto target_file = open_file_for_writing(KFR_FILEPATH("C:\\temp\\messij-reverb.wav"));

	auto source_reader = audio_reader_wav<double>(source_file);

	auto source_format = source_reader.format();

	auto source_rate = source_format.samplerate;
	auto target_rate = source_rate; // TODO

	auto target_format = source_format;

	target_format.samplerate = target_rate;

	auto target_writer = audio_writer_wav<double>(target_file, target_format);

	auto length = source_format.length;

	auto block_size = static_cast<size_t>(1024);
	auto blocks = static_cast<size_t>(std::ceil(static_cast<double>(length) / static_cast<double>(block_size)));

	auto channels = source_format.channels;

	auto resamplers = std::vector(
		channels,
		resampler<double>(sample_rate_conversion_quality::high, static_cast<size_t>(target_rate), static_cast<size_t>(source_rate))
	);

	auto source_length = block_size * channels;
	auto target_length = block_size * static_cast<size_t>(target_rate) / static_cast<size_t>(source_rate) * channels;
	auto source_buffer = univector<double>(source_length);
	auto target_buffer = univector<double>(target_length);
	auto sample_buffer = univector2d<double, 2, 4>();

	for (auto block = 0u; block < blocks; ++block)
	{
		size_t r_size = source_reader.read(source_buffer);
		size_t w_size = target_writer.write(source_buffer.data(), r_size);
		std::cout << std::format("Block {} of {}\n", block + 1, blocks);
	}
}
