/** 
 * @file DeviceTime.h
 * @brief Header file for the DEVICETIME class.
 */
#ifndef DEVICETIME_H
#define DEVICETIME_H

#include "CommonIncludes.h"

#define TIMETAG "TIME"
#define DEFAULT_TIMEZONE "GMT+3"
#define DEFAULT_NTP_SERVER "ar.pool.ntp.org"
#define WAIT_TIME_FOR_NTP_MS 10000

/**
 * @class DEVICETIME
 * @brief Represents a device time management class.
 *
 * This class provides functionality for managing the device time, including updating time from a network server,
 * printing the current time, and saving/loading time to/from a file system.
 *
 * The class follows the Singleton design pattern, ensuring that only one instance of DEVICETIME can exist.
 *
 * Usage:
 *  - Call DEVICETIME::getInstance() to get the instance of DEVICETIME.
 *  - Call Begin() to initialize the device time.
 *  - Call updateTimeFromNet() to update the device time from a network server.
 *  - Call printTime() to print the current device time.
 *  - Call saveTimeToFs() to save the device time to a file system.
 *  - Call loadTimeFromFs() to load the device time from a file system.
 *
 * Note: The copy constructor and assignment operator are deleted to prevent copying of DEVICETIME instances.
 */
class DEVICETIME
{
public:
    /**
     * @brief Deleted copy constructor.
     */
    DEVICETIME(const DEVICETIME &) = delete;

    /**
     * @brief Deleted assignment operator.
     */
    DEVICETIME &operator=(const DEVICETIME &) = delete;

    /**
     * @brief Get the instance of DEVICETIME.
     * @return The instance of DEVICETIME.
     */
    static DEVICETIME &getInstance();

    /**
     * @brief Initialize the device time.
     * @return True if successful, false otherwise.
     */
    bool Begin(void);

    /**
     * @brief Update the device time from a network server.
     * @param server The network server address.
     * @param serverLen The length of the server address.
     * @return True if successful, false otherwise.
     */
    bool updateTimeFromNet(const char *server, uint8_t serverLen);

    /**
     * @brief Print the current device time.
     * @return True if successful, false otherwise.
     */
    bool printTime(void);

    /**
     * @brief Save the device time to a file system.
     * @return True if successful, false otherwise.
     */
    bool saveTimeToFs(void);

    /**
     * @brief Load the device time from a file system.
     * @return True if successful, false otherwise.
     */
    bool loadTimeFromFs(void);

private:
    /**
     * @brief Private constructor.
     */
    DEVICETIME() {}
};

#endif // DEVICETIME_H