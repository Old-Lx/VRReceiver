using UnrealBuildTool;

public class VRReceiver : ModuleRules
{
    public VRReceiver(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        // Inyección de librerías GStreamer
        PublicIncludePaths.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/include/gstreamer-1.0");
        PublicIncludePaths.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/include/glib-2.0");
        PublicIncludePaths.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/lib/glib-2.0/include");

        PublicAdditionalLibraries.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/lib/gstreamer-1.0.lib");
        PublicAdditionalLibraries.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/lib/gobject-2.0.lib");
        PublicAdditionalLibraries.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/lib/glib-2.0.lib");
        PublicAdditionalLibraries.Add("C:/Users/lartrax/AppData/Local/Programs/gstreamer/1.0/msvc_x86_64/lib/gstapp-1.0.lib");
    }
}