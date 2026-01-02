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

#ifndef STW_DATE_TIME_HPP
#define STW_DATE_TIME_HPP

#include <chrono>
#include <string>
#include <cstdint>

namespace stw
{
    using time_stamp = std::chrono::system_clock::time_point;

    class date_time
    {
    public:
        date_time();
        date_time(const time_stamp& timestamp);
        time_stamp get_date() const;
        int32_t get_day() const;
        int32_t get_day_of_week() const;
        int32_t get_day_of_year() const;
        int32_t get_hour() const ;
        int32_t get_minute() const;
        int32_t get_month() const;
        date_time add_milliseconds(int64_t milliseconds);
        date_time add_seconds(int64_t seconds);
        date_time add_minutes(int64_t minutes);
        date_time add_hours(int64_t hours);
        date_time add_days(int64_t days);
        static date_time get_now();
		static int64_t get_epoch();
        int32_t get_second() const;
        int32_t get_year() const;
        int64_t get_time_since_epoch_in_seconds() const;
        int64_t get_time_since_epoch_in_milliseconds() const;
        std::string get_formatted_timestamp() const;
        bool operator==(const date_time& other) const;
        bool operator!=(const date_time& other) const;
        bool operator>(const date_time& other) const;
        bool operator>=(const date_time& other) const;
        bool operator<(const date_time& other) const;
        bool operator<=(const date_time& other) const;
    private:
        time_stamp timestamp;
    };
}

#endif