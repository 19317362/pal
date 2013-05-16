/*--------------------------------------------------------------------------------
    Copyright (c) Microsoft Corporation.  All rights reserved.

*/
/**
    \file

    \brief       Utilities to facilitate implementation of provider tests

    \date        08-10-128 11:23:02


*/
/*----------------------------------------------------------------------------*/

#include <scxcorelib/scxcmn.h>
#include <scxcorelib/scxprocess.h>
#include <testutils/scxunit.h>
#include <testutils/providertestutils.h>
#include <netdb.h>
#include <limits.h>
#include <sys/utsname.h>

// Declare storage for static members of class

std::map<MI_Context *, TestableContext *> TestableContext::ms_map;

std::wstring TestableInstance::PropertyInfo::GetValue_MIString(std::wostringstream &errMsg) const
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_STRING, type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, exists);
    return SCXCoreLib::StrFromMultibyte(std::string(value.string));
}

MI_Uint8 TestableInstance::PropertyInfo::GetValue_MIUint8(std::wostringstream &errMsg) const
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_UINT8, type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, exists);
    return value.uint8;
}

MI_Uint16 TestableInstance::PropertyInfo::GetValue_MIUint16(std::wostringstream &errMsg) const
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_UINT16, type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, exists);
    return value.uint16;
}

MI_Uint32 TestableInstance::PropertyInfo::GetValue_MIUint32(std::wostringstream &errMsg) const
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_UINT32, type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, exists);
    return value.uint32;
}

MI_Uint64 TestableInstance::PropertyInfo::GetValue_MIUint64(std::wostringstream &errMsg) const
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_UINT64, type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, exists);
    return value.uint64;
}

MI_Uint32 TestableInstance::GetNumberOfKeys() const
{
    const MI_Instance* self = GetInstance();
    const MI_ClassDecl* cd = self->classDecl;
    MI_Uint32 i, keyCount = 0;

    for (i = 0; i < cd->numProperties; i++)
    {
        const MI_PropertyDecl* pd = cd->properties[i];
        const Field* field = (Field*)((char*)self + pd->offset);

        if (Field_GetExists(field, static_cast<MI_Type> (pd->type)))
        {
            if (pd->flags & MI_FLAG_KEY)
                keyCount++;
        }
    }

    return keyCount;
}

MI_Uint32 TestableInstance::GetNumberOfProperties() const
{
    const MI_Instance* self = GetInstance();
    const MI_ClassDecl* cd = self->classDecl;
    MI_Uint32 i, propCount = 0;

    for (i = 0; i < cd->numProperties; i++)
    {
        const MI_PropertyDecl* pd = cd->properties[i];
        const Field* field = (Field*)((char*)self + pd->offset);

        if (Field_GetExists(field, static_cast<MI_Type> (pd->type)))
        {
            propCount++;
        }
    }

    return propCount;
}

MI_Result TestableInstance::FindProperty(const char* name, struct PropertyInfo& info) const
{
    const MI_Instance* self = GetInstance();
    const MI_ClassDecl* cd = self->classDecl;

    for (MI_Uint32 i = 0; i < cd->numProperties; i++)
    {
        const MI_PropertyDecl* pd = cd->properties[i];
        const Field* field = (Field*)((char*)self + pd->offset);

        // Did we find the field being requested?
        if (0 == strcmp(name, pd->name))
        {
            info.name = SCXCoreLib::StrFromMultibyte(std::string(pd->name));
            info.isKey = (pd->flags & MI_FLAG_KEY);
            info.type = static_cast<MI_Type> (pd->type);
            Field_Extract(field, info.type, &info.value, &info.exists, &info.flags);
            return MI_RESULT_OK;
        }
    }

    // Not found.
    return MI_RESULT_NOT_FOUND;
}

MI_Result TestableInstance::FindProperty(MI_Uint32 index, struct PropertyInfo& info, bool keysOnly) const
{
    const MI_Instance* self = GetInstance();
    const MI_ClassDecl* cd = self->classDecl;
    MI_Uint32 i, propCount = 0;

    for (i = 0; i < cd->numProperties; i++)
    {
        const MI_PropertyDecl* pd = cd->properties[i];
        const Field* field = (Field*)((char*)self + pd->offset);

        // Did we find the field being requested?
        if (Field_GetExists(field, static_cast<MI_Type> (pd->type)) &&
            ((!keysOnly) || (keysOnly && (pd->flags & MI_FLAG_KEY))))
        {
            if (index == propCount)
            {
                info.name = SCXCoreLib::StrFromMultibyte(std::string(pd->name));
                info.isKey = (pd->flags & MI_FLAG_KEY);
                info.type = static_cast<MI_Type> (pd->type);
                Field_Extract(field, info.type, &info.value, &info.exists, &info.flags);
                return MI_RESULT_OK;
            }
            propCount++;
        }
    }

    // Not found.
    return MI_RESULT_NOT_FOUND;
}

MI_Boolean TestableInstance::GetMIReturn_MIBoolean(std::wostringstream &errMsg) const
{
    struct PropertyInfo info;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_RESULT_OK, FindProperty("MIReturn", info));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, false, info.isKey);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_BOOLEAN, info.type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, info.exists);
    return info.value.boolean;
}

std::wstring TestableInstance::GetKey(const wchar_t *name, std::wostringstream &errMsg) const
{
    struct PropertyInfo info;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_RESULT_OK,
        FindProperty(SCXCoreLib::StrToMultibyte(name).c_str(), info));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, info.isKey);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_STRING, info.type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, info.exists);
    return SCXCoreLib::StrFromMultibyte(std::string(info.value.string));
}

void TestableInstance::GetKey(MI_Uint32 index, std::wstring &name, std::wstring &value, std::wostringstream &errMsg) const
{
    struct PropertyInfo info;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_RESULT_OK, FindProperty(index, info, true));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, info.isKey);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_STRING, info.type);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, true, info.exists);
    name = info.name;
    value = SCXCoreLib::StrFromMultibyte(std::string(info.value.string));
}

std::wstring TestableInstance::GetKeyName(MI_Uint32 index, std::wostringstream &errMsg) const
{
    std::wstring name;
    std::wstring value;
    GetKey(index, name, value, CALL_LOCATION(errMsg));
    return name;
}

std::wstring TestableInstance::GetKeyValue(MI_Uint32 index, std::wostringstream &errMsg) const
{
    std::wstring name;
    std::wstring value;
    GetKey(index, name, value, CALL_LOCATION(errMsg));
    return value;
}

void TestableContext::Print() const
{
    std::cout<<"------------------------------------------------------------------------------------"<<std::endl;
    std::cout<<"TestableContext size: "<<m_inst.size()<<std::endl;
    for (size_t i = 0; i < m_inst.size(); i++)
    {
        m_inst[i].Print();
    }
}

MI_Result MI_CALL TestableContext::PostResult(MI_Context* context, MI_Result result)
{
    std::map<MI_Context*,TestableContext*>::iterator it = TestableContext::ms_map.find(context);
    CPPUNIT_ASSERT_MESSAGE("Unable to find TestableContext!", TestableContext::ms_map.end() != it);
    it->second->m_result = result;

    return MI_RESULT_OK;
}

MI_Result MI_CALL TestableContext::PostInstance(MI_Context *context, const MI_Instance* instance)
{
    std::map<MI_Context*,TestableContext*>::iterator it = TestableContext::ms_map.find(context);
    CPPUNIT_ASSERT_MESSAGE("Unable to find TestableContext!", TestableContext::ms_map.end() != it);

#if 0
    // If the instance that we have has no function table, give it one
    // (Going through hoops to get rid fo const)
    MI_Instance sourceInstance;
    memcpy(&sourceInstance, instance, sizeof(MI_Instance));

    // Hack to eliminate instance.h need (temporary)
    extern MI_InstanceFT __mi_instanceFT;
    sourceInstance.ft = &__mi_instanceFT;

    MI_Instance *inst;
    CPPUNIT_ASSERT_EQUAL( MI_RESULT_OK, MI_Instance_Clone(&sourceInstance,&inst) );
    it->second->m_inst.push_back( inst );
#else
    // Can we build an Instance out of this, and will we live with multiple instances?
    TestableInstance sourceInst(instance);
    sourceInst.__setCopyOnWrite(true);

    it->second->m_inst.push_back( sourceInst );
#endif

    return MI_RESULT_OK;
}

MI_Result MI_CALL TestableContext::RefuseUnload(MI_Context* context)
{
    std::map<MI_Context*,TestableContext*>::iterator it = TestableContext::ms_map.find(context);
    CPPUNIT_ASSERT_MESSAGE("Unable to find TestableContext!", TestableContext::ms_map.end() != it);
    it->second->m_wasRefuseUnloadCalled = true;

    return MI_RESULT_OK;
}


TestableContext::TestableContext()
    : m_pCppContext(NULL),
      m_result(MI_RESULT_SERVER_IS_SHUTTING_DOWN),
      m_wasRefuseUnloadCalled(false)
{
    memset(&m_miContext, 0, sizeof(m_miContext));
    memset(&m_contextFT, 0, sizeof(m_contextFT));
    memset(&m_miPropertySet, 0, sizeof(m_miPropertySet));
    m_pPropertySet = new mi::PropertySet(&m_miPropertySet);

    // Set up the FT table for unit test purposes
    m_contextFT.PostResult = PostResult;
    m_contextFT.PostInstance = PostInstance;
    m_contextFT.RefuseUnload = RefuseUnload;

    m_miContext.ft = &m_contextFT;

    // Be able to find ourselves from the static methods
    TestableContext::ms_map[&m_miContext] = this;
}

TestableContext::~TestableContext()
{
#if 0
    for (std::vector<MI_Instance*>::iterator it = m_inst.begin(); it != m_inst.end(); ++it)
    {
        CPPUNIT_ASSERT_EQUAL( MI_RESULT_OK, MI_Instance_Delete(*it) );
        it = m_inst.erase(it);
    }
#endif

    delete m_pCppContext;
    delete m_pPropertySet;
}

void TestableContext::Reset()
{
    delete m_pCppContext;
    m_pCppContext = NULL;
    m_result = MI_RESULT_SERVER_IS_SHUTTING_DOWN;
    m_wasRefuseUnloadCalled = false;
    m_inst.clear();
}

TestableContext::operator mi::Context&()
{
    delete m_pCppContext;
    m_pCppContext = new mi::Context(&m_miContext);
    return *m_pCppContext;
}

MI_Result FindFieldString(mi::Instance &instance, const char* name, Field* &foundField)
{
    foundField = NULL;

    const MI_Instance* self = instance.GetInstance();
    const MI_ClassDecl* cd = self->classDecl;

    for (MI_Uint32 i = 0; i < cd->numProperties; i++)
    {
        const MI_PropertyDecl* pd = cd->properties[i];
        Field* field = (Field*)((char*)self + pd->offset);

        // Did we find the field being requested?
        if (0 == strcmp(name, pd->name))
        {
            if (MI_STRING != (static_cast<MI_Type>(pd->type)))
            {
                return MI_RESULT_TYPE_MISMATCH;
            }
            foundField = field;
            return MI_RESULT_OK;
        }
    }

    // Not found.
    return MI_RESULT_NOT_FOUND;
}

void VerifyInstancePropertyNames(const TestableInstance &instance, const std::wstring* expectedPropertiesList,
    size_t expectedPropertiesCnt, std::wostringstream &errMsg)
{
    std::set<std::wstring> expectedProperties(expectedPropertiesList, expectedPropertiesList + expectedPropertiesCnt);

    for (MI_Uint32 i = 0; i < instance.GetNumberOfProperties(); ++i)
    {
        TestableInstance::PropertyInfo info;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, MI_RESULT_OK, instance.FindProperty(i, info));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE + "Property mismatch: " + SCXCoreLib::StrToMultibyte(info.name),
            1u, expectedProperties.count(info.name));
    }

    // Be sure that all of the properties in our set exist in the property list
    for (std::set<std::wstring>::const_iterator iter = expectedProperties.begin();
         iter != expectedProperties.end(); ++iter)
    {
        TestableInstance::PropertyInfo info;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE + "Missing property: " + SCXCoreLib::StrToMultibyte(*iter),
            MI_RESULT_OK, instance.FindProperty((*iter).c_str(), info));
    }
}

std::wstring GetFQHostName(std::wostringstream &errMsg)
{
#if defined(sun) || defined(hpux)
    char hostName[MAXHOSTNAMELEN];
#else
    char hostName[HOST_NAME_MAX];
#endif
    CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, 0, gethostname(hostName, sizeof(hostName)));
    hostName[sizeof(hostName) - 1] = 0;
    
    std::wstring fqHostName;
    std::ostringstream processOutput;
    std::ostringstream processErr;
    try 
    {
        std::string command = std::string("sh -c \"nslookup ") + hostName + " | grep \'Name:\' | awk \'{print $2}\'\"";
        std::istringstream processInput;
        int status = SCXCoreLib::SCXProcess::Run(SCXCoreLib::StrFromMultibyte(command),
            processInput, processOutput, processErr, 15000);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE, 0, status);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(ERROR_MESSAGE + "; stderr: " + processErr.str(), 0u, processErr.str().size());
        fqHostName = SCXCoreLib::StrTrim(SCXCoreLib::StrFromMultibyte(processOutput.str()));
    }
    catch(SCXCoreLib::SCXException &e)
    {
        CPPUNIT_FAIL(ERROR_MESSAGE + SCXCoreLib::StrToMultibyte(L" In GetFQHostName(), executing nslookup threw " +
            e.What() + L" " + e.Where()) + "; stderr: " + processErr.str() + "; stdout: " + processOutput.str());
    }
    return SCXCoreLib::StrToLower(fqHostName);
}

bool MeetsPrerequisites(std::wstring testName)
{
#if defined(aix)
    // No privileges needed on AIX.
    return true;
#elif defined(linux) | defined(hpux) | defined(sun)
    // Most platforms need privileges to execute Update() method.
    if (0 == geteuid())
    {
        return true;
    }

    std::wstring warnText;

    warnText = L"Platform needs privileges to run " + testName + L" test";

    SCXUNIT_WARNING(warnText);
    return false;
#else
#error Must implement method MeetsPrerequisites for this platform
#endif
}

std::wstring GetDistributionName(std::wostringstream &errMsg)
{
    std::wstring distributionName;
#if defined(sun) || defined(aix) || defined(hpux)
    struct utsname utsName;
    CPPUNIT_ASSERT_MESSAGE(ERROR_MESSAGE, 0 <= uname(&utsName));
    distributionName = SCXCoreLib::StrFromMultibyte(utsName.sysname);
#elif defined(linux)
#if defined(PF_DISTRO_SUSE)
    distributionName =  L"SuSE Distribution";
#elif defined(PF_DISTRO_REDHAT)
    distributionName =  L"Red Hat Distribution";
#elif defined(PF_DISTRO_ULINUX)
    distributionName =  L"Linux Distribution";
#endif // defined(PF_DISTRO_SUSE)
#endif // defined(sun) || defined(aix) || defined(HPUX)
    return distributionName;
}

