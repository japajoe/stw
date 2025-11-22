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