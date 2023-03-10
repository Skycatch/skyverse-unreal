
/**
 * Including the Header libraries and files required
 **/
#include "SkycatchTerrain.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Cesium3DTileset.h"
#include "CesiumCartographicPolygon.h"
#include "CesiumPolygonRasterOverlay.h"
#include "Math/Vector.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "SkycatchSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"


/**
 * @brief Constructor of the class that sets the default values of the global actors needed
 */
ASkycatchTerrain::ASkycatchTerrain()
:
	Cesium3DTilesetActor(nullptr),
	GeoreferenceActor(nullptr),
	CartographicPolygon(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

/**
 * @brief Called when the game starts or when spawned
 */
void ASkycatchTerrain::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * @brief Called every frame
 * 
 * @param DeltaTime 
 */
void ASkycatchTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * @brief function to evaluate if any of the public properties of the tile (Latitude, Longitude) change
 * to create or update the current tileset of the actor.
 * 
 * @param PropertyChangedEvent the event that triggers any changes in the (Latitude, Longitude) params
 */
#if WITH_EDITOR
void ASkycatchTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//Checks if the event is not null, if not we have a change in the values and need to re-render the tileset
	if(PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetFName());
		const FName	NameLatitude = FName(TEXT("Latitude"));
		const FName	NameLongitude = FName(TEXT("Longitude"));
		const FName	NameRasterOverlayVisible = FName(TEXT("RasterOverlayVisible"));
		const FName	NameCesium3DTilesetActorVisible = FName(TEXT("Cesium3DTilesetActorVisible"));

		//Checks if any of the latitude and longitude parameters changed
		if (PropertyName == NameLatitude || PropertyName == NameLongitude ){

			//Creates an array for making the query params used in the HTTP calling to Skycatch services
			TArray<FStringFormatArg> args;
			args.Add( FStringFormatArg(Latitude));
			args.Add(FStringFormatArg(Longitude));

			//Creates the string with the query params
			QueryParams = FString::Format(TEXT("lat={0}&lng={1}"), args) ;

			//Calls the function to render the tileset over the new latitude, longitude parameters
			FindResource(QueryParams);
		}

		//Checks if we have a change over the visibility value of the raster overlay checkbox 
		if (PropertyName == NameRasterOverlayVisible){
			//Calls the function to change the visibility of the RasterOverlay
			SetRasterOverlayVisible(RasterOverlayVisible);
		}

		//Checks if we have a change over the visibility value of the Cesium3DTileset
		if (PropertyName == NameCesium3DTilesetActorVisible){
			SetCesium3DTilesetVisible(Cesium3DTilesetActorVisible);
		}
		
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

/**
 * @brief Function who??s task is to take the (Latitude, Longitude) parameters and make an HTTP call to Skycatch
 * services, then parse the response and continue the process of rendering.
 * 
 * @param Params as a string to add as query params for the API call
 */
void ASkycatchTerrain::FindResource(FString Params){

	//Checks if there is a Georeference Actor selected, if not the process stops
	if(GeoreferenceActor == nullptr)
	{
		UE_LOG(LogSkycatch, Error, TEXT("No Georeference Actor selected in SkycatchTerrain Actor"));
		return;
	}
	
	FHttpModule& httpModule = FHttpModule::Get();

	// Create an http request
	// The request will execute asynchronously, and call us back on the Lambda below
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> pRequest = httpModule.CreateRequest();

	// This is where we set the HTTP method (GET, POST, etc)
	pRequest->SetVerb(TEXT("GET"));

	// We'll need to tell the server what type of content to expect in the GET data
	pRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	
	// Authorization header
	pRequest->SetHeader(TEXT("SKYVERSE_KEY"), GetData(SkycatchSettings->SKYVERSE_KEY));;


	FString ENDPOINT = GetData(SkycatchSettings->SKYVERSE_ENDPOINT);
	const FString URL = ENDPOINT.Append(Params);

	UE_LOG(LogSkycatch, Warning, TEXT("Full URL: %s"), *URL);
	
	// Set the http URL
	pRequest->SetURL(URL);

	// Set the callback, which will execute when the HTTP call is complete
	pRequest->OnProcessRequestComplete().BindLambda(
		// Here, we "capture" the 'this' pointer (the "&"), so our lambda can call this
		// class's methods in the callback.
		[&](
			FHttpRequestPtr pRequest,
			FHttpResponsePtr pResponse,
			bool connectedSuccessfully) mutable {

		if (connectedSuccessfully) {

			// We should have a JSON response - attempt to process it.
			HttpData = pResponse->GetContentAsString();
			TArray<TSharedPtr<FJsonValue>> Tiles;
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(HttpData);
			FJsonSerializer::Deserialize(JsonReader, Tiles);
			if(Tiles.Num()>0)
			{
				//Selects the first tileset from the response
				SelectedTile = Tiles[0]->AsObject();
				//Parse the tileset url from the response
				FString TilesetUrl = SelectedTile->GetStringField("tilesetUrl");
				//Calls the function that renders the requested tileset
				RenderResource(TilesetUrl);
				
			}else
			{
				//If there is not tiles, prints an error
				UE_LOG(LogSkycatch, Error, TEXT("No tiles found"));
			}
			
		}
		else {
			//If there is an error in the connection to Skycatch services, sends an error
			switch (pRequest->GetStatus()) {
			case EHttpRequestStatus::Failed_ConnectionError:
				UE_LOG(LogSkycatch, Error, TEXT("Connection failed."));
			default:
				UE_LOG(LogSkycatch, Error, TEXT("Request failed."));
			}
		}
	});

	// Finally, submit the request for processing
	pRequest->ProcessRequest();

}

/**
 * @brief Function that receives and string url from the fetched tileset and instantiates or updates the current
 * tileset from the actor.
 * 
 * @param url as a string used to render the Cesium3DTileset
 */
void ASkycatchTerrain::RenderResource(FString url)
{
	//Checks if the global Cesium3DTilesetActor is already instantiated
	if (!Cesium3DTilesetActor){
		//If not, instantiates a new Cesium3DTilesetActor and sets the required properties
		const FVector Location = FVector(0,0,0);
		const FRotator Rotation = FRotator(0,0,0);
		Cesium3DTilesetActor = GetWorld()->SpawnActor<ACesium3DTileset>(Location, Rotation);
		this->Children.Add(Cesium3DTilesetActor);
		Cesium3DTilesetActor->Tags.Add(FName("Skycatch"));
		
		// Change tileset configuration
		Cesium3DTilesetActor->SetEnableOcclusionCulling(false);
		Cesium3DTilesetActor->MaximumScreenSpaceError = 32.0;
	}
	
	//If the actor exists, updates the actor properties to the new response from Skycatch services
	Cesium3DTilesetActor->SetGeoreference(GeoreferenceActor);
	Cesium3DTilesetActor->SetTilesetSource(ETilesetSource::FromUrl);
	Cesium3DTilesetActor->SetUrl(url);
	UE_LOG(LogSkycatch, Display, TEXT("Response %s"), *url);
	
	//Calls the function to re-render the RasterOverlay from the new/updated tileset
	RenderRasterOverlay();
	
}

/**
 * @brief Function that takes the data from a geojson obtained over the HTTP call to create and instantiate a
 * CesiumRasterOverlay, then this new object is added to the world overlay to avoid oclussion in the current
 * tileset.
 */
void ASkycatchTerrain::RenderRasterOverlay(){
	float altitude_m = 0;
	const FVector Location = FVector(0,0,0);
	const FRotator Rotation = FRotator(0,0,0);

	//Gets a reference to the world terrain actor
	if(const UWorld* World = GetWorld())
	{
		WorldTerrain = UGameplayStatics::GetActorOfClass(World, ACesium3DTileset::StaticClass());
	}

	//Parses the response from the current tile fetched from Skycatch services
	TArray<TSharedPtr<FJsonValue>> Coordinates;
	Coordinates = SelectedTile->GetObjectField("outline")->GetArrayField("coordinates")[0]->AsArray();
	TArray<FVector> SplinePoints = {};

	//Iterates over the geojson data that contains the coordinates to create the Raster polygon
	for(int i = 0 ; i<Coordinates.Num(); i++)
	{
		TArray<TSharedPtr<FJsonValue>> Coords;
		Coords = Coordinates[i]->AsArray();
		FString lat;
		FString lng;
		//Gets the latitude and longitude of every coordinate
		Coords[0].Get()->TryGetString(lat);
		Coords[1].Get()->TryGetString(lng);
		//Adds the latitude and longitude as a new vector with double-precision numbers
		glm::dvec3 Point = glm::dvec3(FCString::Atod(*lat), FCString::Atod(*lng), altitude_m);
		//Transforms the latitude, longitude to UE world coordinates
		const glm::dvec3 UECoords = GeoreferenceActor->TransformLongitudeLatitudeHeightToUnreal(Point);
		//Adds the coordinate to the vector to create the cartographic polygon
		SplinePoints.Add(FVector(UECoords.x, UECoords.y, UECoords.z));
	}

	//Checks if already exists a cartographic polygon, if not instantiates a new CesiumCartographicPolygon
	if (CartographicPolygon == nullptr)
	{
		CartographicPolygon = GetWorld()->SpawnActor<ACesiumCartographicPolygon>(Location, Rotation);
		this->Children.Add(CartographicPolygon);
		CartographicPolygon->Tags.Add(FName("Skycatch"));
	}
	
	//Sets the polygon of the CesiumCartographicPolygon
	CartographicPolygon->Polygon->SetSplinePoints(SplinePoints,ESplineCoordinateSpace::Local);

	//Checks if the world terrain exists
	if(WorldTerrain)
	{
		//Finds the CesiumPolygonRasterOverlay component from the world terrain
		UCesiumPolygonRasterOverlay* Raster = WorldTerrain->FindComponentByClass<UCesiumPolygonRasterOverlay>();
		if (!Raster)
		{
			// Create a new PolygonRasterOverlayComponent and add it to the World Terrain
			const auto NewRaster = NewObject<UCesiumPolygonRasterOverlay>(WorldTerrain, FName("CesiumPolygonRasterOverlay"));
			NewRaster->RegisterComponent();
			Raster = NewRaster;
			WorldTerrain->AddInstanceComponent(Raster);
		}

		// At this point we should have a valid Raster Overlay object
		if(Raster)
		{
			//adds the cartographic polygon to the raster overlay to avoid occlusion
			Raster->Polygons.Add(CartographicPolygon);
			//Refresh the world terrain tilesets
			Cast<ACesium3DTileset>(WorldTerrain)->RefreshTileset();
		}
	}
}

/**
 * @brief Function that changes the visibility of the owned 3Dtiles model
 * Note* For the moment it only works properly in runtime
 *
 * @param isVisible as a boolean to indicate the visibility of the tileset
 */
void ASkycatchTerrain::SetCesium3DTilesetVisible(bool isVisible)
{
	if (!Cesium3DTilesetActor)
	{
		UE_LOG(LogSkycatch, Warning, TEXT("No Cesium 3DTileset found"));
		return;
	}
	
	//Sets the actor visibility to the requested value (true, false)
	Cesium3DTilesetActor->SetHidden(!isVisible);
	//Refresh the tileset
	Cesium3DTilesetActor->RefreshTileset();
	Cesium3DTilesetActorVisible = isVisible;
	UE_LOG(LogSkycatch, Log, TEXT("Changed 3DTileset visibility"));
}

/**
 * @brief Function that changes the visibility of the owned polygon raster overlay
 * Note* For the moment it only works in runtime.
 * 
 * @param isVisible as a boolean to indicate the visibility of the polygon raster overlay.
 */
void ASkycatchTerrain::SetRasterOverlayVisible(bool isVisible)
{
	if (!CartographicPolygon)
	{
		UE_LOG(LogSkycatch, Warning, TEXT("No polygon found"));
		return;
	}
	
	//Gets a reference to the world terrain actor
	if(const UWorld* World = GetWorld())
	{
		WorldTerrain = UGameplayStatics::GetActorOfClass(World, ACesium3DTileset::StaticClass());
	}

	if(WorldTerrain)
	{
		//Finds the CesiumPolygonRasterOverlay component from the world terrain
		UCesiumPolygonRasterOverlay* Raster = WorldTerrain->FindComponentByClass<UCesiumPolygonRasterOverlay>();
		if(Raster) {
			if (!isVisible){
				//removes the polygon from the world terrain
				Raster->Polygons.Remove(CartographicPolygon);
			}
			if (isVisible)
			{
				//adds the polygon from the world terrain
				Raster->Polygons.Add(CartographicPolygon);
			}

			Cast<ACesium3DTileset>(WorldTerrain)->RefreshTileset();
			RasterOverlayVisible = isVisible;
		}
	}
}

/**
 * @brief This function can be used to request a tileset in specific Latitude and Longitude coordinates.
 * 
 * @param Lat as a string containing the required Latitude
 * @param Lon as a string containing the required Longitude
 */
void ASkycatchTerrain::RequestTilesetAtCoordinates(FString Lat, FString Lon){
	
	//Creates an array for making the query params used in the HTTP calling to Skycatch services
	TArray<FStringFormatArg> args;
	args.Add( FStringFormatArg(Lat));
	args.Add(FStringFormatArg(Lon));

	//Creates the string with the query params
	QueryParams = FString::Format(TEXT("lat={0}&lng={1}"), args);

	//Calls the function to render the tileset over the new latitude, longitude parameters
	FindResource(QueryParams);
}


/**
 * @brief This function can be used to request a tileset at the actor's location.
 * This actor's reference to the active Cesium Georeference must be valid,
 * since that is used to convert from Unreal to Lat, Lon coordinates
 */
void ASkycatchTerrain::RequestTilesetAtActorLocation()
{
	//Checks if there is a Georeference Actor selected, if not the process stops
	if(GeoreferenceActor == nullptr)
	{
		UE_LOG(LogSkycatch, Error, TEXT("No Georeference Actor selected in SkycatchTerrain Actor"));
		return;
	}

	// Get actor position
	const auto ActorLocation = GetActorLocation();
	
	// Convert actor coordinates from unreal to Lat, Lon
	const auto LatLonHeight = GeoreferenceActor->TransformUnrealToLongitudeLatitudeHeight(glm::dvec3(ActorLocation.X, ActorLocation.Y, ActorLocation.Z));

	// Set Lat, Lon values
	Latitude = FString::SanitizeFloat(LatLonHeight.y);
	Longitude = FString::SanitizeFloat(LatLonHeight.x);

	// Now make a request on the given coordinates
	RequestTilesetAtCoordinates(Latitude, Longitude);
}

