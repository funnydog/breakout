#pragma once

#include <cassert>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <unordered_map>

template <typename Resource, typename Identifier>
class ResourceHolder
{
public:
	typedef std::unique_ptr<Resource> ResourcePtr;

public:
	template<typename... Args>
	void load(Identifier id, const std::filesystem::path &path, Args&&... args);

	void add(Identifier id, ResourcePtr resource);
	Resource& get(Identifier id);
	const Resource& get(Identifier id) const;

private:
	std::unordered_map<Identifier, ResourcePtr> mResourceMap;
};

template <typename Resource, typename Identifier>
template <typename... Args>
void
ResourceHolder<Resource, Identifier>::load(
	Identifier id,
	const std::filesystem::path &path,
	Args&& ...args)
{
	auto pRes = std::make_unique<Resource>();
	if (!pRes->loadFromFile(path, std::forward<Args>(args)...))
	{
		throw std::runtime_error("ResourceHolder::load(): "
		                         "Failed to load " + path.string());
	}
	add(id, std::move(pRes));
}

template <typename Resource, typename Identifier>
void
ResourceHolder<Resource, Identifier>::add(Identifier id, ResourcePtr ptr)
{
	auto [_, added] = mResourceMap.insert(std::make_pair(id, std::move(ptr)));
	if (!added)
	{
		throw std::runtime_error("ResourceHolder::add() failed");
	}
}

template <typename Resource, typename Identifier>
Resource&
ResourceHolder<Resource, Identifier>::get(Identifier id)
{
	return const_cast<Resource&>(static_cast<const ResourceHolder&>(*this).get(id));
}

template <typename Resource, typename Identifier>
const Resource&
ResourceHolder<Resource, Identifier>::get(Identifier id) const
{
	auto found = mResourceMap.find(id);
	assert(found != mResourceMap.end() && "Resource not found");
	return *found->second;
}
