#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ReSharper disable once CppUnusedIncludeDirective
#include <stdbool.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <stdint.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <stdlib.h>

#ifdef __cplusplus
  #ifdef _MSVC_LANG
    #define CPP_VERSION_DATE _MSVC_LANG
  #else 
    #define CPP_VERSION_DATE __cplusplus
  #endif
#elif defined(_MSVC_LANG)
  #define CPP_VERSION_DATE _MSVC_LANG
#else
  #define CPP_VERSION_DATE 1
#endif

#if defined(__clang__) && CPP_VERSION_DATE >= 201103L
    #define DYNAMIC_EXPORT [[gnu::dllexport]]
    #define DYNAMIC_IMPORT [[gnu::dllimport]]
#elif defined(_WIN32)
  #define DYNAMIC_EXPORT __declspec(dllexport)
  #define DYNAMIC_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
  #define DYNAMIC_EXPORT __attribute__((visibility("default")))
  #define DYNAMIC_IMPORT 
#else
  #define DYNAMIC_EXPORT
  #define DYNAMIC_IMPORT
#endif

#if defined(PET_AI_BUILD_SHARED)
  #define TAU_UTILS_LIB DYNAMIC_EXPORT
#elif defined(PET_AI_BUILD_STATIC)
  #define TAU_UTILS_LIB
#elif defined(PET_AI_IMPORT_SHARED)
  #define TAU_UTILS_LIB DYNAMIC_IMPORT
#else
  #define TAU_UTILS_LIB
#endif

typedef int PetStatus;

typedef enum PetStatuses : uint32_t
{
    PetSuccess = 0,
    NoMoreItems = 1,
    PetFail = 0xC0000001,
    PetInvalidArg = 0xC0000002,
    PetNotImplemented = 0xC0000003,
} PetStatuses;

inline bool IsStatusSuccess(const PetStatus status)
{
    return status >= 0;
}

inline bool IsStatusError(const PetStatus status)
{
    return !IsStatusSuccess(status);
}

#define PET_AI_VERSION_1_0 10
#define PET_AI_VERSION PET_AI_VERSION_1_0

typedef struct PetAIHandle {
    // ReSharper disable once CppInconsistentNaming
    void* Ptr;
} PetAIHandle;

typedef struct PetAppHandle {
    void* Ptr;
} PetAppHandle;

typedef struct PetHandle {
    void* Ptr;
} PetHandle;

struct PetAICallbacks;

/**
 * Create the pet implementation.
 *
 *   This is simply the function for initializing the implementation's
 * pet controller class, this has nothing to do with individual pets.
 *
 *   The Pet implementation should use this function to allocate a
 * state object and store it into pOutPetHandle.
 *
 * @param pOutPetAppHandle A pointer to the handle for the
 *   implementation. The implementation should store a value
 *   sizeof(void*) that it will use to store its state.
 * @param pPetAICallbacks A pointer to a set of callbacks to call into
 *   the PetAI library. The implementation should store this object in
 *   its entirety.
 * @return A status code.
 */
typedef PetStatus CreatePetApp_f(PetAppHandle* pOutPetAppHandle, const PetAICallbacks* pPetAICallbacks);
/**
 * Destroy the pet implementation.
 *
 *   This is simply the function for destroying the implementation's
 * pet controller class, this has nothing to do with individual pets.
 *
 *   The pet implementation should use this function to free the state
 * object stored in petHandle. From this point no more functions will
 * be called in the PetFunctions object without a preceding call the
 * CreatePet.
 *
 * @param petAppHandle The state object the implementation should free.
 * @return A status code.
 */
typedef PetStatus DestroyPetApp_f(PetAppHandle petAppHandle);

/**
 * Specifies the file to load or save to.
 *
 *   This is essentially equivalent to a file path, the difference
 * being that it is on the pet implementation to decide where and how
 * the file is stored.
 */
typedef uint16_t PetFileHandle;

typedef PetStatus SavePetState_f(PetAppHandle petAppHandle, PetFileHandle file, size_t offset, const void* pData, const size_t size);
typedef PetStatus LoadPetState_f(PetAppHandle petAppHandle, PetFileHandle file, size_t offset, void* pData, size_t* pSize);

typedef PetStatus CreatePet_f(PetAppHandle petAppHandle);

typedef PetStatus EnumPets_f(PetAIHandle petAIHandle, PetHandle* pPetHandle);

typedef struct PetAICallbacks
{
    PetAIHandle Handle;
    EnumPets_f* EnumPets;
} PetAICallbacks;

typedef struct PetFunctions
{
    uint32_t Version;

    CreatePetApp_f* CreatePetApp;
    DestroyPetApp_f* DestroyPetApp;

    SavePetState_f* SavePetState;
    LoadPetState_f* LoadPetState;
} PetFunctions;

PetStatus TAU_UTILS_LIB InitPetAI(const PetFunctions* const pFunctions);

PetStatus TAU_UTILS_LIB RunPetAI();

#ifdef __cplusplus
}
#endif
