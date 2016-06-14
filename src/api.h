/* api.h: Functions to directly convert APIs and inner data structures, usually used by the library itself but men.
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

/* _gank_io_api_url_form: Generate the RESTFul API according to the arguments
 */
int _gank_io_api_url_form ();

/* _gank_io_apt_get: Perform network connection to fetch JSON from gank.io
 */
int _gank_io_api_get ();

/* _gank_io_api_parse: Parse JSON to convert it into inner data structures
 */
int _gank_io_api_parse ();