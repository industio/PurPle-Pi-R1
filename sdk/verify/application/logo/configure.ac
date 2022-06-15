#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([logo], [1.0.0])
AC_CONFIG_SRCDIR([logo.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdlib.h string.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday memset munmap strstr])

debug_support="no"
disp_timingtype="VGA"
disp_timingsync="USER"
png_support="yes"
bmp_support="no"
jpeg_support="yes"
raw_support="yes"

AC_ARG_WITH(timingtype,
[  --with-timingtype=LCD/HDMI/VGA  disp out type <default=LCD>],
disp_timingtype=$withval)

AC_ARG_WITH(lcdheader,
[  --with-lcdheader=SAT070CP50_1024x600.h/...  lcd arguments header <default=SAT070CP50_1024x600>],
disp_lcdheader=$withval)
AC_DEFINE_UNQUOTED(MI_DISP_INTF_LCD_HEADER, "${disp_lcdheader}",[Define if MI_DISP_INTF_TYPE=LCD])

CheckDebugsupport()
{
AC_ARG_ENABLE(logodebug,
[  --enable-logodebug      support debug <default=no>],
debug_support=$enableval)
    if test "debug_support" = "xyes"; then
        AC_DEFINE(LOGO_DEBUG, 1,
                  [Define if  debug])
    fi
}

CheckPNGsupport()
{
AC_ARG_ENABLE(png,
[  --enable-png      support png logo <default=yes>],
png_support=$enableval)

    if test "x$png_support" = "xyes"; then
        AC_DEFINE(LOGO_WITH_PNG, 1,
                  [Define if support png logo])
        LOGO_SUBDIRS="$VIDEO_SUBDIRS png "
    fi
}
CheckBMPsupport()
{
AC_ARG_ENABLE(bmp,
[  --enable-bmp      support bmp logo <default=no>],
bmp_support=$enableval)

    if test "x$bmp_support" = "xyes"; then
        AC_DEFINE(LOGO_WITH_BMP, 1,
                  [Define if support bmp logo])
        LOGO_SUBDIRS="$VIDEO_SUBDIRS bmp "
    fi
}
CheckJPEGsupport()
{
AC_ARG_ENABLE(jpeg,
[  --enable-jpeg      support jpeg logo <default=yes>],
jpeg_support=$enableval)

    if test "x$jpeg_support" = "xyes"; then
        AC_DEFINE(LOGO_WITH_JPEG, 1,
                  [Define if support jpeg logo])
        LOGO_SUBDIRS="$VIDEO_SUBDIRS jpeg "
    fi
}
CheckRAWsupport()
{
AC_ARG_ENABLE(raw,
[  --enable-raw      support raw logo <default=yes>],
raw_support=$enableval)

    if test "x$raw_support" = "xyes"; then
        AC_DEFINE(LOGO_WITH_RAW, 1,
                  [Define if support raw logo])
    fi
}

{ 
    CheckDebugsupport
    CheckPNGsupport
    CheckBMPsupport
    CheckJPEGsupport
    CheckRAWsupport
}

case "$disp_timingtype" in
    HDMI)
        AC_DEFINE(MI_DISP_INTF_TYPE, E_MI_DISP_INTF_HDMI,
            [Define if MI_DISP_INTF_TYPE=HDMI])
        AC_DEFINE(MI_DISP_INTF_TYPE_HDMI, 1,[Define if MI_DISP_INTF_TYPE=HDMI])
    ;;
    VGA)
        AC_DEFINE(MI_DISP_INTF_TYPE, E_MI_DISP_INTF_VGA,
            [Define if MI_DISP_INTF_TYPE=VGA])
        AC_DEFINE(MI_DISP_INTF_TYPE_VGA, 1,[Define if MI_DISP_INTF_TYPE=VGA])
    ;;
    LCD)
        AC_DEFINE(MI_DISP_INTF_TYPE, E_MI_DISP_INTF_LCD,
            [Define if MI_DISP_INTF_TYPE=LCD])
        AC_DEFINE(MI_DISP_INTF_TYPE_LCD, 1,[Define if MI_DISP_INTF_TYPE=LCD])
    ;;
esac


AC_OUTPUT
