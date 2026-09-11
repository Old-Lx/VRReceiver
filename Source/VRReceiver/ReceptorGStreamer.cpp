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
	DepthTexture = nullptr;
	Pipeline = nullptr;
	PipelineDepth = nullptr;
	AppSink = nullptr;
	AppSinkDepth = nullptr;
}

void AReceptorGStreamer::BeginPlay()
{
	// Crear ambas texturas dinámicas (El formato BGRA es nativo para Unreal)
	VideoTexture = UTexture2D::CreateTransient(640, 480, PF_B8G8R8A8);
	VideoTexture->UpdateResource();

	DepthTexture = UTexture2D::CreateTransient(640, 480, PF_B8G8R8A8);
	DepthTexture->UpdateResource();

	IniciarGStreamer();
	IniciarGStreamerProfundidad();

	Super::BeginPlay();
}

void AReceptorGStreamer::IniciarGStreamer()
{
	if (!gst_is_initialized()) gst_init(nullptr, nullptr);

	FString PipelineStr = "udpsrc port=5000 ! application/x-rtp, encoding-name=H264, payload=96 ! rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw, format=BGRA ! appsink name=videoshink sync=false max-buffers=1 drop=true";
	GstError* GstErr = nullptr;
	Pipeline = gst_parse_launch(TCHAR_TO_UTF8(*PipelineStr), &GstErr);

	if (!GstErr) {
		AppSink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(Pipeline), "videoshink"));
		gst_element_set_state(Pipeline, GST_STATE_PLAYING);
	}
}

void AReceptorGStreamer::IniciarGStreamerProfundidad()
{
	// Puerto 5001 y Payload 97 (Debe coincidir con tu emisor en Debian)
	FString PipelineDepthStr = "udpsrc port=5001 ! application/x-rtp, encoding-name=H264, payload=97 ! rtph264depay ! avdec_h264 ! videoconvert ! video/x-raw, format=BGRA ! appsink name=depthsink sync=false max-buffers=1 drop=true";
	GstError* GstErr = nullptr;
	PipelineDepth = gst_parse_launch(TCHAR_TO_UTF8(*PipelineDepthStr), &GstErr);

	if (!GstErr) {
		AppSinkDepth = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(PipelineDepth), "depthsink"));
		gst_element_set_state(PipelineDepth, GST_STATE_PLAYING);
	}
}

void AReceptorGStreamer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Extraer fotogramas de ambos buffers en cada frame del motor gráfico
	ActualizarTextura(AppSink, VideoTexture);
	ActualizarTextura(AppSinkDepth, DepthTexture);
}

void AReceptorGStreamer::ActualizarTextura(GstAppSink* Sink, UTexture2D* TexturaDestino)
{
	if (!Sink || !TexturaDestino) return;

	GstSample* Sample = gst_app_sink_try_pull_sample(Sink, 0);
	if (Sample) {
		GstBuffer* Buffer = gst_sample_get_buffer(Sample);
		GstMapInfo MapInfo;

		if (gst_buffer_map(Buffer, &MapInfo, GST_MAP_READ)) {
			FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, 640, 480);
			uint8* TextureData = new uint8[MapInfo.size];
			FMemory::Memcpy(TextureData, MapInfo.data, MapInfo.size);

			TexturaDestino->UpdateTextureRegions(0, 1, Region, 640 * 4, 4, TextureData, [](uint8* SrcData, const FUpdateTextureRegion2D* Regions) {
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
	if (PipelineDepth) {
		gst_element_set_state(PipelineDepth, GST_STATE_NULL);
		gst_object_unref(PipelineDepth);
		PipelineDepth = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}