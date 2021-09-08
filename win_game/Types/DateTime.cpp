#if _WIN32
#include "Windows.h"
#endif
#include "DateTime.h"
#include <ctime>
#include "String.h"

const DateTime DateTime::MinValue(MinTicks, DateTimeKind::Unspecified);
const DateTime DateTime::MaxValue(MaxTicks, DateTimeKind::Unspecified);

inline uint64 TimespecToTicks(timespec& t)
{
	return t.tv_sec * TicksPerSecond + t.tv_nsec / NanosInTick;
}

inline uint64 GetUtcTimestamp() {
#if _WIN32
	// Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
	FILETIME ft;
	constexpr uint64 offset = (DaysTo1970 - DaysTo1601) * TicksPerDay;
	// COMPAT: Windows 8, Windows Server 2012
	//GetSystemTimePreciseAsFileTime(&ft); // ломают спидхаком, 45кк вызовов/сек, в 6.5 раз медленнее чем GetSystemTimeAsFileTime
	GetSystemTimeAsFileTime(&ft); // 291кк вызовов в сек
	uint64 ts = ft.dwLowDateTime;
	ts += (uint64)ft.dwHighDateTime << 32;
	return ts - offset;
#endif
#if POSIX
	timespec t;
	int r = clock_gettime(CLOCK_REALTIME, &t); // ломают спидхаком
	if (r != 0)
		LogPosixError("GetUtcTimestamp");

	return TimespecToTicks(t);
#endif
}

DateTime::DateTime(uint64 ticks, DateTimeKind kind) {
	this->rawData_ = ticks;
	switch (kind) {
	case DateTimeKind::Local:
		this->rawData_ |= KindLocal;
		break;
	case DateTimeKind::Utc:
		this->rawData_ |= KindUtc;
		break;
	default:
		break;
	}
}

DateTime::DateTime() {


}

DateTime DateTime::AddMilliseconds(uint64 msecs) const
{
	return DateTime{ rawData_ + TicksPerMillisecond * msecs, Kind() };
}

uint64 DateTime::TotalMilliseconds() const
{
	return rawData_ / TicksPerMillisecond;
}

DateTimeKind DateTime::Kind() const {
	return static_cast<DateTimeKind>((rawData_ & FlagsMask) >> 62);
}

DateTime DateTime::Date() {
	auto ticks = ExtractTicks();
	return DateTime(static_cast<uint64>(ticks - ticks % TicksPerDay) | ExtractKind());
}

int32 DateTime::Day() {
	return GetDatePart(DatePart::Day);
}

DayOfWeek DateTime::DayOfWeek() {
	return static_cast<::DayOfWeek>(ExtractTicks() / TicksPerDay % 7);
}

int32 DateTime::DayOfYear() {
	return GetDatePart(DatePart::Year);
}

int32 DateTime::Hour() {
	return static_cast<int32>(ExtractTicks() / TicksPerHour % 24);
}

int32 DateTime::Millisecond() {
	return static_cast<int32>(ExtractTicks() / TicksPerMillisecond % 1000);
}

int32 DateTime::Minute() {
	return static_cast<int32>(ExtractTicks() / TicksPerMinute % 60);
}

int32 DateTime::Month() {
	return GetDatePart(DatePart::Month);
}

int32 DateTime::Second() {
	return static_cast<int>(ExtractTicks() / TicksPerSecond % 60);
}

int32 DateTime::Year() {
	return GetDatePart(DatePart::Year);
}

uint64 DateTime::Ticks() {
	return ExtractTicks();
}

bool DateTime::IsLeapYear(int32 year) {
	if (year < 1 || year > 9999)
		return false;
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int64 DateTime::TimeZoneOffset()
{
	time_t t = time(nullptr);
	tm universal;
	tm local;
	gmtime_s(&universal, &t);
	localtime_s(&local, &t);

	int64 result;
	if (local.tm_yday == universal.tm_yday)
	{
		return
			(int64)(local.tm_hour - universal.tm_hour) * TicksPerHour
			+ (int64)(local.tm_min - universal.tm_min) * TicksPerMinute;
	}
	else if (local.tm_yday > universal.tm_yday)
	{
		result = (int64)(23 - universal.tm_hour) * TicksPerHour
			+ (int64)(60 - universal.tm_min) * TicksPerMinute
			+ (int64)local.tm_hour * TicksPerHour
			+ (int64)local.tm_min * TicksPerMinute;
		return result;
	}
	result = (int64)(23 - local.tm_hour) * TicksPerHour
		+ (int64)(60 - local.tm_min) * TicksPerMinute
		+ (int64)universal.tm_hour * TicksPerHour
		+ (int64)universal.tm_min * TicksPerMinute;
	return result;
}

DateTime DateTime::ToLocal() const
{
	if ((rawData_ | KindUtc) == 0)
		return *this;
	uint64 ticks = ExtractTicks() + GetUtcTimestamp();
	return DateTime(ticks, DateTimeKind::Local);
}

DateTime DateTime::ToUtc() const
{
	if ((rawData_ | KindLocal) == 0)
		return *this;
	uint64 ticks = ExtractTicks() - GetUtcTimestamp();
	return DateTime(ticks, DateTimeKind::Utc);
}

DateTime DateTime::Now() {
	auto utc = GetUtcTimestamp();

	uint64 tick = utc + TimeZoneOffset() + TicksTo1970;
	if (tick > MaxTicks) {
		return DateTime(MaxTicks, DateTimeKind::Local);
	}
	if (tick < MinTicks) {
		return DateTime(MinTicks, DateTimeKind::Local);
	}
	return DateTime(tick, DateTimeKind::Local);
}

DateTime DateTime::UtcNow() {
	return DateTime(GetUtcTimestamp() + TicksTo1970, DateTimeKind::Utc);
}

DateTime::DateTime(uint64 rawData) {
	this->rawData_ = rawData;
}

inline uint64 DateTime::ExtractTicks() const {
	return rawData_ & TicksMask;
}

inline uint64 DateTime::ExtractKind() const {
	return rawData_ & FlagsMask;
}

int32 DateTime::GetDatePart(DatePart part) {
	auto ticks = ExtractTicks();
	// n = number of days since 1/1/0001
	int n = static_cast<int>(ticks / TicksPerDay);
	// y400 = number of whole 400-year periods since 1/1/0001
	int y400 = n / DaysPer400Years;
	// n = day number within 400-year period
	n -= y400 * DaysPer400Years;
	// y100 = number of whole 100-year periods within 400-year period
	int y100 = n / DaysPer100Years;
	// Last 100-year period has an extra day, so decrement result if 4
	if (y100 == 4) y100 = 3;
	// n = day number within 100-year period
	n -= y100 * DaysPer100Years;
	// y4 = number of whole 4-year periods within 100-year period
	int y4 = n / DaysPer4Years;
	// n = day number within 4-year period
	n -= y4 * DaysPer4Years;
	// y1 = number of whole years within 4-year period
	int y1 = n / DaysPerYear;
	// Last year has an extra day, so decrement result if 4
	if (y1 == 4) y1 = 3;
	// If year was requested, compute and return it
	if (part == DatePart::Year) {
		return y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;
	}
	// n = day number within year
	n -= y1 * DaysPerYear;
	// If day-of-year was requested, return it
	if (part == DatePart::DayOfYear) return n + 1;
	// Leap year calculation looks different from IsLeapYear since y1, y4,
	// and y100 are relative to year 1, not year 0
	bool leapYear = y1 == 3 && (y4 != 24 || y100 == 3);
	auto days = leapYear ? DaysToMonth366 : DaysToMonth365;
	// All months have less than 32 days, so n >> 5 is a good conservative
	// estimate for the month
	int m = (n >> 5) + 1;
	// m = 1-based month number
	while (n >= days[m]) m++;
	// If month was requested, return it
	if (part == DatePart::Month) return m;
	// Return 1-based day-of-month
	return n - days[m - 1] + 1;
}

String DateTime::ToString()
{
	// fromat: 2020-03-31T17:27:10.492
	wchar_t buff[24];

	int32 y = Year() % 10000;
	buff[0] = (wchar_t)(y / 1000) | 0x30;
	buff[1] = (wchar_t)(y % 1000 / 100) | 0x30;
	buff[2] = (wchar_t)(y % 100 / 10) | 0x30;
	buff[3] = (wchar_t)(y % 10) | 0x30;

	buff[4] = 0x2d; // -

	int32 m = Month();
	buff[5] = (wchar_t)(m > 9 ? 0x31 : 0x30);
	buff[6] = (wchar_t)(m % 10) | 0x30;

	buff[7] = 0x2d; // -

	int32 d = Day();
	buff[8] = (wchar_t)(d / 10) | 0x30;
	buff[9] = (wchar_t)(d % 10) | 0x30;

	buff[10] = 0x20; // пробел

	int32 h = Hour();
	buff[11] = (wchar_t)(h / 10) | 0x30;
	buff[12] = (wchar_t)(h % 10) | 0x30;

	buff[13] = 0x3a; // :

	int32 min = Minute();
	buff[14] = (wchar_t)(min / 10) | 0x30;
	buff[15] = (wchar_t)(min % 10) | 0x30;

	buff[16] = 0x3a; // :

	int32 s = Second();
	buff[17] = (wchar_t)(s / 10) | 0x30;
	buff[18] = (wchar_t)(s % 10) | 0x30;

	buff[19] = 0x2e; // .

	int32 ms = Millisecond();
	buff[20] = (wchar_t)(ms % 1000 / 100) | 0x30;
	buff[21] = (wchar_t)(ms % 100 / 10) | 0x30;
	buff[22] = (wchar_t)(ms % 10) | 0x30;
	buff[23] = 0;

	return String(buff);
}