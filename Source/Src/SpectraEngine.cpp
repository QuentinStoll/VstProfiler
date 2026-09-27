#include "SpectraEngine.h"

#define SPECTRA_API
#include "spectra_dsp.h"

#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {
juce::String libraryFileName() {
#if JUCE_WINDOWS
    return "SpectraDsp.dll";
#elif JUCE_MAC
    return "libSpectraDsp.dylib";
#else
    return "libSpectraDsp.so";
#endif
}

juce::File thisModuleFile() {
#if JUCE_WINDOWS
    HMODULE module = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(&thisModuleFile),
                           &module) == 0) {
        return {};
    }

    wchar_t path[MAX_PATH] = {};
    const auto length = GetModuleFileNameW(module, path, MAX_PATH);
    if (length == 0) {
        return {};
    }
    return juce::File(juce::String(path));
#else
    Dl_info info{};
    if (dladdr(reinterpret_cast<const void*>(&thisModuleFile), &info) == 0 || info.dli_fname == nullptr) {
        return {};
    }
    return juce::File(juce::CharPointer_UTF8(info.dli_fname));
#endif
}

juce::File resolveLibraryFile(const juce::File& requested) {
    if (requested.existsAsFile()) {
        return requested;
    }

    const auto fromEnvironment = juce::SystemStats::getEnvironmentVariable("SPECTRA_DSP_PATH", {});
    if (fromEnvironment.isNotEmpty()) {
        const juce::File environmentFile(fromEnvironment);
        if (environmentFile.existsAsFile()) {
            return environmentFile;
        }
        const auto named = environmentFile.getChildFile(libraryFileName());
        if (named.existsAsFile()) {
            return named;
        }
    }

    if (requested.getFileName() == libraryFileName() && requested.getParentDirectory().isDirectory()) {
        return {};
    }

    const auto sibling = thisModuleFile().getSiblingFile(libraryFileName());
    if (sibling.existsAsFile()) {
        return sibling;
    }
    return {};
}
}  // namespace

struct SpectraEngine::Functions {
    int (*abiVersion)() = nullptr;
    int (*sessionUnlock)(const char*) = nullptr;
    void (*sessionLock)() = nullptr;
    int (*decryptModel)(const uint8_t*, size_t, SpectraBuffer*) = nullptr;
    int (*decryptIr)(const uint8_t*, size_t, SpectraBuffer*) = nullptr;
    void (*bufferFree)(SpectraBuffer*) = nullptr;
    int (*captureBegin)(int) = nullptr;
    int (*capturePush)(const float*, size_t) = nullptr;
    int (*captureFinish)() = nullptr;
};

SpectraEngine::SpectraEngine() = default;

SpectraEngine::~SpectraEngine() {
    close();
}

bool SpectraEngine::open(const juce::File& libraryFile) {
    if (isLoaded()) {
        return true;
    }

    const auto resolved = resolveLibraryFile(libraryFile);
    if (!resolved.existsAsFile()) {
        return false;
    }

    if (!_library.open(resolved.getFullPathName())) {
        return false;
    }

    auto functions = std::make_unique<Functions>();
    functions->abiVersion = reinterpret_cast<int (*)()>(_library.getFunction("spectra_abi_version"));
    functions->sessionUnlock = reinterpret_cast<int (*)(const char*)>(_library.getFunction("spectra_session_unlock"));
    functions->sessionLock = reinterpret_cast<void (*)()>(_library.getFunction("spectra_session_lock"));
    functions->decryptModel = reinterpret_cast<int (*)(const uint8_t*, size_t, SpectraBuffer*)>(
        _library.getFunction("spectra_decrypt_model"));
    functions->decryptIr = reinterpret_cast<int (*)(const uint8_t*, size_t, SpectraBuffer*)>(
        _library.getFunction("spectra_decrypt_ir"));
    functions->bufferFree = reinterpret_cast<void (*)(SpectraBuffer*)>(_library.getFunction("spectra_buffer_free"));
    functions->captureBegin = reinterpret_cast<int (*)(int)>(_library.getFunction("spectra_capture_begin"));
    functions->capturePush = reinterpret_cast<int (*)(const float*, size_t)>(_library.getFunction("spectra_capture_push"));
    functions->captureFinish = reinterpret_cast<int (*)()>(_library.getFunction("spectra_capture_finish"));

    const bool complete = functions->abiVersion != nullptr && functions->sessionUnlock != nullptr &&
                          functions->sessionLock != nullptr && functions->decryptModel != nullptr &&
                          functions->decryptIr != nullptr && functions->bufferFree != nullptr &&
                          functions->captureBegin != nullptr && functions->capturePush != nullptr &&
                          functions->captureFinish != nullptr;
    if (!complete || functions->abiVersion() != SPECTRA_ABI_VERSION) {
        close();
        return false;
    }

    _abiVersion = functions->abiVersion();
    _functions = std::move(functions);
    return true;
}

void SpectraEngine::close() {
    _functions.reset();
    _abiVersion = 0;
    _library.close();
}

bool SpectraEngine::isLoaded() const noexcept {
    return _functions != nullptr;
}

int SpectraEngine::abiVersion() const noexcept {
    return _abiVersion;
}

bool SpectraEngine::unlock(const juce::String& accessToken) {
    if (!isLoaded() || accessToken.isEmpty()) {
        return false;
    }
    return _functions->sessionUnlock(accessToken.toRawUTF8()) == SPECTRA_OK;
}

void SpectraEngine::lock() {
    if (isLoaded()) {
        _functions->sessionLock();
    }
}

namespace {
bool decryptWith(int (*function)(const uint8_t*, size_t, SpectraBuffer*),
                 void (*freeBuffer)(SpectraBuffer*),
                 const void* cipher,
                 size_t size,
                 juce::MemoryBlock& plain) {
    plain.reset();
    if (function == nullptr || cipher == nullptr || size == 0) {
        return false;
    }

    SpectraBuffer buffer{};
    const auto status = function(static_cast<const uint8_t*>(cipher), size, &buffer);
    if (status != SPECTRA_OK || buffer.data == nullptr || buffer.size == 0) {
        if (freeBuffer != nullptr) {
            freeBuffer(&buffer);
        }
        return false;
    }

    plain.replaceWith(buffer.data, buffer.size);
    if (freeBuffer != nullptr) {
        freeBuffer(&buffer);
    }
    return plain.getSize() > 0;
}
}  // namespace

bool SpectraEngine::decryptModel(const void* cipher, size_t size, juce::MemoryBlock& plain) const {
    if (!isLoaded()) {
        return false;
    }
    return decryptWith(_functions->decryptModel, _functions->bufferFree, cipher, size, plain);
}

bool SpectraEngine::decryptIr(const void* cipher, size_t size, juce::MemoryBlock& plain) const {
    if (!isLoaded()) {
        return false;
    }
    return decryptWith(_functions->decryptIr, _functions->bufferFree, cipher, size, plain);
}
