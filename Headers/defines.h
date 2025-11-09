#ifndef DEFINES_H
#define DEFINES_H

#define persist         static
#define internal        static

#define kb(x)           (x * 1024)
#define mb(x)           (kb(x) * 1024)
#define gb(x)           (mb(x) * 1024)

#define unused(x)       (void)(x)
#define packed          __attribute__((__packed__))

#define file            __FILE__
#define line            __LINE__

#endif // DEFINES_H