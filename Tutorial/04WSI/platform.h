#ifndef PLATFORM_H
#define PLATFORM_H

/* Если мы используем Windows, то тогда у нас есть сециальное макроимя _WIN32 (т.е. оно объявлено по умолчанию).
 * Поэтому, если оно есть, нам нужно объявить другое макроимя, говорящее, что мы будем использвать Win32 Surface.
*/ 

#if defined(_WIN32)
// VK_USE_PLATFORM_WIN32_KHR — именно это макроимя и нужно объявить. 
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#endif // PLATFORM_H