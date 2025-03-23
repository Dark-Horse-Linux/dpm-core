#include "signing.hpp"

/**
 * @brief Signs a component archive using GPGME
 *
 * Creates a detached GPG signature for a component archive
 *
 * @param stage_path Path to the stage directory
 * @param key_id GPG key ID or email to use for signing
 * @param component_name Name of the component to sign (contents, hooks, metadata)
 * @return 0 on success, non-zero on failure
 */
static int sign_component(const std::filesystem::path& stage_path, const std::string& key_id,
                          const std::string& component_name) {
    dpm_log(LOG_INFO, ("Signing " + component_name + " component...").c_str());

    std::filesystem::path component_path = stage_path / component_name;
    std::filesystem::path signature_path = stage_path / "signatures" / (component_name + ".signature");

    // Initialize GPGME
    gpgme_ctx_t ctx;
    gpgme_error_t err;

    // Initialize GPGME library
    const char* version = gpgme_check_version(NULL);
    if (version == NULL) {
        dpm_log(LOG_ERROR, "Failed to initialize GPGME library");
        return 1;
    }

    // Create a new GPGME context
    err = gpgme_new(&ctx);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, "Failed to create GPGME context");
        return 1;
    }

    // Set protocol to OpenPGP
    err = gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, "Failed to set GPGME protocol");
        gpgme_release(ctx);
        return 1;
    }

    // Set armor mode (for ASCII-armored output)
    gpgme_set_armor(ctx, 1);

    // Set signing key
    gpgme_key_t key;
    err = gpgme_get_key(ctx, key_id.c_str(), &key, 1);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, ("Failed to find signing key: " + key_id).c_str());
        gpgme_release(ctx);
        return 1;
    }

    // Add the key to the context
    gpgme_signers_clear(ctx);
    err = gpgme_signers_add(ctx, key);
    gpgme_key_unref(key);

    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, "Failed to add signing key to context");
        gpgme_release(ctx);
        return 1;
    }

    // Open the component file
    FILE* component_file = fopen(component_path.string().c_str(), "rb");
    if (!component_file) {
        dpm_log(LOG_ERROR, ("Failed to open component file: " + component_path.string()).c_str());
        gpgme_release(ctx);
        return 1;
    }

    // Open the signature file
    FILE* signature_file = fopen(signature_path.string().c_str(), "wb");
    if (!signature_file) {
        dpm_log(LOG_ERROR, ("Failed to create signature file: " + signature_path.string()).c_str());
        fclose(component_file);
        gpgme_release(ctx);
        return 1;
    }

    // Create data objects for input and output
    gpgme_data_t in_data, out_data;
    err = gpgme_data_new_from_stream(&in_data, component_file);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, "Failed to create input data object");
        fclose(component_file);
        fclose(signature_file);
        gpgme_release(ctx);
        return 1;
    }

    err = gpgme_data_new_from_stream(&out_data, signature_file);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, "Failed to create output data object");
        gpgme_data_release(in_data);
        fclose(component_file);
        fclose(signature_file);
        gpgme_release(ctx);
        return 1;
    }

    // Sign the data
    err = gpgme_op_sign(ctx, in_data, out_data, GPGME_SIG_MODE_DETACH);
    if (gpgme_err_code(err) != GPG_ERR_NO_ERROR) {
        dpm_log(LOG_ERROR, ("Failed to sign component: " + std::string(gpgme_strerror(err))).c_str());
        gpgme_data_release(in_data);
        gpgme_data_release(out_data);
        fclose(component_file);
        fclose(signature_file);
        gpgme_release(ctx);
        return 1;
    }

    // Clean up
    gpgme_data_release(in_data);
    gpgme_data_release(out_data);
    fclose(component_file);
    fclose(signature_file);
    gpgme_release(ctx);

    return 0;
}

int sign_stage_directory(const std::string& stage_dir, const std::string& key_id, bool force) {
    dpm_log(LOG_INFO, ("Signing package stage: " + stage_dir).c_str());

    // Verify the stage directory structure
    std::filesystem::path stage_path(stage_dir);
    std::filesystem::path contents_path = stage_path / "contents";
    std::filesystem::path hooks_path = stage_path / "hooks";
    std::filesystem::path metadata_path = stage_path / "metadata";
    std::filesystem::path signatures_path = stage_path / "signatures";

    // Check if required directories exist
    if (!std::filesystem::exists(contents_path)) {
        dpm_log(LOG_ERROR, ("Invalid stage directory: contents not found in " + stage_dir).c_str());
        return 1;
    }

    if (!std::filesystem::exists(hooks_path)) {
        dpm_log(LOG_ERROR, ("Invalid stage directory: hooks not found in " + stage_dir).c_str());
        return 1;
    }

    if (!std::filesystem::exists(metadata_path)) {
        dpm_log(LOG_ERROR, ("Invalid stage directory: metadata not found in " + stage_dir).c_str());
        return 1;
    }

    // Create signatures directory if it doesn't exist
    if (!std::filesystem::exists(signatures_path)) {
        dpm_log(LOG_INFO, ("Creating signatures directory in " + stage_dir).c_str());
        try {
            std::filesystem::create_directory(signatures_path);
        } catch (const std::filesystem::filesystem_error& e) {
            dpm_log(LOG_ERROR, ("Failed to create signatures directory: " + std::string(e.what())).c_str());
            return 1;
        }
    }

    // Sign each component
    int result = 0;

    // Sign contents
    if (sign_component(stage_path, key_id, "contents") != 0) {
        dpm_log(LOG_ERROR, "Failed to sign contents component");
        result = 1;
    }

    // Sign hooks
    if (sign_component(stage_path, key_id, "hooks") != 0) {
        dpm_log(LOG_ERROR, "Failed to sign hooks component");
        result = 1;
    }

    // Sign metadata
    if (sign_component(stage_path, key_id, "metadata") != 0) {
        dpm_log(LOG_ERROR, "Failed to sign metadata component");
        result = 1;
    }

    if (result == 0) {
        dpm_log(LOG_INFO, "Package stage signed successfully.");
    }

    return result;
}

int sign_package_file(const std::string& package_path, const std::string& key_id, bool force) {
    dpm_log(LOG_INFO, ("Signing package file: " + package_path).c_str());

    // Get the temporary stage path by removing .dpm extension
    std::string tmp_stage_path = package_path;
    if (tmp_stage_path.ends_with(".dpm")) {
        tmp_stage_path = tmp_stage_path.substr(0, tmp_stage_path.length() - 4);
    }

    // Check if temporary stage path already exists - fail if it does
    if (std::filesystem::exists(tmp_stage_path)) {
        dpm_log(LOG_ERROR, ("Temporary stage directory already exists: " + tmp_stage_path).c_str());
        return 1;
    }

    // 1. Unseal the package to the stage parent path
    std::filesystem::path stage_parent_path = std::filesystem::path(tmp_stage_path).parent_path();
    dpm_log(LOG_INFO, "Unsealing package file...");
    int result = unseal_package(package_path, stage_parent_path, force);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to unseal package file");
        return result;
    }

    // 2. Sign the stage directory components
    dpm_log(LOG_INFO, "Signing package components...");
    result = sign_stage_directory(tmp_stage_path, key_id, force);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to sign package components");
        return result;
    }

    std::string backup_path = package_path + ".old";

    // Back up the original package
    try {
        std::filesystem::rename(package_path, backup_path);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_ERROR, ("Failed to backup original package: " + std::string(e.what())).c_str());
        return 1;
    }

    // 3. Create a new sealed package at the original location
    dpm_log(LOG_INFO, "Creating signed package file...");

    // seal the package path
    result = seal_final_package(tmp_stage_path, stage_parent_path.string(), force);
    if (result != 0) {
        dpm_log(LOG_ERROR, "Failed to create signed package");
        return result;
    }

    // 4. Clean up
    try {
        std::filesystem::remove_all(tmp_stage_path);
        std::filesystem::remove(backup_path);
    } catch (const std::filesystem::filesystem_error& e) {
        dpm_log(LOG_WARN, ("Failed to clean up temporary files: " + std::string(e.what())).c_str());
    }

    dpm_log(LOG_INFO, ("Successfully signed package: " + package_path).c_str());
    return 0;
}