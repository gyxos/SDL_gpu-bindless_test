#include <vulkan/vulkan.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include "gpu.h"

SDL_GPUDevice * CreateGPUDevice() {
    VkPhysicalDeviceFeatures vk10 = {
        .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
        .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
    };

    VkPhysicalDeviceVulkan12Features vk12 =  {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    VkPhysicalDeviceVulkan11Features vk11 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vk12,
        .shaderDrawParameters = VK_TRUE,
    };

    SDL_GPUVulkanOptions options = {
        .vulkan_api_version = VK_API_VERSION_1_2,
        .feature_list = &vk11,
        .vulkan_10_physical_device_features = &vk10,
    };

    SDL_PropertiesID props = SDL_CreateProperties();
    if (props == 0) { return NULL; }

    SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_FEATURE_BINDLESS_RESOURCES_BOOLEAN, true);
    SDL_SetPointerProperty(props, SDL_PROP_GPU_DEVICE_CREATE_VULKAN_OPTIONS_POINTER, &options);

    SDL_GPUDevice *device = SDL_CreateGPUDeviceWithProperties(props);

    if (device == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "SDL_CreateGPUDeviceWithProperties: %s", SDL_GetError());
    }

    SDL_DestroyProperties(props);

    return device;
}
