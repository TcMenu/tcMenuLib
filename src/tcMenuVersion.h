/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_VERSION_H
#define TCMENU_VERSION_H

namespace tccore {

// here we define the version as both a string and separate field
#define TCMENU_MAJOR 1
#define TCMENU_MINOR 8
#define TCMENU_PATCH 0
#define TCMENU_VERSION_STR "1.8.0"

/**
 * A helper to generate the major minor version numbers used in the protocol
 */
#define majorminor(maj, min) ((maj * 100) + min)

/**
 * Definition of the current API version
 */
#define API_VERSION majorminor(TCMENU_MAJOR, TCMENU_MINOR)

}

#endif //TCMENU_VERSION_H
