#include "stw_file_cache.hpp"
#include "stw_file.hpp"
#include <filesystem>

namespace stw
{
    file_cache::file_cache()
    {
        set_max_age(10 * 60);
    }

    bool file_cache::read_file(const std::string &filePath, uint8_t **pData, uint64_t *size)
    {
        invalidate();

        if(!stw::file::exists(filePath))
            return false;
        
        auto getLastModifiedTime = [&] () -> uint64_t {
            std::filesystem::path p(filePath);
            auto lastWriteTime = std::filesystem::last_write_time(p);
            auto sctp = decltype(lastWriteTime)::clock::to_sys(lastWriteTime);
            auto duration = sctp.time_since_epoch();
            uint64_t lastModified = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return lastModified;
        };

        auto getLastAccessedTime = [] () -> uint64_t {
            auto ts = std::chrono::system_clock::now();
            uint64_t lastAccessed = std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count();
            return lastAccessed;
        };

        uint64_t lastModified = getLastModifiedTime();
        uint64_t lastAccessed = getLastAccessedTime();

        if(!files.contains(filePath))
        {
            files[filePath] = {
                .path = filePath,
                .data = stw::file::read_all_bytes(filePath),
                .lastModified = lastModified,
                .lastAccessed = lastAccessed
            };

            auto &file = files[filePath];
            *pData = file.data.data();
            *size = file.data.size();
            return true;
        }
        else
        {
            auto &file = files[filePath];

            // Update the file if it has been changed            
            if(file.lastModified != lastModified)
            {
                file.data = stw::file::read_all_bytes(filePath);
                file.lastModified = lastModified;
            }

            file.lastAccessed = lastAccessed;

            *pData = file.data.data();
            *size = file.data.size();
            return true;
        }
    }

    void file_cache::clear()
    {
        files.clear();
    }

    void file_cache::invalidate()
    {
        if(files.size() == 0)
            return;
        
        auto ts = std::chrono::system_clock::now();
        uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count();

        for (auto it = files.begin(); it != files.end();)
        {
            if (currentTime - it->second.lastAccessed > maxAge)
                it = files.erase(it);
            else
                ++it;
        }
    }

    void file_cache::set_max_age(uint64_t maxAgeInSeconds)
    {
        if(maxAgeInSeconds < 1)
            maxAgeInSeconds = 1;
        this->maxAge = maxAgeInSeconds * 1000;
    }

    uint64_t file_cache::get_max_age() const
    {
        return maxAge / 1000;
    }
}