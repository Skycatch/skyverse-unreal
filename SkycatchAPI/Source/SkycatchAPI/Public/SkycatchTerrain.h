#pragma once

/**
 * Including the Header libraries and files required
 **/
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"
#include "GameFramework/Actor.h"
#include "Async/Async.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CesiumGeoreference.h"
#include "Cesium3DTileset.h"
#include "CesiumCartographicPolygon.h"
#include "CesiumPolygonRasterOverlay.h"
#include "SkycatchSettings.h"
#include "SkycatchTerrain.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTilesetRequestCompleted, bool, bSuccess, ACesium3DTileset*, CesiumTileset, ACesiumCartographicPolygon*, CesiumPolygon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTilesetLoaded, ACesium3DTileset*, CesiumTileset);

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

	/*
	* @brief Property that allows to define whether to automatically register the cartographic polygon of a new request as raster overlay
	*/
	UPROPERTY(EditAnywhere,
		BlueprintReadWrite,
		Category = SkycatchTerrainProperties)
	bool AutoRegisterPolygon = true;

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
	void FindResource(FString Params, bool CalledFromEditor);

	/**
	 * @brief Function that receives a string url from the fetched tileset and instantiates or updates the current
	 * tileset from the actor.
	 * 
	 * @param url as a string used to render the Cesium3DTileset
	 */
	void RenderResource(FString url);
	
	/*
	* @brief Adds a CesiumPolygonRasterOverlay component into the World Terrain Actor. Internal use only
	*/
	void AddRasterOverlayComponentToWorldTerrain();

	/*
	* @brief Functions that takes info from a request response to spawn a cartographic polygon
	*/
	void SpawnCartographicPolygon();


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
	

	/*
	* Event called when a request is completed (even if its unsuccessful)
	*/
	UPROPERTY(BlueprintAssignable)
	FOnTilesetRequestCompleted OnTilesetRequestCompleted;

	/*
	* Event called when a tileset is loaded (it automatically binds to the Cesium3DTileset actor every time a new request is made)
	*/
	UPROPERTY(BlueprintAssignable)
	FOnTilesetLoaded OnTilesetLoaded;

	TScriptDelegate <FWeakObjectPtr> CesiumTilesetLoadedListener;

	UFUNCTION(Category = SkycatchTerrain)
	void CesiumTilesetLoadedForwardBroadcast();

	/*
	* Makes a request using lat and lon values. Internal C++ Use only
	*/
	void MakeRequest(double Lat, double Lon);

	/*
	* @brief This function can be used to request a tileset in specific Latitude and Longitude coordinates
	* 
	* @param Lat is the Latitude value
	* @param Lon is the Longitude value
	*/
	UFUNCTION(BlueprintCallable, Category = SkycatchTerrain)
	/*void RequestTilesetAtCoordinates(double Lat, double Lon, ACesiumCartographicPolygon*& CesiumPolygon, ACesium3DTileset*& CesiumTileset);*/
	void RequestTilesetAtCoordinates(double Lat, double Lon);

	/**
	 * @brief This function can be used to request a tileset at the actor's location.
	 * This actor's reference to the active Cesium Georeference must be valid,
	 * since that is used to convert from Unreal to Lat, Lon coordinates
	 */
	UFUNCTION(BlueprintCallable, Category=SkycatchTerrain)
	void RequestTilesetAtActorLocation();
	
	UFUNCTION(CallInEditor, Category = SkycatchTerrain)
	void RequestTilesetAtActorLocationEditor();

	/*
	* @brief This function unloads the current tileset (if any) by destroying the associated Cesium actors
	*/
	UFUNCTION(BlueprintCallable, CallInEditor, Category = SkycatchTerrain)
	void UnloadTileset();

	/**
	 * @brief Global instance for the raster overlay component of the world terrain.
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

/*
* This class implements a blueprint function with async response for the SkycatchTerrain Request at coordinates function
*/
UCLASS()
class SKYCATCHAPI_API URequestSkycatchTilesetAtCoordinates : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnTilesetRequestCompleted OnTilesetRequestCompleted;

	TScriptDelegate <FWeakObjectPtr> SkycatchTerrainEventListener;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = SkycatchTerrain)
	static URequestSkycatchTilesetAtCoordinates* RequestSkycatchTilesetAtCoordinates(UObject* WorldContextObject, 
		ASkycatchTerrain* SkycatchTerrain, 
		double Lat, 
		double Lon,
		bool AutoRegisterPolygon);

	virtual void Activate() override;

private:
	UObject* WorldContextObject;
	ASkycatchTerrain* SkycatchTerrain;
	double Lat;
	double Lon;
	bool AutoRegisterPolygon;

	UFUNCTION()
	void Execute(bool success, ACesium3DTileset* CesiumTileset, ACesiumCartographicPolygon* Polygon);
};

/*
* This class implements a blueprint function with async response for the SkycatchTerrain Request at actor location function
*/
UCLASS()
class SKYCATCHAPI_API URequestSkycatchTilesetAtActorLocation : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FOnTilesetRequestCompleted OnTilesetRequestCompleted;

	TScriptDelegate <FWeakObjectPtr> SkycatchTerrainEventListener;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = SkycatchTerrain)
		static URequestSkycatchTilesetAtActorLocation* RequestSkycatchTilesetAtActorLocation(UObject* WorldContextObject, 
			ASkycatchTerrain* SkycatchTerrain, 
			bool AutoRegisterPolygon);

	virtual void Activate() override;

private:
	UObject* WorldContextObject;
	ASkycatchTerrain* SkycatchTerrain;
	bool AutoRegisterPolygon;

	UFUNCTION()
		void Execute(bool success, ACesium3DTileset* CesiumTileset, ACesiumCartographicPolygon* Polygon);
};