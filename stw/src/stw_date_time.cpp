// MIT License
// Copyright © 2025 W.M.R Jap-A-Joe

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "stw_date_time.hpp"
#include <iomanip>
#include <sstream>

namespace stw
{
	date_time::date_time()
	{
		timestamp = std::chrono::system_clock::now();
	}

	date_time::date_time(const time_stamp& timestamp)
	{
		this->timestamp = timestamp;
	}

	time_stamp date_time::get_date() const
	{
		return timestamp;
	}

	int32_t date_time::get_day() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_mday;
	}

	int32_t date_time::get_day_of_week() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_wday;
	}

	int32_t date_time::get_day_of_year() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_yday;
	}

	int32_t date_time::get_hour() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_hour;
	}

	int32_t date_time::get_minute() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_min;
	}

	int32_t date_time::get_month() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_mon;
	}

	date_time date_time::add_milliseconds(int64_t milliseconds)
	{
        timestamp += std::chrono::milliseconds(milliseconds);
        return *this;
	}

	date_time date_time::add_seconds(int64_t seconds)
	{
        timestamp += std::chrono::seconds(seconds);
        return *this;
	}

	date_time date_time::add_minutes(int64_t minutes)
	{
        timestamp += std::chrono::minutes(minutes);
        return *this;
	}

	date_time date_time::add_hours(int64_t hours)
	{
        timestamp += std::chrono::hours(hours);
        return *this;
	}

	date_time date_time::add_days(int64_t days)
	{
        timestamp += std::chrono::hours(days * 24);
        return *this;
	}

	date_time date_time::get_now()
	{
		return date_time(std::chrono::system_clock::now());
	}

	int32_t date_time::get_second() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_sec;
	}

	int32_t date_time::get_year() const
	{
        time_t tt = std::chrono::system_clock::to_time_t(timestamp);
        tm local_tm = *localtime(&tt);
        return local_tm.tm_year;
	}

	int64_t date_time::get_time_since_epoch_in_seconds() const
	{
        auto epoch = timestamp.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);
        return value.count();    
	}

	int64_t date_time::get_time_since_epoch_in_milliseconds() const
	{
        auto epoch = timestamp.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return value.count();        
	}

	std::string date_time::get_formatted_timestamp() const
	{
        auto now = std::chrono::system_clock::to_time_t(timestamp);
        std::tm tmNow = *std::localtime(&now);

        std::ostringstream oss;
        oss << std::put_time(&tmNow, "%m-%d-%Y %H:%M:%S");
        return oss.str();
	}

	bool date_time::operator==(const date_time& other) const
	{
		return timestamp == other.timestamp;
	}

	bool date_time::operator!=(const date_time& other) const
	{
		return timestamp != other.timestamp;
	}

	bool date_time::operator>(const date_time& other) const
	{
		return timestamp > other.timestamp;
	}

	bool date_time::operator>=(const date_time& other) const
	{
		return timestamp >= other.timestamp;
	}

	bool date_time::operator<(const date_time& other) const
	{
		return timestamp < other.timestamp;
	}

	bool date_time::operator<=(const date_time& other) const
	{
		return timestamp <= other.timestamp;
	}
}