if(NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/DownloadFile.cmake")
get_filename_component(output_dir "${OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${output_dir}")
set(checksum_file "${output_dir}/cacert.pem.sha256")

download_file("https://curl.se/ca/cacert.pem.sha256" "${checksum_file}" TLS_VERIFY)
file(READ "${checksum_file}" checksum_content)
if(NOT checksum_content MATCHES "^([0-9a-fA-F]+)")
    file(REMOVE "${checksum_file}")
    message(FATAL_ERROR "Could not parse CA bundle checksum")
endif()
set(expected_hash "${CMAKE_MATCH_1}")

download_file(
    "https://curl.se/ca/cacert.pem"
    "${OUTPUT}"
    EXPECTED_HASH "SHA256=${expected_hash}"
    TLS_VERIFY
)
file(REMOVE "${checksum_file}")
