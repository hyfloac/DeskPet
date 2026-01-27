#ifdef _WIN32
#include <PetAI.h>
#include <new>
#include <cstdlib>
#include <cstdio>
#include <thread>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef Yield
#include <atomic>

class Win32CliPet final
{
public:
    static Win32CliPet* FromHandle(const PetAppHandle handle) noexcept
    {
        return static_cast<Win32CliPet*>(handle.Ptr);
    }
public:
    Win32CliPet(const PetAICallbacks* const pPetAICallbacks) noexcept;

    ~Win32CliPet() noexcept = default;

    Win32CliPet(const Win32CliPet& copy) noexcept = delete;
    Win32CliPet(Win32CliPet&& move) noexcept = delete;

    Win32CliPet& operator=(const Win32CliPet& copy) noexcept = delete;
    Win32CliPet& operator=(Win32CliPet&& move) noexcept = delete;

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

static BOOL WINAPI CtrlHandler(const DWORD fdwCtrlType) noexcept;

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

    if(!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        CleanupPetAI();
        return -3;
    }

    status = RunPetAI();

    CleanupPetAI();

    if(!IsStatusSuccess(status))
    {
        return -2;
    }

    return 0;
}

Win32CliPet::Win32CliPet(const PetAICallbacks* const pPetAICallbacks) noexcept
    : m_Callbacks(*pPetAICallbacks)
{ }

PetStatus Win32CliPet::SavePetState(const PetFileHandle file, const size_t offset, const void* const pData, const size_t size) noexcept
{
    if(!pData)
    {
        return PetInvalidArg;
    }

    char nameBuffer[6];
    (void) _itoa(file, nameBuffer, 10);

    FILE* cFile;
    errno_t err = fopen_s(&cFile, nameBuffer, "wb+");
    if(!err)
    {
        return PetFail;
    }

    if(!_fseeki64(cFile, static_cast<int64_t>(offset), SEEK_SET))
    {
        return PetFail;
    }

    (void) fwrite(pData, 1, size, cFile);

    (void) fclose(cFile);

    return PetSuccess;
}

PetStatus Win32CliPet::LoadPetState(const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize) noexcept
{
    char nameBuffer[6];
    (void) _itoa(file, nameBuffer, 10);

    FILE* cFile;
    errno_t err = fopen_s(&cFile, nameBuffer, "wb+");
    if(!err)
    {
        *pSize = 0;
        return PetFail;
    }

    if(!_fseeki64(cFile, static_cast<int64_t>(offset), SEEK_SET))
    {
        *pSize = 0;
        return PetFail;
    }

    if(!pData)
    {
        const int64_t currentPos = _ftelli64(cFile);
        if(!_fseeki64(cFile, 0, SEEK_END))
        {
            *pSize = 0;
            return PetFail;
        }
        const int64_t endPos = _ftelli64(cFile);

        *pSize = static_cast<size_t>(endPos - currentPos);
    }
    else
    {
        *pSize = fread_s(pData, *pSize, 1, *pSize, cFile);
    }

    (void) fclose(cFile);

    return PetSuccess;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
PetStatus Win32CliPet::Sleep(TimeMs_t* const pSleepTime) noexcept
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

PetStatus Win32CliPet::Yield(TimeMs_t* const pSleepTime) noexcept
{
    ::std::this_thread::yield();

    if(pSleepTime)
    {
        *pSleepTime = 0;
    }

    return PetSuccess;
}

PetStatus Win32CliPet::Update(const float deltaTime) noexcept
{
    (void) deltaTime;

    if(s_ShouldExit)
    {
        (void) m_Callbacks.NotifyExit(m_Callbacks.Handle);
    }

    return PetSuccess;
}

PetStatus Win32CliPet::CreatePet(const CreatePetData* const pCreatePetData) noexcept
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

    Win32CliPet* const pet = new(::std::nothrow) Win32CliPet(pPetAICallbacks);

    pOutPetAppHandle->Ptr = pet;

    return PetSuccess;
}

static PetStatus DestroyPetApp(const PetAppHandle petAppHandle)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    delete pet;

    return PetSuccess;
}

static PetStatus SavePetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, const void* const pData, const size_t size)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    return pet->SavePetState(file, offset, pData, size);
}

static PetStatus LoadPetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    return pet->LoadPetState(file, offset, pData, pSize);
}

static PetStatus Sleep(const PetAppHandle petAppHandle, TimeMs_t* const pSleepTime)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    return pet->Sleep(pSleepTime);
}

static PetStatus Yield(const PetAppHandle petAppHandle, TimeMs_t* const pSleepTime)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    return pet->Yield(pSleepTime);
}

static PetStatus Update(const PetAppHandle petAppHandle, const float deltaTime)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    return pet->Update(deltaTime);
}

PetStatus CreatePet(const PetAppHandle petAppHandle, const CreatePetData* const pCreatePetData)
{
    if(!petAppHandle.Ptr)
    {
        return PetInvalidArg;
    }

    Win32CliPet* const pet = Win32CliPet::FromHandle(petAppHandle);

    return pet->CreatePet(pCreatePetData);
}

static BOOL WINAPI CtrlHandler(const DWORD fdwCtrlType) noexcept
{
    switch(fdwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
            s_ShouldExit = true;
            return TRUE;
        default: return FALSE;
    }
}
#endif
