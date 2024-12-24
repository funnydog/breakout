#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <al.h>
#include <alc.h>

#include "resources.hpp"

class AudioDevice
{
public:
	explicit AudioDevice();
	~AudioDevice();

	static std::vector<std::string> enumerate();

	bool open(const std::string &name);
	void close();

	void bind() const;

	float getMasterVolume() const;
	void setMasterVolume(float value);

	void play(SoundID id);
	void update();

	bool load(SoundID id, const std::filesystem::path &path);

private:
	ALCdevice *mAudioDevice;
	ALCcontext *mAudioContext;
	float mMasterVolume;
	std::vector<unsigned> mStoppedSources;
	std::vector<unsigned> mPlayingSources;
	std::unordered_map<SoundID, unsigned> mBuffers;
};
