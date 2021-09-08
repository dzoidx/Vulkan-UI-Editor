#pragma once
#include "String.h"
#include "Types.h"

/// Ticks with this resolution is enough to store 10000 years in 64bit
constexpr uint64 NanosInTick{ 100 };
constexpr uint64 NanosInMillisecond{ 1000000 };
constexpr uint64 TicksPerMillisecond{ 10000 };
constexpr uint64 TicksPerSecond{ TicksPerMillisecond * 1000 };			// 10,000,000
constexpr uint64 TicksPerMinute{ TicksPerSecond * 60 };					// 600,000,000
constexpr uint64 TicksPerHour{ TicksPerMinute * 60 };					// 36,000,000,000
constexpr uint64 TicksPerDay{ TicksPerHour * 24 };						// 864,000,000,000
constexpr uint64 NanosInSec{ 1000000000 };
constexpr uint64 MicrosInSec{ 1000000 };
constexpr uint64 MillisPerSecond{ 1000 };
constexpr uint64 MillisPerMinute{ MillisPerSecond * 60 };				//     60,000
constexpr uint64 MillisPerHour{ MillisPerMinute * 60 };					//  3,600,000
constexpr uint64 MillisPerDay{ MillisPerHour * 24 };					// 86,400,000
constexpr uint64 DaysPerYear{ 365 };
constexpr uint64 DaysInLeapYear{ 366 };
constexpr uint64 DaysPer4Years{ DaysPerYear * 3 + DaysInLeapYear };		// 1461
constexpr uint64 DaysPer100Years{ DaysPer4Years * 25 - 1 };				// 36524
constexpr uint64 DaysPer400Years{ DaysPer100Years * 4 + 1 };			// 146097
// Number of days from 0001.01.01 to 1600.12.31
constexpr uint64 DaysTo1601{ DaysPer400Years * 4 };						// 584388
// Number of days from 0001.01.01 to 9999.12.31
constexpr uint64 DaysTo1899{ DaysPer400Years * 4 + DaysPer100Years * 3 - 367 };
constexpr uint64 DaysTo1970{ DaysPer400Years * 4 + DaysPer100Years * 3 + DaysPer4Years * 17 + DaysPerYear }; // 719,162
constexpr uint64 DaysTo10000{ DaysPer400Years * 25 - DaysInLeapYear };	// 3652059
constexpr uint64 TicksTo1970{ DaysTo1970 * TicksPerDay };
constexpr uint64 MinTicks{ 0 };
constexpr uint64 MaxTicks{ DaysTo10000 * TicksPerDay - 1 };				// 3155378975999999999
constexpr uint64 MaxMilliseconds{ DaysTo10000 * MillisPerDay };			// 315537897600000

constexpr double MillisecondsPerTick{ 1.0 / TicksPerMillisecond };
constexpr double SecondsPerTick{ 1.0 / TicksPerSecond };				// 0.0001
constexpr double MinutesPerTick{ 1.0 / TicksPerMinute };				// 1.6666666666667e-9
constexpr double HoursPerTick{ 1.0 / TicksPerHour };					// 2.77777777777777778e-11
constexpr double DaysPerTick{ 1.0 / TicksPerDay };						// 1.1574074074074074074e-12

constexpr uint64 MaxSeconds = 0xffffffffffffffffui64 / TicksPerSecond;
constexpr uint64 MinSeconds = 0 / TicksPerSecond;

constexpr uint64 MaxMilliSeconds = 0xffffffffffffffffui64 / TicksPerMillisecond;
constexpr uint64 MinMilliSeconds = 0 / TicksPerMillisecond;

constexpr long TicksPerTenthSecond = TicksPerMillisecond * 100;

enum class DateTimeKind {
	Unspecified,
	Utc,
	Local
};

enum class DayOfWeek
{
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
	Sunday,
};

enum class DatePart {
	Year,
	DayOfYear,
	Month,
	Day
};

constexpr const int DaysToMonth365[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
constexpr const int DaysToMonth366[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

constexpr const uint64 TicksMask = 0x3FFFFFFFFFFFFFFF;
constexpr const uint64 FlagsMask = 0xC000000000000000;
constexpr const uint64 KindUnspecified = 0x0000000000000000;
constexpr const uint64 KindUtc = 0x4000000000000000;
constexpr const uint64 KindLocal = 0x8000000000000000;

class DateTime {
public:
	DateTime(uint64 ticks, DateTimeKind kind);
	DateTime();

	DateTime AddMilliseconds(uint64 msecs) const;
	uint64 TotalMilliseconds() const;

	DateTimeKind Kind() const;
	DateTime Date();
	int32 Day();
	DayOfWeek DayOfWeek();
	int32 DayOfYear();
	uint64 Ticks();
	int32 Millisecond();
	int32 Second();
	int32 Minute();
	int32 Hour();
	int32 Month();
	int32 Year();
	bool IsLeapYear(int32 year);
	static DateTime Now();
	static DateTime UtcNow();
	static int64 TimeZoneOffset();
	DateTime ToLocal() const;
	DateTime ToUtc() const;

	String ToString();

	friend bool operator==(const DateTime& lhs, const DateTime& rhs) { return lhs.ExtractTicks() == rhs.ExtractTicks(); }
	friend bool operator!=(const DateTime& lhs, const DateTime& rhs) { return !(lhs == rhs); }
	friend bool operator >(const DateTime& lhs, const DateTime& rhs) { return lhs.ExtractTicks() > rhs.ExtractTicks(); }
	friend bool operator>=(const DateTime& lhs, const DateTime& rhs) { return lhs.ExtractTicks() >= rhs.ExtractTicks(); }
	friend bool operator <(const DateTime& lhs, const DateTime& rhs) { return lhs.ExtractTicks() < rhs.ExtractTicks(); }
	friend bool operator<=(const DateTime& lhs, const DateTime& rhs) { return lhs.ExtractTicks() <= rhs.ExtractTicks(); }

	const static DateTime MinValue;
	const static DateTime MaxValue;

private:
	DateTime(uint64 rawData);

	inline uint64 ExtractTicks() const;
	inline uint64 ExtractKind() const;
	inline int32 GetDatePart(DatePart part);

private:
	uint64 rawData_;
};