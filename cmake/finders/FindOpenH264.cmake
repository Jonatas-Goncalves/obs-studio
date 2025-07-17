find_library(OPENH264_LIBRARY NAMES openh264)
find_path(OPENH264_INCLUDE_DIR wels/codec_api.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenH264 DEFAULT_MSG OPENH264_LIBRARY OPENH264_INCLUDE_DIR)

mark_as_advanced(OPENH264_LIBRARY OPENH264_INCLUDE_DIR)

if(OpenH264_FOUND)
    add_library(OpenH264::OpenH264 UNKNOWN IMPORTED)
    set_target_properties(OpenH264::OpenH264 PROPERTIES
        IMPORTED_LOCATION "${OPENH264_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENH264_INCLUDE_DIR}")
endif()
