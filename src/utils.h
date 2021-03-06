/* utils.h: Utilities used by libGankIo
 * Copyright (C) 2016 Junde Yhi <lmy441900@gmail.com>
 * This file is part of libGankIo.
 *
 * libGankIo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libGankIo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU lesser General Public
 * License along with libGankIo.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _GANK_IO_UTILS_H
#define _GANK_IO_UTILS_H

/* GankIoOutputType: type of messages for gank_io_output */
enum GankIoOutputType {
    Info = 0,
    Warn_NoAck = 1,
    Warn_Ack = 2,
    Error = 3
};

/* gank_io_xmalloc: safe malloc implementation, which kills the program when memory allocation fails.
 */
void *gank_io_xmalloc (const size_t size);

/* gank_io_xrealloc: safe realloc implementation, which kills the program when memory allocation fails.
 */
void *gank_io_xrealloc (void *ptr, const size_t size);

/* gank_io_xfree: safe free implementation, which frees the data and sets the pointer to NULL.
 */
#define gank_io_xfree(ptr) \
{ \
    free ((ptr)); \
    (ptr) = NULL; \
}

/* gank_io_output: print messages in different types.
 */
void _gank_io_output (const char *file, const char *func, const int line, enum GankIoOutputType outputType, const char *fmt, ...);
#define gank_io_output(output_type, ...) _gank_io_output (__FILE__, __func__, __LINE__, output_type, ##__VA_ARGS__)

#define gank_io_info(...) gank_io_output (Info, ##__VA_ARGS__)
#define gank_io_warn_noack(...) gank_io_output (Warn_NoAck, ##__VA_ARGS__)
#define gank_io_warn_ack(...) gank_io_output (Warn_Ack, ##__VA_ARGS__)
#define gank_io_warn gank_io_warn_noack
#define gank_io_warning gank_io_warn
#define gank_io_error(...) gank_io_output (Error, ##__VA_ARGS__)

#endif /* end of include guard: _GANK_IO_UTILS_H */
