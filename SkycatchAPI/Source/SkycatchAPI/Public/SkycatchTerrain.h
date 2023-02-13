#pragma once

/**
 * Including the Header libraries and files required
 **/
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"
#include "GameFramework/Actor.h"
#include "CesiumGeoreference.h"
#include "Cesium3DTileset.h"
#include "CesiumCartographicPolygon.h"
#include "CesiumPolygonRasterOverlay.h"
#include "SkycatchSettings.h"
#include "SkycatchTerrain.generated.h"


UCLASS(Blueprintable)
class SKYCATCHAPI_API ASkycatchTerrain : public AActor
{
	GENERATED_BODY()

	
	
public:	
	
	/**
	 ** @brief Constructor for the actor class that includes all the functionality for the rendering of the tilesets
	**/
	ASkycatchTerrain();

	
	/**
	 * @brief Global Cesium actor used for rendering the tileset retrieved from Skycatch services.
	 * This property can be edited over Blueprints in UE editor.
	**/
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadWrite,
		Category=SkycatchTerrainProperties)
	ACesium3DTileset* Cesium3DTilesetActor;

	
	/**
	 * @brief Global Georeference actor used as instance for transforming the geojson information to UE coordinates for the
	 * Cartographic polygon.
	 * This property can be edited over Blueprints in UE editor.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category=SkycatchTerrainProperties)
	ACesiumGeoreference* GeoreferenceActor;

	
	/**
	 * @brief Global Cartographic polygon used for remove the occlusion between the tileset and the world terrain
	 * This property can be edited over Blueprints in UE editor.
	 */
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadWrite,
		Category=SkycatchTerrainProperties)
	ACesiumCartographicPolygon* CartographicPolygon;

	
	/**
	 * @brief Global property for managing the current visibility of the rendered tileset
	 * This property can be edited over Blueprints in UE editor.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category=SkycatchTerrainProperties)
	bool Cesium3DTilesetActorVisible = true;

	/**
	 * @brief Global property for managing the current visibility of the rendered tileset cartographic polygon
	 * This property can be edited over Blueprints in UE editor.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadOnly,
		Category=SkycatchTerrainProperties)
	bool RasterOverlayVisible = true;

	/**
	 * @brief Global property for managing the latitude of the tileset to be retrieved
	 * This property can be edited over Blueprints in UE editor.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadWrite,
		Category=SkycatchQueryParams)
	FString Latitude;

	/**
	 * @brief Global property for managing the longitude of the tileset to be retrieved
	 * This property can be edited over Blueprints in UE editor.
	 */
	UPROPERTY(EditAnywhere,
		BlueprintReadWrite,
		Category=SkycatchQueryParams)
	FString Longitude;



protected:
	
	virtual void BeginPlay() override;
	
public:	
	
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Function that takes the (Latitude, Longitude) parameters and makes an HTTP call to Skycatch
	 * services, then parses the response and continues the process of rendering.
	 * 
	 * @param Params as a string to add as query params for the API call
	 */
	void FindResource(FString Params);

	/**
	 * @brief Function that receives a string url from the fetched tileset and instantiates or updates the current
	 * tileset from the actor.
	 * 
	 * @param url as a string used to render the Cesium3DTileset
	 */
	void RenderResource(FString url);
	
	/**
	 * @brief Function that takes the data from a geojson obtained over the HTTP call to create and instantiate a
	 * CesiumRasterOverlay, then this new object is added to the world overlay to avoid oclussion in the current
	 * tileset.
	 */
	void RenderRasterOverlay();
	
	/**
	 * @brief function to evaluate if any of the public properties of the tile (Latitude, Longitude) change
	 * to create or update the current tileset of the actor.
	 * 
	 * @param PropertyChangedEvent the event that triggers any changes in the (Latitude, Longitude) params
	 */
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * @brief Function that changes the visibility of the owned 3Dtiles model
	 * Note* For the moment it only works properly in runtime
	 *
	 * @param isVisible as a boolean to indicate the visibility of the tileset
	 */
	UFUNCTION(BlueprintCallable, Category=SkycatchTerrain)
	void SetCesium3DTilesetVisible(bool isVisible);
	
	/**
	 * @brief Function that changes the visibility of the owned polygon raster overlay
	 * Note* For the moment it only works in runtime.
	 * 
	 * @param isVisible as a boolean to indicate the visibility of the polygon raster overlay.
	 */
	UFUNCTION(BlueprintCallable, Category=SkycatchTerrain)
	void SetRasterOverlayVisible(bool isVisible);
	
	/**
	 * @brief This function can be used to request a tileset in specific Latitude and Longitude coordinates.
	 * 
	 * @param Lat as a string containing the required Latitude
	 * @param Lon as a string containing the required Longitude
	 */
	UFUNCTION(BlueprintCallable, Category=SkycatchTerrain)
	void RequestTilesetAtCoordinates(FString Lat, FString Lon);

	/**
	 * @brief This function can be used to request a tileset at the actor's location.
	 * This actor's reference to the active Cesium Georeference must be valid,
	 * since that is used to convert from Unreal to Lat, Lon coordinates
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category=SkycatchTerrain)
	void RequestTilesetAtActorLocation();
	
	/**
	 * @brief Global instance for the raster overlay of the world terrain.
	 */
	UCesiumPolygonRasterOverlay* RasterOverlay;
	
	/**
	 * @brief Global instance of the World Terrain actor.
	 */
	AActor* WorldTerrain;
	
	
	/**
	 * @brief Global instance of the plugin settings visible over Project Project Settings>Plugins>Skycatch Skyverse.
	 */
	USkycatchSettings* SkycatchSettings = GetMutableDefault<USkycatchSettings>();

	/**
	 * @brief Global variable used as intermediate for storing the data obtained over the HTTP request to Skycatch
	 * services.
	 */
	FString HttpData;


	/**
	 * @brief Global variable to storage the current query params used in the HTTP request to Skycatch
	 */
	FString QueryParams;
	
	/**
	 * @brief Global instance of the current tileset that is rendering.
	 */
	TSharedPtr<FJsonObject> SelectedTile;
};
