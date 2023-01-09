/**
 * @file language_select.h
 * @brief contains the logic to select the right language entries for internal tcMenu library strings at compile time
 */

#ifndef TCLIBRARYDEV_LANGUAGE_SELECT_H
#define TCLIBRARYDEV_LANGUAGE_SELECT_H

//
// The highest precedence is to add `project_locale.h` if your build chain supports it. English is the default
//
// PlatformIO/full build systems -  for French ASCII as an example -DTC_LOCALE_FRENCH -DTC_LOCAL_ASCII
//
// If you're using original Arduino IDE you can force a definition of TC_LOCALE here. Commented out example below.
// For example if we wanted french with only ASCII (32..126)
//
// #define TC_LOCALE_FR
// #define TC_LOCAL_ASCII
//


#if __has_include(<project_locale.h>)
# include <project_locale.h>
#elif defined(TC_LOCALE_FR)
#if defined(TC_LOCAL_ASCII)
# include "language_fr_ascii.h"
#else
# include "language_fr.h"
#endif // use ASCII
#elif defined(TC_LOCALE_SK)
#if defined(TC_LOCAL_ASCII)
# include "language_sk_ascii.h"
#else
# include "language_sk.h"
#endif // use ASCII
#elif defined(TC_LOCALE_CS)
#if defined(TC_LOCAL_ASCII)
# include "language_cs_ascii.h"
#else
# include "language_cs.h"
#endif // use ASCII
#else
# include "language_en.h"
#endif

#endif //TCLIBRARYDEV_LANGUAGE_SELECT_H
