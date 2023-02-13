#pragma once

/**
 * Including the Header libraries and files required
 **/
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkycatchSettings.generated.h"

UCLASS(config = Engine, DefaultConfig, meta = ( DisplayName = "SkycatchAPI" ), Blueprintable)
class SKYCATCHAPI_API USkycatchSettings : public UObject
{
	GENERATED_BODY()

public:
	
	/**
	 ** @brief Constructor for the plugin configuration class.
	 * This class contains the global variables used for all the API calling to Skycatch services.
	**/
	void UMSkycatchSettings(const FObjectInitializer& obj);

	/**
	 ** @brief Authentication Key used as header for all the petitions for the Skycatch services.
	 * Can be edited over Project Settings>Plugins>Skycatch Skyverse.
	 **/
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = API)
		FString SKYVERSE_KEY;
		
	/**
	 ** @brief Endpoint route for retrieving the tiles over the Skycatch services.
	 * Can be edited over Project Settings>Plugins>Skycatch Skyverse.
	 **/
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = API)
		FString SKYVERSE_ENDPOINT;
	
};

DECLARE_LOG_CATEGORY_EXTERN(LogSkycatch, Log, All);

