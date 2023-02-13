
/**
 * Including the Header libraries and files required
 **/
#include "SkycatchAPI.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

#include "SkycatchSettings.h"

#define LOCTEXT_NAMESPACE "FSkycatchAPIModule"

/**
 * @brief Function that manages the startup of the Plugin
 */
void FSkycatchAPIModule::StartupModule(){

	/**
	 * If we have a valid instance of the settings class, we register the global configuration for the settings and
	 * can be viewed over Project Project Settings>Plugins>Skycatch Skyverse.
	 */
#if WITH_EDITOR
	if (ISettingsModule* SettingModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingModule->RegisterSettings("Project", "Plugins", "Skycatch Skyverse",
			LOCTEXT("RuntimeSettingsName", "Skycatch Skyverse"),
			LOCTEXT("RuntimeSettingsDescription", "Skycatch Skyverse authentication configuration"),
			GetMutableDefault<USkycatchSettings>()
		);
	}
#endif

}

/**
 * @brief Function that manages the shutdown of the Plugin
 */
void FSkycatchAPIModule::ShutdownModule()
{

	/**
	 * If we have a valid instance of the settings class, we unregister the global configuration for the settings.
	 */
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Skycatch Skyverse");
	}
#endif

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSkycatchAPIModule, SkycatchAPI)