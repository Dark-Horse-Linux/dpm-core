#include "error.hpp"

// Simple helper function that takes only the required error category
FlexDPMError make_error(DPMErrorCategory error_category)
{
    // Create an empty error struct
    FlexDPMError error;

    // Set the error category
    error.error = error_category;

    // Initialize the other fields to NULL
    error.module_name = NULL;
    error.module_path = NULL;
    error.message = NULL;

    // let the consumer populate any other fields they want with `self.field_name = whatever`.
    return error;
}
