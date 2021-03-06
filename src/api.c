/* api.c: Functions to directly convert APIs and inner data structures, usually used by the library itself but men.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "data.h"
#include "utils.h"
#define BUFFER_SIZE 4096

static const char *BaseUrl = "http://gank.io/api";
static size_t _gank_io_curl_write_callback (char *ptr, size_t size, size_t nmemb, void *userdata);
static int _gank_io_item_single_parse (GankIoItem *item, json_object *obj);





int _gank_io_api_daily_url_form (char **url, unsigned int year, unsigned int month, unsigned int day)
{
    char *buf = gank_io_xmalloc (BUFFER_SIZE);
    int retVal = 0;

    retVal = snprintf (buf, BUFFER_SIZE, "%s/day/%d/%d/%d", BaseUrl, year, month, day);
    if (retVal <= BUFFER_SIZE) {
        *url = buf;
        return EXIT_SUCCESS;
    } else {
        // Overflowed. (It seems impossible)
        gank_io_error ("String overflowed!");
    }

    // Whatever.
    return EXIT_FAILURE;
}





int _gank_io_api_sorted_url_form (char **url, enum GankIoResourceType resType, unsigned int nRequest, unsigned int nPage)
{
    char *buf = gank_io_xmalloc (BUFFER_SIZE);
    char *strResType = gank_io_xmalloc (BUFFER_SIZE);
    int retVal = 0;

    switch (resType) {
        case Goods:
            strncpy (strResType, "福利", BUFFER_SIZE);
            break;
        case Android:
            strncpy (strResType, "Android", BUFFER_SIZE);
            break;
        case Ios:
            strncpy (strResType, "iOS", BUFFER_SIZE);
            break;
        case RelaxingMovies:
            strncpy (strResType, "休息视频", BUFFER_SIZE);
            break;
        case ExtendRes:
            strncpy (strResType, "拓展资源", BUFFER_SIZE);
            break;
        case Frontend:
            strncpy (strResType, "前端", BUFFER_SIZE);
            break;
        default:
            // WTF
            gank_io_error ("Unexpected: Wrong resType");
            break;
    }

    retVal = snprintf (buf, BUFFER_SIZE, "%s/%s/%d/%d", BaseUrl, strResType, nRequest, nPage);
    if (retVal <= BUFFER_SIZE) {
        *url = buf;
        gank_io_xfree (strResType);
        return EXIT_SUCCESS;
    } else {
        gank_io_error ("String overflowed!");
    }

    // Whatever.
    gank_io_xfree (strResType);
    return EXIT_FAILURE;
}





int _gank_io_api_get (char **json, const char *url)
{
    CURL *curl = curl_easy_init ();
    void *buffer = NULL;

    if (curl) {
        curl_easy_setopt (curl, CURLOPT_URL, url);
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, _gank_io_curl_write_callback);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, buffer);

        curl_easy_perform (curl);
        curl_easy_cleanup (curl);
    } else {
        gank_io_error ("libcurl initialization failed");
    }

    *json = buffer;
    return EXIT_SUCCESS;
}





int _gank_io_api_daily_parse (GankIoDailyFeed *dailyFeed, const char *json)
{
    json_object *jReceived            = NULL;
    json_object *jError               = NULL;
    json_object *jCategory            = NULL;
    json_object *jResults             = NULL;
    json_object *jRsltPtTypeArray     = NULL;
    GankIoDailyFeed *dailyFeedTmp     = gank_io_xmalloc (sizeof (GankIoDailyFeed)); // Temporary 'GankIoDailyFeed'

    jReceived = json_tokener_parse (json);
    if (jReceived) {
        json_bool bRetVal = 0;

        bRetVal = json_object_object_get_ex (jReceived, "error", &jError);
        if ((bRetVal == TRUE) && (strcmp (json_object_to_json_string (jError), "false") == 0)) {
            bRetVal = json_object_object_get_ex (jReceived, "category", &jCategory);
            if ((bRetVal == TRUE) && json_object_is_type (jCategory, json_type_array)) {
                // Count the number of catrgories , declare '_GankIoDailyItem's, and set 'nDailyItem'
                unsigned int nCategory = json_object_array_length (jCategory);
                struct _GankIoDailyItem *dailyItemTmp = gank_io_xmalloc (sizeof (struct _GankIoDailyItem) * nCategory);

                dailyFeedTmp->nDailyItem = nCategory;

                bRetVal = json_object_object_get_ex (jReceived, "results", &jResults);
                if (bRetVal == TRUE) {
                    unsigned int i = 0; // nDailyItem indicator
                    json_object_object_foreach (jResults, key, val) {
                        bRetVal = json_object_object_get_ex (jResults, key, &jRsltPtTypeArray);
                        if ((bRetVal == TRUE) && json_object_is_type (jRsltPtTypeArray, json_type_array)) {
                            unsigned int nRsltPtTypeArray = json_object_array_length (jRsltPtTypeArray);
                            GankIoItem *itemTmp = gank_io_xmalloc (sizeof (GankIoItem) * nRsltPtTypeArray);

                            // Fill in 'GankIoItem's
                            dailyItemTmp[i].nItem = nRsltPtTypeArray;
                            dailyItemTmp[i].type  = _gank_io_api_restype_toenum (key);

                            for (int j = 0; j < nRsltPtTypeArray; j++) { // 'j' is nItem indicator
                                _gank_io_item_single_parse (&itemTmp[j], json_object_array_get_idx (jRsltPtTypeArray, j));
                            }
                            dailyItemTmp[i].item = &itemTmp; // Connect temporary 'GankIoItem' pointer to the upper one

                        } else {
                            gank_io_error ("Unexpected error: Cannot get jRsltPtTypeArray.");
                        }

                        i += 1; // Plus 1 to add the current position of dailyItemTmp
                    } // json_object_object_foreach (jResults, key, val)
                } else {
                    gank_io_error ("Unexpected error: Cannot get jResults.");
                }

                // Connect temporary 'struct _GankIoDailyItem' to the upper one
                dailyFeedTmp->dailyItem = &dailyItemTmp;

            } else {
                gank_io_error ("Unexpected error: Cannot get jCategory.");
            }
        } else {
            gank_io_warn ("Tainted JSON data: either \"error\"==\"true\" or cannot to get jError");
        }
    } else {
        gank_io_warn ("Cannot parse JSON string. Please report this bug to the author.");
    }

    dailyFeed = dailyFeedTmp;
    return EXIT_SUCCESS;
}





int _gank_io_api_sorted_parse (GankIoItem **item, const char *json, unsigned int nItem)
{
    json_object *jReceived = NULL; // JSON Object for received data ('json')
    json_object *jError    = NULL; // JSON Object for the "error" section
    json_object *jResults  = NULL; // JSON Object for the "results" section

    jReceived = json_tokener_parse (json);
    if (jReceived) {
        json_bool bRetVal = 0;

        bRetVal = json_object_object_get_ex (jReceived, "error", &jError); // Check if request fails
        if ((bRetVal == TRUE) && (strcmp (json_object_to_json_string (jError), "false") == 0)) {
            bRetVal = json_object_object_get_ex (jReceived, "results", &jResults); // Get "results" section
            if ((bRetVal == TRUE) && (json_object_is_type (jResults, json_type_array))) { // NOTE: 'jResults' is an array
                // XXX: nItem must be the actual number of '**item'. If it is:
                //   - If nItem > json_object_array_length (jResults) some items will remain NULL;
                //   - If nItem < json_object_array_length (jResults) JSON will not be fully parsed.
                for (int i = 0; i < nItem; i++) {
                    _gank_io_item_single_parse (item[i], json_object_array_get_idx (jResults, i));
                }
            } else {
                gank_io_error ("Unexpected error: failed to get jResults. Please report this bug to the author.");
            }
        } else {
            gank_io_warn ("Tainted JSON data: either \"error\"==\"true\" or cannot to get jError");
        }
    } else {
        gank_io_warn ("Cannot parse JSON string. Please report this bug to the author.");
    }

    // Free JSON objects
    // FIXME: Until json_object_put returns 1 we cannot guarantee the object was free'd.
    json_object_put (jReceived);
    json_object_put (jError);
    json_object_put (jResults);

    return EXIT_SUCCESS;
}





static size_t _gank_io_curl_write_callback (char *ptr, size_t size, size_t nmemb, void *userdata)
{
    // "[ptr] points to the delivered data, and the size of that data is [size] multiplied with [nmemb]."
    // "Set the [userdata] argument with the CURLOPT_WRITEDATA option."
    //   -- https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html

    void *ptrToWrite = NULL; // Pointer that points to where data should be written into
    size_t bufUsed = 0;      // Used space of buffer ('userdata')

    bufUsed = strlen (userdata == NULL ? "\0" : userdata);

    // realloc: "If ptr is NULL, the behavior is the same as calling malloc(new_size). "
    //   -- http://en.cppreference.com/w/c/memory/realloc
    userdata = gank_io_xrealloc (userdata, bufUsed + size * nmemb);
    ptrToWrite = userdata + bufUsed;

    return fwrite (ptr, size, nmemb, ptrToWrite);
}





static int _gank_io_item_single_parse (GankIoItem *item, json_object *obj)
{
    json_object_object_foreach (obj, key, val) {
        const char *value = json_object_to_json_string (val);
        const char *strResType = NULL;
        size_t     valueSize = strlen (value);

        // Fill in the specified GankIoItems
        switch (key[0]) { // 'key' is of type 'char*'
            // NOTE: Tricks to make parsing JSON object faster.
            //   Use the first letter of the JSON key first. If
            //   there's a collision, use 'strcmp'.
            case '_': // "_id"
                item->id = gank_io_xmalloc (valueSize);
                strncpy (item->id, value, valueSize);
                break;
            case 'c': // "createdAt"
                item->createdAt = gank_io_xmalloc (valueSize);
                strncpy (item->id, value, valueSize);
                break;
            case 'd': // "desc"
                item->desc = gank_io_xmalloc (valueSize);
                strncpy (item->id, value, valueSize);
                break;
            case 'p': // "publishedAt"
                item->publishedAt = gank_io_xmalloc (valueSize);
                strncpy (item->id, value, valueSize);
                break;
            case 's': // "source"
                item->source = gank_io_xmalloc (valueSize);
                strncpy (item->id, value, valueSize);
                break;
            case 't': // "type"
                strResType = json_object_to_json_string (val);
                item->type = _gank_io_api_restype_toenum (strResType);
                break;
            case 'u': // "url" & "used" (collision)
                if (strcmp (key, "url") == 0) {
                    item->desc = gank_io_xmalloc (valueSize);
                    strncpy (item->id, value, valueSize);
                } else if (strcmp (key, "used") == 0) {
                    const char *strUsed = json_object_to_json_string (val);
                    if (strcmp (strUsed, "true") == 0) {
                        item->used = 1;
                    } else {
                        item->used = 0;
                    }
                } else {}
                break;
            case 'w': // "who"
                item->who = gank_io_xmalloc (valueSize);
                strncpy (item->id, value, valueSize);
                break;
            default:
                // Unrecognized key (May because of API changes)
                gank_io_warn ("Unrecognized key: %s. Please report this bug to the author.", key);
                break;
        }
    }

    return EXIT_SUCCESS;
}





char *_gank_io_api_restype_tostring (enum GankIoResourceType resType)
{
    char *strResTypeTmp = gank_io_xmalloc (BUFFER_SIZE);

    switch (resType) {
        case Goods:
            strncpy (strResTypeTmp, "福利", BUFFER_SIZE);
            break;
        case Android:
            strncpy (strResTypeTmp, "Android", BUFFER_SIZE);
            break;
        case Ios:
            strncpy (strResTypeTmp, "iOS", BUFFER_SIZE);
            break;
        case RelaxingMovies:
            strncpy (strResTypeTmp, "休息视频", BUFFER_SIZE);
            break;
        case ExtendRes:
            strncpy (strResTypeTmp, "拓展资源", BUFFER_SIZE);
            break;
        case Frontend:
            strncpy (strResTypeTmp, "前端", BUFFER_SIZE);
            break;
        case Recommends:
            strncpy (strResTypeTmp, "瞎推荐", BUFFER_SIZE);
            break;
        case App:
            strncpy (strResTypeTmp, "App", BUFFER_SIZE);
            break;
        case All:
            strncpy (strResTypeTmp, "all", BUFFER_SIZE);
            break;
        default:
            gank_io_warn ("Unrecognized resource type: %d. Please report this bug to the author.", resType);
            break;
    }

    return strResTypeTmp;
}





enum GankIoResourceType _gank_io_api_restype_toenum (const char *strResType)
{
    enum GankIoResourceType resTypeTmp;

    // Please refer to the API documents on gank.io (http://gank.io/api) for more information about
    //   these strings.
    if (strcmp (strResType, "福利") == 0) {
        resTypeTmp = Goods;
    } else if (strcmp (strResType, "Android") == 0) {
        resTypeTmp = Android;
    } else if (strcmp (strResType, "iOS") == 0) {
        resTypeTmp = Ios;
    } else if (strcmp (strResType, "休息视频") == 0) {
        resTypeTmp = RelaxingMovies;
    } else if (strcmp (strResType, "拓展资源") == 0) {
        resTypeTmp = ExtendRes;
    } else if (strcmp (strResType, "前端") == 0) {
        resTypeTmp = Frontend;
    } else if (strcmp (strResType, "瞎推荐") == 0) {
        resTypeTmp = Recommends;
    } else if (strcmp (strResType, "App") == 0) {
        resTypeTmp = App;
    } else if (strcmp (strResType, "all") == 0) {
        // ???
        resTypeTmp = All;
    } else {
        // ?????? Unknown resource type
        gank_io_warn ("Unrecognized resource type: %s. Please report this bug to the author.", strResType);
    }

    return resTypeTmp;
}
