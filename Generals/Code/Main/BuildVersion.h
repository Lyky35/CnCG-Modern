/*
**	Static build version header.
**	In the original project BuildVersion.h/GeneratedVersion.h were emitted at build time by
**	Tools/buildVersionUpdate from the source-controllers version file. For direct builds
**	(and this cross-compiled fork) they are provided statically here.
*/
#ifndef __BUILDVERSION_H
#define __BUILDVERSION_H

#ifndef VERSION_MAJOR
#define VERSION_MAJOR 1
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif

#ifndef VERSION_BUILDNUM
#define VERSION_BUILDNUM 32
#endif

#ifndef VERSION_LOCALBUILDNUM
#define VERSION_LOCALBUILDNUM VERSION_BUILDNUM
#endif

#ifndef VERSION_BUILDUSER
#define VERSION_BUILDUSER "builder"
#endif

#ifndef VERSION_BUILDLOC
#define VERSION_BUILDLOC "localhost"
#endif

#endif
