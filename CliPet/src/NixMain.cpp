#ifndef _WIN32
#include <PetAI.h>
#include <new>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <atomic>
#include <signal.h>
#include <unistd.h>

class NixCliPet final
{
public:
    static NixCliPet* FromHandle(const PetAppHandle handle) noexcept
    {
        return static_cast<NixCliPet*>(handle.Ptr);
    }
public:
    NixCliPet(const PetAICallbacks* const pPetAICallbacks) noexcept;

    ~NixCliPet() noexcept = default;

    NixCliPet(const NixCliPet& copy) noexcept = delete;
    NixCliPet(NixCliPet&& move) noexcept = delete;

    NixCliPet& operator=(const NixCliPet& copy) noexcept = delete;
    NixCliPet& operator=(NixCliPet&& move) noexcept = delete;

    PetStatus SavePetState(PetFileHandle file, const size_t offset, const void* pData, const size_t size) noexcept;
    PetStatus LoadPetState(PetFileHandle file, const size_t offset, void* pData, size_t* pSize) noexcept;

    PetStatus Sleep(TimeMs_t* const pSleepTime) noexcept;
    PetStatus Yield(TimeMs_t* const pSleepTime) noexcept;

    PetStatus Update(const float deltaTime) noexcept;

    PetStatus CreatePet(const CreatePetData* const pCreatePetData) noexcept;
private:
    PetAICallbacks m_Callbacks;  // NOLINT(clang-diagnostic-unused-private-field)
};

static PetStatus CreatePetApp(PetAppHandle* const pOutPetAppHandle, const PetAICallbacks* const pPetAICallbacks);
static PetStatus DestroyPetApp(const PetAppHandle petAppHandle);

static PetStatus SavePetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, const void* const pData, const size_t size);
static PetStatus LoadPetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize);

static PetStatus Sleep(const PetAppHandle petAppHandle, TimeMs_t* const pSleepTime);
static PetStatus Yield(const PetAppHandle petAppHandle, TimeMs_t* const pSleepTime);

static PetStatus Update(const PetAppHandle petAppHandle, const float deltaTime);

static PetStatus CreatePet(const PetAppHandle petAppHandle, const CreatePetData* const pCreatePetData);

static ::std::atomic_bool s_ShouldExit(false);

static void SignalHandler(const int signal) noexcept;

int main(int argCount, char* args[])
{
    (void) argCount;
    (void) args;

    PetFunctions petFunctions {};
    petFunctions.Version = PET_AI_VERSION;
    petFunctions.CreatePetApp = CreatePetApp;
    petFunctions.DestroyPetApp = DestroyPetApp;
    petFunctions.SavePetState = SavePetState;
    petFunctions.LoadPetState = LoadPetState;
    petFunctions.Sleep = Sleep;
    petFunctions.Yield = Yield;
    petFunctions.Update = Update;
    petFunctions.CreatePet = CreatePet;

    PetStatus status = InitPetAI(&petFunctions);

    if(!IsStatusSuccess(status))
    {
        return -1;
    }

    struct sigaction sigIntHandler { };
    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    (void) sigaction(SIGINT, &sigIntHandler, nullptr);
    (void) sigaction(SIGTERM, &sigIntHandler, nullptr);

    status = RunPetAI();

    CleanupPetAI();

    if(!IsStatusSuccess(status))
    {
        return -2;
    }

    return 0;
}

NixCliPet::NixCliPet(const PetAICallbacks* const pPetAICallbacks) noexcept
    : m_Callbacks(*pPetAICallbacks)
{ }

PetStatus NixCliPet::SavePetState(const PetFileHandle file, const size_t offset, const void* const pData, const size_t size) noexcept
{
    if(!pData)
    {
        return PetInvalidArg;
    }

    char nameBuffer[6];
    (void) ::std::snprintf(nameBuffer, sizeof(nameBuffer), "%d", file);

    FILE* cFile = fopen(nameBuffer, "wb+");
    if(!cFile)
    {
        return PetFail;
    }

    if(!fseek(cFile, static_cast<long>(offset), SEEK_SET))
    {
        return PetFail;
    }

    (void) fwrite(pData, 1, size, cFile);

    (void) fclose(cFile);

    return PetSuccess;
}

PetStatus NixCliPet::LoadPetState(const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize) noexcept
{
    char nameBuffer[6];
    (void) ::std::snprintf(nameBuffer, sizeof(nameBuffer), "%d", file);

    FILE* cFile = fopen(nameBuffer, "wb+");
    if(!cFile)
    {
        *pSize = 0;
        return PetFail;
    }

    if(!fseek(cFile, static_cast<long>(offset), SEEK_SET))
    {
        *pSize = 0;
        return PetFail;
    }

    if(!pData)
    {
        const int64_t currentPos = static_cast<int64_t>(ftell(cFile));
        if(!fseek(cFile, 0, SEEK_END))
        {
            *pSize = 0;
            return PetFail;
        }
        const int64_t endPos = static_cast<int64_t>(ftell(cFile));

        *pSize = static_cast<size_t>(endPos - currentPos);
    }
    else
    {
        *pSize = fread(pData, *pSize, 1, cFile);
    }

    (void) fclose(cFile);

    return PetSuccess;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
PetStatus NixCliPet::Sleep(TimeMs_t* const pSleepTime) noexcept
{
    if(!pSleepTime)
    {
        return PetInvalidArg;
    }

    const TimeMs_t sleepTime = *pSleepTime;

    if(sleepTime <= 0)
    {
        return PetInvalidArg;
    }

    ::std::this_thread::sleep_for(::std::chrono::milliseconds(sleepTime));

    return PetSuccess;
}

PetStatus NixCliPet::Yield(TimeMs_t* const pSleepTime) noexcept
{
    ::std::this_thread::yield();

    if(pSleepTime)
    {
        *pSleepTime = 0;
    }

    return PetSuccess;
}

PetStatus NixCliPet::Update(const float deltaTime) noexcept
{
    (void) deltaTime;

    if(s_ShouldExit)
    {
        (void) m_Callbacks.NotifyExit(m_Callbacks.Handle);
    }

    return PetSuccess;
}

PetStatus NixCliPet::CreatePet(const CreatePetData* const pCreatePetData) noexcept
{
    CreatePetAIData createData {};
    createData.ParentMale = pCreatePetData->ParentMale;
    createData.ParentFemale = pCreatePetData->ParentFemale;
    createData.Gender = PetGenderNeuter;
    createData.State = nullptr;
    createData.StateSize = 0;

    return m_Callbacks.CreatePet(m_Callbacks.Handle, &createData, nullptr);
}

static PetStatus CreatePetApp(PetAppHandle* const pOutPetAppHandle, const PetAICallbacks* const pPetAICallbacks)
{
    if(!pOutPetAppHandle)
    {
        return PetInvalidArg;
    }

    if(!pPetAICallbacks)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = new(::std::nothrow) NixCliPet(pPetAICallbacks);

    pOutPetAppHandle->Ptr = pet;

    return PetSuccess;
}

static PetStatus DestroyPetApp(const PetAppHandle petAppHandle)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    delete pet;

    return PetSuccess;
}

static PetStatus SavePetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, const void* const pData, const size_t size)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    return pet->SavePetState(file, offset, pData, size);
}

static PetStatus LoadPetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    return pet->LoadPetState(file, offset, pData, pSize);
}

static PetStatus Sleep(const PetAppHandle petAppHandle, TimeMs_t* const pSleepTime)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    return pet->Sleep(pSleepTime);
}

static PetStatus Yield(const PetAppHandle petAppHandle, TimeMs_t* const pSleepTime)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    return pet->Yield(pSleepTime);
}

static PetStatus Update(const PetAppHandle petAppHandle, const float deltaTime)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    return pet->Update(deltaTime);
}

PetStatus CreatePet(const PetAppHandle petAppHandle, const CreatePetData* const pCreatePetData)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    NixCliPet* const pet = NixCliPet::FromHandle(petAppHandle);

    return pet->CreatePet(pCreatePetData);
}

static void SignalHandler(const int signal) noexcept
{
    switch(signal)
    {
        case SIGINT:
        case SIGTERM:
            s_ShouldExit = true;
            return;
        default: return;
    }
}
#endif
