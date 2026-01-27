/**
 * @file
 */
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
#include <SysLib.h>

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
    // #define DYNAMIC_EXPORT [[gnu::dllexport]]
    // #define DYNAMIC_IMPORT [[gnu::dllimport]]
    #define DYNAMIC_EXPORT __attribute__((visibility("default")))
    #define DYNAMIC_IMPORT __attribute__((visibility("default")))
#elif defined(_WIN32)
  #define DYNAMIC_EXPORT __declspec(dllexport)
  #define DYNAMIC_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
  #define DYNAMIC_EXPORT __attribute__((visibility("default")))
  #define DYNAMIC_IMPORT __attribute__((visibility("default")))
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

typedef uint32_t PetStatus;

#if defined(PET_NO_SIZED_ENUMS)
#define PetSuccess (0u)
#define PetNoMoreItems (1u)
#define PetEarlyWakeup (1u)
#define PetFail (0xC0000001u)
#define PetInvalidArg (0xC0000002u)
#define PetNotImplemented (0xC0000003u)
#else
typedef enum PetStatuses : uint32_t
{
    PetSuccess = 0,
    PetNoMoreItems = 1,
    PetEarlyWakeup = 2,
    PetFail = 0xC0000001,
    PetInvalidArg = 0xC0000002,
    PetNotImplemented = 0xC0000003,
} PetStatuses;
#endif

inline bool IsStatusSuccess(const PetStatus status)
{
    return status < 0x80000000;
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
 * Create the pet application.
 *
 *   This is simply the function for initializing the application's
 * pet controller class, this has nothing to do with individual pets.
 *
 *   The Pet application should use this function to allocate a state
 * object and store it into pOutPetHandle.
 *
 * @param pOutPetAppHandle A pointer to the handle for the application.
 *   The application should store a value sizeof(void*) that it will
 *   use to store its state.
 * @param pPetAICallbacks A pointer to a set of callbacks to call into
 *   the PetAI library. The application should store this object in
 *   its entirety.
 * @return A status code.
 */
typedef PetStatus CreatePetApp_f(PetAppHandle* pOutPetAppHandle, const PetAICallbacks* pPetAICallbacks);
/**
 * Destroy the pet application.
 *
 *   This is simply the function for destroying the application's pet
 * controller class, this has nothing to do with individual pets.
 *
 *   The pet application should use this function to free the state
 * object stored in petHandle. From this point no more functions will
 * be called in the PetFunctions object without a preceding call the
 * CreatePet.
 *
 * @param petAppHandle The state object the application should free.
 * @return A status code.
 */
typedef PetStatus DestroyPetApp_f(PetAppHandle petAppHandle);

/**
 * Specifies the file to load or save to.
 *
 *   This is essentially equivalent to a file path, the difference
 * being that it is on the pet application to decide where and how
 * the file is stored.
 */
typedef uint16_t PetFileHandle;

typedef PetStatus SavePetState_f(PetAppHandle petAppHandle, PetFileHandle file, size_t offset, const void* pData, const size_t size);
typedef PetStatus LoadPetState_f(PetAppHandle petAppHandle, PetFileHandle file, size_t offset, void* pData, size_t* pSize);

/**
 *   This is used to tell the application that Pet AI has no tasks to
 * do for a given period of time, and thus would like to sleep. It is
 * up to the application to decide what to do during this time.
 *
 *   The application does not need to sleep for the requested amount of
 * time, and it does not need to report how long it actually slept for,
 * though this might cause some slippage in how long a task was
 * supposed to take.
 * 
 * @param petAppHandle The state object of the application.
 * @param pSleepTime A pointer to how much time to sleep, and to store
 *   how much time was actually slept. 
 * @return A status code.
 */
typedef PetStatus Sleep_f(PetAppHandle petAppHandle, TimeMs_t* pSleepTime);
/**
 *   This is used to tell the application that Pet AI is giving up a
 * small amount of time for other processes to run. This can take as
 * little or as long as the application wants, though it should ideally
 * return control to Pet AI as soon as possible.
 *
 *   The application does not need to report how long it actually slept
 * for, though this might cause some slippage in how long a task was
 * supposed to take.
 * 
 * @param petAppHandle The state object of the application.
 * @param pSleepTime A to store how much time was slept. 
 * @return A status code.
 */
typedef PetStatus Yield_f(PetAppHandle petAppHandle, TimeMs_t* pSleepTime);

typedef PetStatus Update_f(PetAppHandle petAppHandle, float deltaTime);

typedef enum PetGender
{
    PetGenderNeuter = 0,
    PetGenderMale = 1,
    PetGenderFemale = 2
} PetGender;

typedef struct CreatePetData
{
    PetHandle ParentMale;
    PetHandle ParentFemale;
}CreatePetData;

typedef PetStatus CreatePet_f(PetAppHandle petAppHandle, const CreatePetData* pCreatePetData);

typedef enum PetEventType
{
    Unknown = 0,
    Birth = 1
} PetEventType;

typedef struct PetEventBirth
{
    PetHandle ParentMale;
    PetHandle ParentFemale;
    PetHandle ChildPet;
} PetEventBirth;

typedef union PetEventUnion
{
    PetEventBirth* Birth;
} PetEventTypes;

typedef struct PetEventData
{
    PetHandle PetHandle;
    const char* EventName;
    PetEventType EventType;
    PetEventUnion Event;
} PetEventData;

typedef PetStatus PetEvent_f(PetAppHandle petAppHandle, const PetEventData* pPetEventData);

typedef PetStatus NotifyExit_f(PetAIHandle petAIHandle);
typedef PetStatus EnumPets_f(PetAIHandle petAIHandle, PetHandle* pPetHandle, uint32_t index);

typedef PetStatus GetPetState_f(PetAIHandle petAIHandle, PetHandle petHandle, void** pState, uint32_t* pSize);

typedef struct CreatePetAIData
{
    PetHandle ParentMale;
    PetHandle ParentFemale;
    PetGender Gender;
    uint32_t StateSize;
    void* State;
} CreatePetAIData;

typedef PetStatus CreatePetAI_f(PetAIHandle petAIHandle, const CreatePetAIData* pCreatePetData, PetHandle* pPetHandle);

typedef struct PetAICallbacks
{
    PetAIHandle Handle;
    NotifyExit_f* NotifyExit;
    EnumPets_f* EnumPets;
    GetPetState_f* GetPetState;
    CreatePetAI_f* CreatePet;
} PetAICallbacks;

typedef struct PetFunctions
{
    uint32_t Version;

    CreatePetApp_f* CreatePetApp;
    DestroyPetApp_f* DestroyPetApp;

    SavePetState_f* SavePetState;
    LoadPetState_f* LoadPetState;

    Sleep_f* Sleep;
    Yield_f* Yield;

    Update_f* Update;

    CreatePet_f* CreatePet;
    PetEvent_f* PetEvent;
} PetFunctions;

PetStatus TAU_UTILS_LIB InitPetAI(const PetFunctions* const pFunctions);

void TAU_UTILS_LIB CleanupPetAI();

PetStatus TAU_UTILS_LIB RunPetAI();

#ifdef __cplusplus
}
#endif
