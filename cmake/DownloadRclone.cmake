if(NOT DEFINED RCLONE_VERSION OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "RCLONE_VERSION and OUTPUT are required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/DownloadFile.cmake")
set(archive_name "rclone-v${RCLONE_VERSION}-linux-arm-v7.zip")
set(base_url "https://github.com/rclone/rclone/releases/download/v${RCLONE_VERSION}")
get_filename_component(output_dir "${OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${output_dir}")

set(sums_file "${output_dir}/rclone-SHA256SUMS")
download_file("${base_url}/SHA256SUMS" "${sums_file}" TLS_VERIFY)

file(STRINGS "${sums_file}" checksum_lines)
set(expected_hash "")
foreach(line IN LISTS checksum_lines)
    if(line MATCHES "^([0-9a-fA-F]+)[ \t]+\\*?${archive_name}$")
        set(expected_hash "${CMAKE_MATCH_1}")
        break()
    endif()
endforeach()
if(NOT expected_hash)
    message(FATAL_ERROR "No checksum found for ${archive_name}")
endif()

set(archive_file "${output_dir}/${archive_name}")
download_file(
    "${base_url}/${archive_name}"
    "${archive_file}"
    EXPECTED_HASH "SHA256=${expected_hash}"
    TLS_VERIFY
)

set(extract_dir "${output_dir}/rclone-extract")
file(REMOVE_RECURSE "${extract_dir}")
file(MAKE_DIRECTORY "${extract_dir}")
file(ARCHIVE_EXTRACT INPUT "${archive_file}" DESTINATION "${extract_dir}")
set(extracted_rclone "${extract_dir}/rclone-v${RCLONE_VERSION}-linux-arm-v7/rclone")
if(NOT EXISTS "${extracted_rclone}")
    message(FATAL_ERROR "rclone binary was not found in ${archive_name}")
endif()
file(COPY_FILE "${extracted_rclone}" "${OUTPUT}")
file(CHMOD "${OUTPUT}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE "${extract_dir}")
file(REMOVE "${archive_file}" "${sums_file}")
