#include <format>
#include <iostream>
#include <kfr/all.hpp>

// ReSharper disable CppInconsistentNaming
// ReSharper disable IdentifierTypo

int main()
{
	using cometa::fbase;
	using namespace kfr;

	auto source = open_file_for_reading("C:\\temp\\white-noise.wav");
	auto target = open_file_for_writing("C:\\temp\\white-noise-filter.wav");

	auto reader = kfr::audio_reader_wav<fbase>(source);

	auto& format = reader.format();

	auto writer = kfr::audio_writer_wav<fbase>(target, format);

	const auto channels = format.channels;

	univector<fbase, 461> taps;
	constexpr double cutoff = 0.25;
	const auto window = to_handle(window_blackman_harris(taps.size()));
	fir_lowpass(taps, cutoff, window, true);
	auto filters = std::vector(channels, kfr::filter_fir<fbase>(taps));

	constexpr auto block_size = static_cast<size_t>(1024);
	auto blocks = static_cast<size_t>(std::ceil(static_cast<double>(format.length) / block_size));

	for (auto block = 0u; block < blocks; ++block)
	{
		auto data = reader.read_channels(block_size);

		for (auto channel = 0u; channel < channels; ++channel)
		{
			auto& filter = filters[channel];
			auto& vector = data[channel];
			filter.apply(vector);
		}

		writer.write_channels(data);

		std::cout << std::format("Processed block {} of {}\n", block + 1, blocks);
	}
}
