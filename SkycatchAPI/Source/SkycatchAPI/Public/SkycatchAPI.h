#pragma once

/**
 * Including the Header libraries and files required
 **/
#include "CoreMinimal.h"


class FSkycatchAPIModule : public IModuleInterface
{
public:

	/**
	 ** @brief Function executed when the plugin starts 
	 **/
	virtual void StartupModule() override;

	/**
	 ** @brief Function executed when the plugin stops 
	 **/
	virtual void ShutdownModule() override;
};
