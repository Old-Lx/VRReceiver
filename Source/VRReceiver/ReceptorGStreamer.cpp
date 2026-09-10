#include "ReceptorGStreamer.h"

// Envolver las librerías y resolver colisiones de nombres con el motor
THIRD_PARTY_INCLUDES_START
#define GError GstError
#define GThreadPool GstThreadPool
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#undef GThreadPool
#undef GError
THIRD_PARTY_INCLUDES_END

AReceptorGStreamer::AReceptorGStreamer()
{
	PrimaryActorTick.bCanEverTick = true;
	VideoTexture = nullptr;
	Pipeline = nullptr;
	AppSink = nullptr;
}

void AReceptorGStreamer::BeginPlay()
{
	// Crear la textura en blanco a 640x480
	VideoTexture = UTexture2D::CreateTransient(640, 480, PF_B8G8R8A8);
	VideoTexture->UpdateResource();

	IniciarGStreamer();

	Super::BeginPlay();
}

void AReceptorGStreamer::IniciarGStreamer()
{
	if (!gst_is_initialized()) {
		gst_init(nullptr, nullptr);
	}

	// Pipeline UDP configurado para recibir la telemetría
	FString PipelineStr = "udpsrc port=5000 ! application/x-rtp, encoding-name=H264, payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw, format=BGRA ! appsink name=videoshink sync=false max-buffers=1 drop=true";

	GstError* GstErr = nullptr; // Uso de la macro renombrada
	Pipeline = gst_parse_launch(TCHAR_TO_UTF8(*PipelineStr), &GstErr);

	if (GstErr) {
		UE_LOG(LogTemp, Error, TEXT("Error GStreamer: %s"), UTF8_TO_TCHAR(GstErr->message));
		g_clear_error(&GstErr);
		return;
	}

	GstElement* SinkElement = gst_bin_get_by_name(GST_BIN(Pipeline), "videoshink");
	AppSink = GST_APP_SINK(SinkElement);

	gst_element_set_state(Pipeline, GST_STATE_PLAYING);
}

void AReceptorGStreamer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ActualizarTextura();
}

void AReceptorGStreamer::ActualizarTextura()
{
	if (!AppSink || !VideoTexture) return;

	GstSample* Sample = gst_app_sink_try_pull_sample(AppSink, 0);
	if (Sample) {
		GstBuffer* Buffer = gst_sample_get_buffer(Sample);
		GstMapInfo MapInfo;

		if (gst_buffer_map(Buffer, &MapInfo, GST_MAP_READ)) {
			// Definir la región de actualización (toda la imagen de 640x480)
			FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, 640, 480);

			// Copiar los datos a un nuevo bloque de memoria para el hilo de renderizado
			uint8* TextureData = new uint8[MapInfo.size];
			FMemory::Memcpy(TextureData, MapInfo.data, MapInfo.size);

			// Inyectar los píxeles directo a la GPU sin destruir el recurso (con Lambda de limpieza)
			VideoTexture->UpdateTextureRegions(0, 1, Region, 640 * 4, 4, TextureData, [](uint8* SrcData, const FUpdateTextureRegion2D* Regions) {
				delete[] SrcData;
				delete Regions;
				});

			gst_buffer_unmap(Buffer, &MapInfo);
		}
		gst_sample_unref(Sample);
	}
}

void AReceptorGStreamer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Pipeline) {
		gst_element_set_state(Pipeline, GST_STATE_NULL);
		gst_object_unref(Pipeline);
		Pipeline = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}