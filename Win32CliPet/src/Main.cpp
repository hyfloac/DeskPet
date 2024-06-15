#include <PetAI.h>
#include <new>

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
private:
    PetAICallbacks m_Callbacks;
};

static PetStatus CreatePetApp(PetAppHandle* const pOutPetAppHandle, const PetAICallbacks* const pPetAICallbacks);
static PetStatus DestroyPetApp(const PetAppHandle petAppHandle);

static PetStatus SavePetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, const void* const pData, const size_t size);
static PetStatus LoadPetState(const PetAppHandle petAppHandle, const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize);

int main(int argCount, char* args[])
{
    PetFunctions petFunctions {};
    petFunctions.Version = PET_AI_VERSION;
    petFunctions.CreatePetApp = CreatePetApp;
    petFunctions.DestroyPetApp = DestroyPetApp;
    petFunctions.SavePetState = SavePetState;
    petFunctions.LoadPetState = LoadPetState;

    PetStatus status = InitPetAI(&petFunctions);

    if(!IsStatusSuccess(status))
    {
        return -1;
    }

    status = RunPetAI();

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
    (void) file;
    (void) offset;
    (void) pData;
    (void) size;

    return PetSuccess;
}

PetStatus Win32CliPet::LoadPetState(const PetFileHandle file, const size_t offset, void* const pData, size_t* const pSize) noexcept
{
    (void) file;
    (void) offset;
    (void) pData;
    (void) pSize;

    *pSize = 0;
    return PetFail;
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
