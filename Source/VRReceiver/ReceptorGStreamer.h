#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "ReceptorGStreamer.generated.h"

// Declaración anticipada para no contaminar Unreal con cabeceras de C
typedef struct _GstElement GstElement;
typedef struct _GstAppSink GstAppSink;

UCLASS()
class VRRECEIVER_API AReceptorGStreamer : public AActor
{
	GENERATED_BODY()

public:
	AReceptorGStreamer();
	virtual void Tick(float DeltaTime) override;

	// Textura expuesta al motor para aplicarla en el HUD del Oculus
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetria")
	UTexture2D* VideoTexture;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	GstElement* Pipeline;
	GstAppSink* AppSink;

	void IniciarGStreamer();
	void ActualizarTextura();
};