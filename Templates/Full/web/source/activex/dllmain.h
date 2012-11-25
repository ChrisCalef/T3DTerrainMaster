// dllmain.h : Declaration of module class.

class CIEWebGamePluginModule : public CAtlDllModuleT< CIEWebGamePluginModule >
{
public :
	DECLARE_LIBID(LIBID_IEFullPluginLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_IEWEBGAMEPLUGIN, "{AB7615A3-A918-488B-B128-96DD62D0AE36}")
};

extern class CIEWebGamePluginModule _AtlModule;
