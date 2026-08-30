#pragma once

void SetLogEnabled(bool enabled);

void Log(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
