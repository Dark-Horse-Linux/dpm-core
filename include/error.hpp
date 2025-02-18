#pragma once

enum class DPMError {
    SUCCESS,
    PATH_NOT_FOUND,
    PATH_NOT_DIRECTORY,
    PERMISSION_DENIED,
    MODULE_NOT_FOUND,
    MODULE_LOAD_FAILED,
    INVALID_MODULE
};