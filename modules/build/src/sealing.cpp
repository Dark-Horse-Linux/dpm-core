#include "sealing.hpp"


bool file_already_compressed(const std::string& path)
{
    // Convert string to filesystem path
    std::filesystem::path fs_path(path);

    // Check if it's a regular file (not a directory)
    if (!std::filesystem::is_regular_file(fs_path))
    {
        return false;
    }

    // Open the file and check the magic number
    std::ifstream file(fs_path, std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    unsigned char header[2];
    if (!file.read(reinterpret_cast<char*>(header), 2))
    {
        file.close();
        return false;
    }

    // Close the file after reading
    file.close();

    // Check for gzip magic number (0x1F 0x8B)
    return (header[0] == 0x1F && header[1] == 0x8B);
}

// transform a directory at source_dir into a gzipped tarball at output_path
// source_dir and output_path cannot match
bool compress_directory( const std::string source_dir, const std::string output_path )
{
    // Verify source directory exists
    std::filesystem::path src_path(source_dir);
    if ( !std::filesystem::exists(src_path) )
    {
        // path to compress doesn't exist, so bail
        dpm_log(LOG_ERROR, ("Source directory does not exist: " + source_dir).c_str());
        return false;
    }

    // Check if source is actually a directory
    if ( !std::filesystem::is_directory(src_path) )
    {
        // it's not a directory, so bail
        dpm_log(LOG_ERROR, ("Source is not a directory: " + source_dir).c_str());
        return false;
    }

    // Check if source and output paths are the same
    if ( source_dir == output_path )
    {
        // they match, so bail
        dpm_log(LOG_ERROR, "Source directory and output path cannot be the same");
        return false;
    }

    // if the output path is empty, bail
    if ( output_path.empty() )
    {
        dpm_log(LOG_ERROR, "Output path is empty.  Refusing to write a non-existant archive.");
        return false;
    }

    // convert the output path to a path object
    std::filesystem::path out_path(output_path);

    // get the parent path directory
    std::filesystem::path parent_path = out_path.parent_path();

    // if the parent path is not empty and it does not exist
    if ( !parent_path.empty() && !std::filesystem::exists(parent_path) )
    {
        // can't write to output path so bail
        dpm_log( LOG_ERROR, ( "Output path parent directory does not exist: " + parent_path.string()).c_str() );
        return false;
    }

    dpm_log( LOG_INFO, ("Compressing directory " + source_dir + " to archive " + out_path.string()).c_str() );

    // Use libarchive to create a compressed tarball
    struct archive * a;
    struct archive_entry * entry;
    char buff[8192];
    int len;

    // Create a new archive
    a = archive_write_new();

    // Set the compression format to gzip
    archive_write_add_filter_gzip(a);

    // Set the archive format to tar
    archive_write_set_format_pax_restricted(a);

    // Open the output file
    if ( archive_write_open_filename( a, out_path.string().c_str()) != ARCHIVE_OK )
    {
        dpm_log( LOG_ERROR, ("Failed to create archive: " + out_path.string()).c_str() );
        archive_write_free(a);
        return false;
    }

    // Get the directory name to use as parent in the archive
    std::string output_parent_dir = src_path.filename().string();

    // First add the parent directory entry
    entry = archive_entry_new();
    archive_entry_set_pathname( entry, output_parent_dir.c_str() );
    archive_entry_set_filetype( entry, AE_IFDIR );
    archive_entry_set_perm( entry, 0755 );  // Standard directory permissions
    archive_write_header( a, entry );
    archive_entry_free( entry );

    // Create a vector to store all entries in the directory for proper empty directory handling
    std::vector<std::filesystem::path> all_entries;

    // First collect all entries including empty directories
    try
    {
        for ( const auto& dir_entry : std::filesystem::recursive_directory_iterator(src_path) )
        {
            all_entries.push_back(dir_entry.path());
        }
    }
    catch (const std::exception& e)
    {
        dpm_log(LOG_ERROR, ("Error scanning directory: " + std::string(e.what())).c_str());
        archive_write_close(a);
        archive_write_free(a);
        return false;
    }

    // Walk through all collected entries and add them to the archive
    try
    {
        for ( const auto& full_path : all_entries )
        {
            // Get the relative path from the component path
            std::string relative_path = std::filesystem::relative( full_path, src_path ).string();

            // Path in archive with parent directory
            std::string archive_path_entry = output_parent_dir + "/" + relative_path;

            // Create a new entry for this file/directory
            entry = archive_entry_new();

            // Set the entry path with parent directory
            archive_entry_set_pathname(entry, archive_path_entry.c_str());

            // Handle different file types
            if ( std::filesystem::is_symlink(full_path) )
            {
                // For symbolic links, set the link target
                std::filesystem::path target = std::filesystem::read_symlink(full_path);
                archive_entry_set_symlink(entry, target.c_str());
                archive_entry_set_filetype(entry, AE_IFLNK);

                // Get file information using lstat for the symlink itself
                struct stat st;
                lstat(full_path.string().c_str(), &st);
                archive_entry_copy_stat(entry, &st);

                // Write the entry header
                archive_write_header(a, entry);
            }
            else if ( std::filesystem::is_directory(full_path) )
            {
                // For directories, set the directory type
                archive_entry_set_filetype(entry, AE_IFDIR);

                // Get file information using stat
                struct stat st;
                stat(full_path.string().c_str(), &st);
                archive_entry_copy_stat(entry, &st);

                // Write the entry header
                archive_write_header(a, entry);
            }
            else if ( std::filesystem::is_regular_file(full_path) )
            {
                // For regular files, add the file content
                archive_entry_set_filetype(entry, AE_IFREG);

                // Get file information using stat
                struct stat st;
                stat(full_path.string().c_str(), &st);
                archive_entry_copy_stat(entry, &st);

                // Write the entry header
                archive_write_header(a, entry);

                // Write file contents
                std::ifstream file(full_path, std::ios::binary);
                if (file.is_open())
                {
                    while (!file.eof())
                    {
                        file.read(buff, sizeof(buff));
                        len = file.gcount();
                        if (len > 0)
                        {
                            archive_write_data(a, buff, len);
                        }
                    }
                    file.close();
                }
                else
                {
                    dpm_log(LOG_ERROR, ("Failed to open file for archiving: " + full_path.string()).c_str());
                }
            }

            // Free the entry
            archive_entry_free(entry);
        }
    }
    catch (const std::exception& e)
    {
        dpm_log(LOG_ERROR, ("Error archiving directory: " + std::string(e.what())).c_str());
        archive_write_close(a);
        archive_write_free(a);
        return false;
    }

    // Close and free the archive
    archive_write_close(a);
    archive_write_free(a);

    dpm_log(LOG_INFO, ("Archive created at: " + out_path.string()).c_str());

    return true;
}

// Uncompress a gzipped tarball at source_path to a directory at output_dir
bool uncompress_archive(const std::string& source_path, const std::string& output_dir)
{
    dpm_log(LOG_INFO, ("Extracting archive " + source_path + " to directory " + output_dir).c_str());

    // Verify source file exists
    std::filesystem::path src_path(source_path);
    if (!std::filesystem::exists(src_path)) {
        dpm_log(LOG_ERROR, ("Source archive does not exist: " + source_path).c_str());
        return false;
    }

    // Check if source is actually a regular file
    if (!std::filesystem::is_regular_file(src_path)) {
        dpm_log(LOG_ERROR, ("Source is not a file: " + source_path).c_str());
        return false;
    }

    // Convert the output directory to a path object
    std::filesystem::path out_path(output_dir);

    // Get the parent path directory
    std::filesystem::path parent_path = out_path.parent_path();

    // If the parent path is not empty and it does not exist
    if (!parent_path.empty() && !std::filesystem::exists(parent_path)) {
        // Can't write to output path so bail
        dpm_log(LOG_ERROR, ("Output path parent directory does not exist: " + parent_path.string()).c_str());
        return false;
    }

    // Use libarchive to extract the archive
    struct archive* a;
    struct archive* ext;
    struct archive_entry* entry;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    int r;

    a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    // Open the archive
    if ((r = archive_read_open_filename(a, source_path.c_str(), 10240)) != ARCHIVE_OK) {
        dpm_log(LOG_ERROR, ("Failed to open archive: " + source_path).c_str());
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    // Extract all entries
    bool success = true;
    while (success) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r != ARCHIVE_OK) {
            dpm_log(LOG_ERROR, ("Archive read error: " + std::string(archive_error_string(a))).c_str());
            success = false;
            break;
        }

        // Modify entry pathname to extract to output directory
        std::string entry_path = archive_entry_pathname(entry);

        // Skip parent directory entries if present (directory with no '/' in path)
        if (entry_path.find('/') == std::string::npos &&
            archive_entry_filetype(entry) == AE_IFDIR) {
            continue;
        }

        // Remove parent directory from path if present
        size_t first_slash = entry_path.find('/');
        if (first_slash != std::string::npos) {
            entry_path = entry_path.substr(first_slash + 1);
        }

        // Skip empty paths after removing parent directory
        if (entry_path.empty()) {
            continue;
        }

        // Set the new path for this entry
        std::string full_path = (out_path / entry_path).string();
        archive_entry_set_pathname(entry, full_path.c_str());

        // Write the entry to disk
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            dpm_log(LOG_ERROR, ("Archive write error: " + std::string(archive_error_string(ext))).c_str());
            success = false;
            break;
        }

        // Copy the file data if it's a regular file
        if (archive_entry_size(entry) > 0) {
            const void* buff;
            size_t size;
            la_int64_t offset;

            while (true) {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) {
                    break;
                }
                if (r != ARCHIVE_OK) {
                    dpm_log(LOG_ERROR, ("Archive read data error: " + std::string(archive_error_string(a))).c_str());
                    success = false;
                    break;
                }

                r = archive_write_data_block(ext, buff, size, offset);
                if (r != ARCHIVE_OK) {
                    dpm_log(LOG_ERROR, ("Archive write data error: " + std::string(archive_error_string(ext))).c_str());
                    success = false;
                    break;
                }
            }

            if (!success) {
                break;
            }
        }

        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            dpm_log(LOG_ERROR, ("Archive finish entry error: " + std::string(archive_error_string(ext))).c_str());
            success = false;
            break;
        }
    }

    // Clean up
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    if (success) {
        dpm_log(LOG_INFO, ("Successfully extracted archive to: " + output_dir).c_str());
    }
    return success;
}

// compresses a directory component in a pacakge stage
bool smart_compress_component( const std::filesystem::path& stage_dir, const std::filesystem::path& component )
{
    std::filesystem::path component_path = stage_dir / component.string().c_str();

    // check if it's not a directory
    if ( ! std::filesystem::is_directory(component_path) )
    {
        // it's not a directory.
        // has it already been compressed?
        if ( file_already_compressed(component_path.string() ) )
        {
            // that component has already been compressed, so behave idempotently
            dpm_log(LOG_INFO, ( component_path.string() + " is already compressed, nothing to do." ).c_str() );
            return true;
        } else {
            // it's not a directory and it's not a compressed archive, so bail
            dpm_log(LOG_ERROR, ("Component is not a directory and not a compressed archive: " + component_path.string() ).c_str() );
            return false;
        }
    } else {
        // it's a directory so compress it
        dpm_log(LOG_INFO, ("Compressing directory: " + component_path.string()).c_str());
        bool result = compress_directory( component_path, component_path.string() + ".tmp" );
        if ( ! result ) {
            dpm_log( LOG_ERROR, ("Failed to compress component directory: " + component_path.string() ).c_str() );
            return false;
        }
    }
    // clean up the evidence
    try {
        std::filesystem::remove_all(component_path);
        std::filesystem::rename( component_path.string() + ".tmp", component_path.string() );
    }
    catch ( const std::exception& e ) {
        dpm_log(LOG_FATAL, ("Error placing new archive: " + std::string(e.what())).c_str());
        std::filesystem::remove( component_path.string() + ".tmp" );
        return false;
    }
    dpm_log( LOG_INFO, ( "Successfully created archive at: " + component_path.string() ).c_str() ); ;
    return true;
}


extern "C" int seal_stage_components(const std::string& stage_dir, bool force)
{
    dpm_con(LOG_INFO, ("Sealing package stage: " + stage_dir).c_str());

    // First refresh the metadata to ensure it's up-to-date
    dpm_con(LOG_INFO, "Refreshing metadata before sealing...");
    bool metadata_refresh_result = metadata_refresh_dynamic_files(stage_dir);
    if (!metadata_refresh_result) {
        dpm_con(LOG_ERROR, "Failed to refresh metadata files before sealing. Aborting.");
        return 1;
    }

    // Verify the stage directory structure
    std::filesystem::path stage_path(stage_dir);

    if (!smart_compress_component(stage_dir, "contents")) {
        dpm_con(LOG_FATAL, ("Failed to compress contents: " + stage_dir).c_str());
        return 1;
    }

    if (!smart_compress_component(stage_dir, "hooks")) {
        dpm_con(LOG_FATAL, ("Failed to compress hooks: " + stage_dir).c_str());
        return 1;
    }

    if (!smart_compress_component(stage_dir, "metadata")) {
        dpm_con(LOG_FATAL, ("Failed to compress metadata: " + stage_dir).c_str());
        return 1;
    }

    // Handle signatures component - check if it's an empty directory
    if (std::filesystem::is_directory(stage_path / "signatures")) {
        bool signatures_empty = true;

        // Check if signatures directory is empty
        for (const auto& entry : std::filesystem::directory_iterator(stage_path / "signatures")) {
            signatures_empty = false;
            break;
        }

        if (signatures_empty) {
            dpm_con(LOG_INFO, "Signatures directory is empty, not compressing.");
        } else {
            dpm_con(LOG_INFO, "Compressing signatures component.");
            if (!smart_compress_component(stage_dir, "signatures")) {
                dpm_con(LOG_FATAL, ("Failed to compress signatures: " + stage_dir).c_str());
                return 1;
            }
        }
    }

    dpm_con(LOG_INFO, "Package stage sealed successfully.");
    return 0;
}

extern "C" int seal_final_package(const std::string &stage_dir, const std::string &output_dir, bool force)
{
    int stage_seal_result = seal_stage_components( stage_dir, force );
    if ( stage_seal_result != 0 ) {
        dpm_log( LOG_FATAL, "Component sealing stage failed.  Exiting." );
        return 1;
    }

    std::filesystem::path stage_path( stage_dir );

    if ( ! std::filesystem::is_directory( stage_path ) ) {
        dpm_log( LOG_FATAL, "Stage is not a directory.  Refusing to continue.");
        return 1;
    }

    std::filesystem::path output_path;
    if ( output_dir.empty() ) {
        // the user didn't supply an output directory, so put the dpm next to the stage
        output_path = stage_path.string() + ".dpm";
    } else {
        // the user supplied an output directory so call it stage_name.dpm and prefix the path
        // with the output dir
        std::string stage_basename = stage_path.filename().string();

        // it's here
        output_path = std::filesystem::path(output_dir) / std::filesystem::path(stage_basename + ".dpm");
    }

    dpm_log( LOG_INFO, "Sealing DPM Package." );
    bool result = compress_directory( stage_path.string(), output_path.string() );
    if ( ! result ) {
        dpm_log( LOG_FATAL, "Could not create DPM package from stage." );
        return 1;
    }
    dpm_log( LOG_INFO, ("Package written to: " + output_path.string() ).c_str() );
    return 0;
}

extern "C" int unseal_package(const std::string& package_filepath, const std::string& output_dir, bool force)
{
    dpm_log(LOG_INFO, ("Unsealing package: " + package_filepath).c_str());

    // Extract filename from package path
    std::filesystem::path supplied_package_path(package_filepath);
    std::string package_filename = supplied_package_path.filename().string();

    // Verify it has .dpm extension
    const std::string dpm_extension = ".dpm";
    if (!package_filename.ends_with(dpm_extension)) {
        dpm_log(LOG_FATAL, "Refusing to unseal package: file must have .dpm extension");
        return 1;
    }

    // Remove .dpm extension to establish the tatget stage directory to extract to
    std::string target_stage_name = package_filename.substr(0, package_filename.length() - dpm_extension.length());

    // Determine the output directory path based on whether output_dir was supplied or not
    std::filesystem::path output_directory;

    if (output_dir.empty()) {
        // output_dir was not supplied, so set it to the supplied package path's parent dir + target_stage_name

        // Set output path to parent_directory/filename_without_extension
        output_directory = supplied_package_path.parent_path() / target_stage_name;
    } else {
        // output_dir was supplied, so use that

        // Use the provided output directory
        output_directory = std::filesystem::path(output_dir) / target_stage_name;
    }

    // Check if target path already exists
    if (std::filesystem::exists(output_directory)) {
        if (!force) {
            // a stage dir already exists with this name at that path so it can't be used unless --force is used
            // to overwrite it
            dpm_log(LOG_ERROR, ("Output directory already exists: " + output_directory.string() +
                ". Use --force to overwrite.").c_str());
            return 1;
        }

        // If force flag is set, remove the existing directory
        try {
            std::filesystem::remove_all(output_directory);
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to remove existing directory: " + std::string(e.what())).c_str());
            return 1;
        }
    }

    // Create the output directory
    try {
        std::filesystem::create_directories(output_directory);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to create output directory: " + std::string(e.what())).c_str());
        return 1;
    }

    // Extract the package to the output directory
    bool result = uncompress_archive(package_filepath, output_directory.string());
    if (!result) {
        dpm_log(LOG_ERROR, "Failed to extract package");
        return 1;
    }

    dpm_log(LOG_INFO, ("Package unsealed successfully to: " + output_directory.string()).c_str());
    return 0;
}

// Uncompress a package component if needed
bool smart_uncompress_component(const std::filesystem::path& stage_dir, const std::filesystem::path& component)
{
    std::filesystem::path component_path = stage_dir / component.string().c_str();

    // Check if component exists
    if (!std::filesystem::exists(component_path)) {
        dpm_log(LOG_ERROR, ("Component not found: " + component_path.string()).c_str());
        return false;
    }

    // Check if it's already a directory (already uncompressed)
    if (std::filesystem::is_directory(component_path)) {
        // Component is already a directory, so nothing to do
        dpm_log(LOG_INFO, (component_path.string() + " is already a directory, nothing to do.").c_str());
        return true;
    }

    // Create a temporary directory for extraction
    std::filesystem::path temp_dir = component_path.string() + ".tmp";

    // Clean up any existing temp directory
    if (std::filesystem::exists(temp_dir)) {
        try {
            std::filesystem::remove_all(temp_dir);
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to clean up existing temp directory: " + std::string(e.what())).c_str());
            return false;
        }
    }

    // Create the temp directory
    try {
        std::filesystem::create_directory(temp_dir);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to create temp directory: " + std::string(e.what())).c_str());
        return false;
    }

    // Extract the component to the temp directory
    dpm_log(LOG_INFO, ("Uncompressing component: " + component_path.string()).c_str());
    bool result = uncompress_archive(component_path.string(), temp_dir.string());
    if (!result) {
        dpm_log(LOG_ERROR, ("Failed to uncompress component: " + component_path.string()).c_str());
        std::filesystem::remove_all(temp_dir);
        return false;
    }

    // Remove the compressed file and rename the temp directory to take its place
    try {
        std::filesystem::remove(component_path);
        std::filesystem::rename(temp_dir, component_path);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_FATAL, ("Error replacing compressed component with uncompressed directory: " + std::string(e.what())).c_str());
        return false;
    }

    dpm_log(LOG_INFO, ("Successfully uncompressed component: " + component_path.string()).c_str());
    return true;
}

extern "C" int unseal_stage_components(const std::filesystem::path& stage_dir)
{
    dpm_log(LOG_INFO, ("Unsealing package components in: " + stage_dir.string()).c_str());

    // Verify the stage directory exists
    if (!std::filesystem::exists(stage_dir)) {
        dpm_log(LOG_ERROR, ("Stage directory does not exist: " + stage_dir.string()).c_str());
        return 1;
    }

    // Check if the required components exist (including signatures)
    std::vector<std::filesystem::path> components = {
        "contents",
        "metadata",
        "hooks",
        "signatures"
    };

    bool all_components_exist = true;
    for (const auto& component : components) {
        if (!std::filesystem::exists(stage_dir / component)) {
            dpm_log(LOG_ERROR, ("Missing required component: " + component.string()).c_str());
            all_components_exist = false;
        }
    }

    if (!all_components_exist) {
        dpm_log(LOG_FATAL, "Cannot unseal package: missing required components");
        return 1;
    }

    // Uncompress each component in order
    for (const auto& component : components) {
        if (!smart_uncompress_component(stage_dir, component)) {
            dpm_log(LOG_FATAL, ("Failed to uncompress component: " + component.string()).c_str());
            return 1;
        }
    }

    dpm_log(LOG_INFO, "Package components unsealed successfully");
    return 0;
}