#include "edc.hpp"

#include <dlfcn.h>

#include <xbt/asserts.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(edc, "edc"); //!< Logging

/**
 * @brief Build an ExternalLibrary from a library path.
 * @param[in] lib_path The path of the library to load as an External Decision Component.
 */
ExternalLibrary::ExternalLibrary(const std::string & lib_path, const EdcLibraryLoadMethod & load_method)
{
    switch (load_method)
    {
    case EdcLibraryLoadMethod::DLMOPEN: {
        // dlmopen(LM_ID_NEWLM) places the library in a new memory namespace just for it.
        // this makes sure that the library and all their dependencies are:
        // - loaded into memory, which would not be done if similar libraries existed in the default (batsim's) namespace.
        // - loaded from the desired path/at the desired version if specified in the loaded ELF (e.g., via DT_RUNPATH).
        // - privatized, that is to say that their global variables are not shared between different components.
        lib_handle = dlmopen(LM_ID_NEWLM, lib_path.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    } break;
    case EdcLibraryLoadMethod::DLOPEN: {
        // dlopen places the library in the default memory namespace.
        // - this may have collision with Batsim's memory (e.g., batprotocol-cpp)
        // - this is strongly discouraged if several EDCs should be loaded
        lib_handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    } break;
    }

    xbt_assert(lib_handle != NULL, "dlmopen failed while loading external decision component library: %s", dlerror());

    init = (uint8_t (*)(const uint8_t*, uint8_t, uint8_t)) load_lib_symbol(lib_handle, "batsim_edc_init");
    deinit = (uint8_t (*)()) load_lib_symbol(lib_handle, "batsim_edc_deinit");
    take_decisions = (uint8_t (*)(const uint8_t*, uint8_t**)) load_lib_symbol(lib_handle, "batsim_edc_take_decisions");

    XBT_INFO("loaded external decision component library from '%s'", lib_path.c_str());
}

/**
 * @brief Deallocate an ExternalLibrary.
 */
ExternalLibrary::~ExternalLibrary()
{
    deinit();
    dlclose(lib_handle);
}

/**
 * @brief Load a symbol from a library handle.
 * @details Just a wrapper around dlsym.
 * @param[in] lib_handle The library handle where the symbol should be loaded
 * @param[in] symbol The symbol to load
 * @return The address of the symbol loaded
 */
void * load_lib_symbol(void * lib_handle, const char * symbol)
{
    void * address = dlsym(lib_handle, symbol);
    xbt_assert(address != NULL, "Could not load symbol '%s': %s", symbol, dlerror());
    return address;
}
