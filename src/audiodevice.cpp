#include <fstream>
#include <iostream>

#include "alcheck.hpp"
#include "audiodevice.hpp"

namespace
{
struct WaveMasterChunk
{
	char          chunkId[4];
	std::uint32_t chunkSize;
	char          waveId[4];
};
struct WaveChunkHeader
{
	char          chunkId[4];
	std::uint32_t chunkSize;
};
struct WaveFormatChunk
{
	std::uint16_t wFormatTag;
	std::uint16_t nChannels;
	std::uint32_t nSamplesPerSec;
	std::uint32_t nAvgBytesPerSec;
	std::uint16_t bBlockAlign;
	std::uint16_t nBitsPerSample;
	std::uint16_t cbSize;
	std::uint16_t wValidBitsPerSample;
	std::uint32_t dwChannelMask;
	char          SubFormat[16];
};
}

std::vector<std::string>
AudioDevice::enumerate()
{
	std::vector<std::string> devices;
	const char *stringList = nullptr;
	if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_EXT"))
	{
		stringList = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
	}
	else if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT"))
	{
		stringList = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
	}
	else
	{
		stringList = "";
	}

	while (stringList[0])
	{
		const auto &deviceName = devices.emplace_back(stringList);
		stringList += deviceName.size() + 1;
	}
	return devices;
}

AudioDevice::AudioDevice()
	: mAudioDevice(nullptr)
	, mAudioContext(nullptr)
	, mMasterVolume(0.f)
{
}

AudioDevice::~AudioDevice()
{
	for (auto source : mPlayingSources)
	{
		alCheck(alSourceStop(source));
	}
	alCheck(alDeleteSources(mPlayingSources.size(), mPlayingSources.data()));
	alCheck(alDeleteSources(mStoppedSources.size(), mStoppedSources.data()));
	for (auto [_, buffer] : mBuffers)
	{
		alCheck(alDeleteBuffers(1, &buffer));
	}
}

bool
AudioDevice::open(const std::string &name)
{
	const char *deviceName = nullptr;
	if (!name.empty())
	{
		deviceName = name.c_str();
	}

	close();
	auto device = alcOpenDevice(deviceName);
	if (!device)
	{
		std::cerr << "AudioDevice::open(\"" << name
		          << "\") - cannot open the device.\n";
		return false;
	}

	auto context = alcCreateContext(device, nullptr);
	if (!context)
	{
		std::cerr << "AudioDevice::open(\"" << name
		          << "\") - cannot create a context.\n";
		alcCloseDevice(device);
		return false;
	}

	mAudioDevice = device;
	mAudioContext = context;
	alcMakeContextCurrent(context);

	// apply the set values
	alCheck(alListenerf(AL_GAIN, mMasterVolume * 0.01f));
	return true;
}

void
AudioDevice::close()
{
	alcMakeContextCurrent(nullptr);
	if (mAudioContext)
	{
		alcDestroyContext(mAudioContext);
		mAudioContext = nullptr;
	}
	if (mAudioDevice)
	{
		alcCloseDevice(mAudioDevice);
		mAudioDevice = nullptr;
	}
}

float
AudioDevice::getMasterVolume() const
{
	return mMasterVolume * 100.f;
}

void
AudioDevice::setMasterVolume(float value)
{
	if (value < 0.f)
	{
		std::cerr << "AudioDevice::setMasterVolume() called with negative value\n";
		value = 0.f;
	}
	if (value > 100.f)
	{
		std::cerr << "AudioDevice::setMasterVolume() called with value > 100\n";
		value = 100.f;
	}
	mMasterVolume = value * 0.01f;
	if (mAudioDevice != nullptr && mAudioContext != nullptr)
	{
		alCheck(alListenerf(AL_GAIN, mMasterVolume));
	}
}

void
AudioDevice::play(SoundID soundId)
{
        // find the sound id
	auto found = mBuffers.find(soundId);
	if (found == mBuffers.end())
	{
		std::cerr << "AudioDevice::play(" << static_cast<int>(soundId)
		          << ") - sound id not found\n";
		return;
	}

	// get an available source
	unsigned source;
	if (!mStoppedSources.empty())
	{
		source = mStoppedSources.back();
		mStoppedSources.pop_back();
	}
	else
	{
		alCheck(alGenSources(1, &source));
	}

	// bind the buffer
	alCheck(alSourcei(source, AL_BUFFER, found->second));
	alCheck(alSourcePlay(source));
	mPlayingSources.push_back(source);
}

void
AudioDevice::update()
{
	auto i = mPlayingSources.size();
	while (i-- > 0)
	{
		int status;
		alCheck(alGetSourcei(mPlayingSources[i], AL_SOURCE_STATE, &status));
		if (status == AL_STOPPED)
		{
			mStoppedSources.push_back(mPlayingSources[i]);
			mPlayingSources[i] = mPlayingSources.back();
			mPlayingSources.pop_back();
		}
	}
}

static unsigned
loadWav(const std::filesystem::path &path)
{
	std::ifstream file(path, std::ios::in|std::ios::binary);
	if (!file)
	{
		return 0;
	}

	WaveMasterChunk master;
	if (!file.read(reinterpret_cast<char*>(&master), sizeof(master))
	    || !std::equal(master.chunkId, master.chunkId+4, "RIFF")
	    || !std::equal(master.waveId, master.waveId+4, "WAVE"))
	{
		return 0;
	}

	// read the chunks we are interested in
	unsigned sampleFormat = 0;
	unsigned sampleRate = 0;
	WaveChunkHeader header;
	while (file.read(reinterpret_cast<char *>(&header), sizeof(header)))
	{
		if (std::equal(header.chunkId+0, header.chunkId+4, "fmt "))
		{
			WaveFormatChunk fmt;
			if (!file.read(reinterpret_cast<char *>(&fmt), header.chunkSize))
			{
				return 0;
			}
			if (fmt.wFormatTag != 1)
			{
				return 0;
			}

			// decode the format of the samples
			switch (fmt.nChannels)
			{
			case 1: sampleFormat = AL_FORMAT_MONO16; break;
			case 2: sampleFormat = AL_FORMAT_STEREO16; break;
			case 4: sampleFormat = alGetEnumValue("AL_FORMAT_QUAD16"); break;
			case 6: sampleFormat = alGetEnumValue("AL_FORMAT_51CHN16"); break;
			case 7: sampleFormat = alGetEnumValue("AL_FORMAT_71CHN16"); break;
			default: break;
			}
			if (sampleFormat == -1U)
			{
				sampleFormat = 0;
			}
			// copy the sample rate
			sampleRate = fmt.nSamplesPerSec;
		}
		else if (std::equal(header.chunkId, header.chunkId+4, "data"))
		{
			// data of size header.chunkSize
			std::vector<char> samples(header.chunkSize);
			if (!file.read(samples.data(), samples.size()))
			{
				return 0;
			}
			unsigned buffer;
			alCheck(alGenBuffers(1, &buffer));
			alBufferData(buffer, sampleFormat,
			             samples.data(), samples.size(),
			             sampleRate);
			if (alGetError())
			{
				alDeleteBuffers(1, &buffer);
				return 0;
			}
			return buffer;
		}
		else
		{
                        // skip the other chunks
			file.seekg(header.chunkSize, std::ios::cur);
		}
	}
	return 0;
}

bool
AudioDevice::load(SoundID id, const std::filesystem::path &path)
{
	unsigned alBuffer = loadWav(path);
	if (!alBuffer)
	{
		std::cerr << "AudioDevice::load() - failed to load \""
		          << path << "\".\n";
		return false;
	}

	auto [it, added] = mBuffers.insert(std::make_pair(id, alBuffer));
	if (!added)
	{
		std::cerr << "AudioDevice::load() - failed to add \""
		          << path << "\".\n";
		return false;
	}

	return true;
}
