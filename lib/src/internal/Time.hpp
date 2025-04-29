#pragma once

#include <ctime>

bool operator==(timespec const& lhs, timespec const& rhs);

bool operator!=(timespec const& lhs, timespec const& rhs);

bool operator<(timespec const& lhs, timespec const& rhs);

bool operator<=(timespec const& lhs, timespec const& rhs);

bool operator>(timespec const& lhs, timespec const& rhs);

bool operator>=(timespec const& lhs, timespec const& rhs);