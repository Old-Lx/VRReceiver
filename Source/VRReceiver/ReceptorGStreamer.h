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

    // Textura del mapa espacial expuesta al Blueprint
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetria")
    UTexture2D* DepthTexture;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    // Punteros del flujo RGB
    GstElement* Pipeline;
    GstAppSink* AppSink;

    // Punteros del flujo de Profundidad 
    GstElement* PipelineDepth;
    GstAppSink* AppSinkDepth;

    void IniciarGStreamer();
    void IniciarGStreamerProfundidad();

        // Ahora recibe parámetros para ser reutilizable
        void ActualizarTextura(GstAppSink* Sink, UTexture2D* TexturaDestino);
};